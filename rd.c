#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/random.h>
#include <sys/mman.h>
#include <error.h>
#include <errno.h>

#define SZ1	64

typedef void (*fun_t)(void **, int);

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
	unsigned long n1 = __measure(f, p, sz, 1, 100, 0);

	__measure(f, p, sz, n1, 1000, 1);
}

int main(int argc, char **argv)
{
	int i, nch;
	unsigned long n;
	unsigned long long sz;
	void *p;
	void **pp;

	if (argc < 2)
		error(1, 0, "USAGE: %s size\n", argv[0]);

	sz = strtoull(argv[1], NULL, 0) << 10;

	printf("%llu ", sz >> 10);

	n = sz / SZ1;
	p = malloc(sz);
	pp = malloc(sz / 64 * sizeof(void *));

	if (mlock(p, sz) < 0 ||
	    mlock(pp, sz / 64 * sizeof(void *)) < 0)
		error(1, errno, "failed to lock mem");

	getrandom(p, sz, 0);

	for (i = 0; i < n; i++)
		pp[i] = p + i * SZ1;
	measure(rd, pp, sz);

	for (nch = 2; nch <= 8; nch *= 2) {
		for (i = 0; i < n; i++)
			pp[i] = p + SZ1 * (n  / nch * (i % nch) + i / nch);
		measure(rd, pp, sz);
	}

	for (i = 0; i < n; i++)
		pp[i] = p + sz - (i + 1) * SZ1;
	measure(rd, pp, sz);

	if (n > RAND_MAX)
		error(1, 0, "n > RAND_MAX\n");
	for (i = 0; i < n; i++) {
		int j = rand() % n;
		void *_p = pp[i];

		pp[i] = pp[j];
		pp[j] = _p;
	}
	measure(rd, pp, sz);

	printf("\n");
	return 0;
}
