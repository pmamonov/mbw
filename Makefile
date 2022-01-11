LOG := $(shell hostname).log
LOGALL := $(shell ls *.log)
PNGALL := $(patsubst %.log,%.png,$(LOGALL))

all: ${LOG}

$(LOG): mbw
	./memlad.sh | tee $@

%.png: %.log
	./plot.py $< $@

mbw: mbw.c
	cc -O3 -o $@ $<

clean:
	rm -f mbw ${LOG}

png: $(PNGALL)
