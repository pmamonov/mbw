LOG := $(shell hostname).log
LOGALL := $(shell ls *.log)
PNGALL := $(patsubst %.log,%.png,$(LOGALL))

all: ${LOG}

$(LOG): rd
	./memlad.sh | tee $@

%.png: %.log
	./plot.py $< $@

rd: rd.c
	cc -o $@ $<

clean:
	rm -f rd ${LOG}

png: $(PNGALL)
