#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/random.h>
#include <sys/mman.h>
#include <errno.h>

#define error(e, r, ...)					\
	do {							\
		fprintf(stderr, "mbw: ");			\
		fprintf(stderr, __VA_ARGS__);			\
		if (r)						\
			fprintf(stderr, ": %s", strerror(r));	\
		fprintf(stderr, "\n");				\
		if (e)						\
			exit(e);				\
	} while (0);

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof((a)[0]))
#define SZ1	64

typedef void (*fun_t)(void **, int);

struct test {
	unsigned char *name;
	int (*t)(fun_t, void *, void **, unsigned long long, int, char **);
	fun_t f;
};


static void rd(void **_p, int n)
{
	register volatile unsigned long long *p;
	register int i;

	for (i = 0, p = _p[0]; i < n; p = _p[++i]) {
		p[0];
		p[1];
		p[2];
		p[3];
		p[4];
		p[5];
		p[6];
		p[7];
	}
}

static void wr(void **_p, int n)
{
	register volatile unsigned long long *p;
	register int i;

	for (i = 0, p = _p[0]; i < n; p = _p[++i]) {
		p[0] = 0;
		p[1] = 0;
		p[2] = 0;
		p[3] = 0;
		p[4] = 0;
		p[5] = 0;
		p[6] = 0;
		p[7] = 0;
	}
}

static unsigned long __measure(fun_t f, void **p, unsigned long long sz, int n1, int ms, int report)
{
	int n = 0;
	struct timeval tv1, tv2;
	unsigned long long t;

	gettimeofday(&tv1, NULL);
	while (1) {
		unsigned long i;

		for (i = 0; i < n1; i++) {
			f(p, sz / SZ1);
			n += 1;
		}
		gettimeofday(&tv2, NULL);
		t = (tv2.tv_sec - tv1.tv_sec) * 1000 + (tv2.tv_usec - tv1.tv_usec) / 1000;
		if (t > ms)
			break;
	}
	if (report)
		printf(" %llu", n * sz / 1024 / t );
	return n;
}

static void measure(fun_t f, void **p, unsigned long long sz)
{
	unsigned long n1 = __measure(f, p, sz, 1, 50, 0);

	__measure(f, p, sz, n1, 100, 1);
}

static int seq(fun_t f, void *p, void **pp, unsigned long long sz, int argc, char **argv)
{
	unsigned long  i, n = sz / SZ1;

	for (i = 0; i < n; i++)
		pp[i] = p + i * SZ1;
	measure(f, pp, sz);
	return 0;
}

static int inv(fun_t f, void *p, void **pp, unsigned long long sz, int argc, char **argv)
{
	unsigned long  i, n = sz / SZ1;

	for (i = 0; i < n; i++)
		pp[i] = p + sz - (i + 1) * SZ1;
	measure(f, pp, sz);
	return 0;
}

static void pstr(void *p, void **pp, unsigned long n, unsigned long nch)
{
	int i;

	for (i = 0; i < n; i++)
		pp[i] = p + SZ1 * (n  / nch * (i % nch) + i / nch);
}

static int seqn(fun_t f, void *p, void **pp, unsigned long long sz, int argc, char **argv)
{
	unsigned long  i, n = sz / SZ1;
	int nch = 2;

	if (argc < 2)
		error(1, 0, "%s requires an argument", __func__);
	nch = atoi(argv[1]);
	pstr(p, pp, n, nch);
	measure(f, pp, sz);
	return 1;
}

static int strn(fun_t f, void *p, void **pp, unsigned long long sz, int argc, char **argv)
{
	unsigned long  i, n = sz / SZ1;
	int str = 2;

	if (argc < 2)
		error(1, 0, "%s requires an argument", __func__);
	str = atoi(argv[1]);
	pstr(p, pp, n, n / str);
	measure(f, pp, sz);
	return 1;
}


static int rnd(fun_t f, void *p, void **pp, unsigned long long sz, int argc, char **argv)
{
	unsigned long  i, n = sz / SZ1;

	if (n > RAND_MAX)
		error(1, 0, "n > RAND_MAX\n");

	for (i = 0; i < n; i++)
		pp[i] = p + i * SZ1;

	for (i = 0; i < n; i++) {
		int j = rand() % n;
		void *_p = pp[i];

		pp[i] = pp[j];
		pp[j] = _p;
	}
	measure(f, pp, sz);
	return 0;
}

static struct test tests[] = {
	{"rseq", seq, rd},
	{"rinv", inv, rd},
	{"rseqn", seqn, rd},
	{"rstrn", strn, rd},
	{"rrnd", rnd, rd},
	{"wseq", seq, wr},
	{"winv", inv, wr},
	{"wseqn", seqn, wr},
	{"wstrn", strn, wr},
	{"wrnd", rnd, wr},
};

int main(int argc, char **argv)
{
	int i, j;
	unsigned long long sz;
	void *p;
	void **pp;

	if (argc < 3)
		error(1, 0, "USAGE: %s size test [test1] [test2] ...\n", argv[0]);

	sz = strtoull(argv[1], NULL, 0) << 10;

	if (!sz)
		error(1, 0, "invalid size");

	p = malloc(sz);
	pp = malloc(sz / 64 * sizeof(void *));

	if (mlock(p, sz) < 0 ||
	    mlock(pp, sz / 64 * sizeof(void *)) < 0)
		error(1, errno, "failed to lock mem");

	getrandom(p, sz, 0);

	printf("%llu ", sz >> 10);

	for (j = 2; j < argc; j++) {
		int found = 0;

		for (i = 0; i < ARRAY_SIZE(tests); i++)
			if (!strcmp(argv[j], tests[i].name)) {
				found = 1;
				j += tests[i].t(tests[i].f, p, pp, sz,
						argc - j, argv + j);
				break;
			}
		if (!found)
			error(1, 0, "invalid test `%s`", argv[j]);
	}


	printf("\n");
	return 0;
}
