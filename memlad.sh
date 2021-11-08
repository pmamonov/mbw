#!/bin/sh

MAX=65536

TESTS="rseq rseq2 rseq4 rseq8 rinv rrnd"

echo "# sz $TESTS"

s=1
while true; do
	./rd $s $TESTS
	s=$(($s * 2))
	if [ $s -gt $MAX ]; then
		break;
	fi;
done
