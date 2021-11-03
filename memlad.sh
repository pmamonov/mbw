#!/bin/sh

MAX=65536

s=1
while true; do
	./rd ${s}
	s=$(($s * 2))
	if [ $s -gt $MAX ]; then
		break;
	fi;
done
