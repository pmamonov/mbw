LOG := $(shell hostname).log

all: ${LOG}

$(LOG): rd
	./memlad.sh | tee $@

%.png: %.log
	./plot.py $< $@

rd: rd.c
	cc -o $@ $<

clean:
	rm -f rd ${LOG}
