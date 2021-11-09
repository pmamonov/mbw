LOG := $(shell hostname).log
LOGALL := $(shell ls *.log)
PNGALL := $(patsubst %.log,%.png,$(LOGALL))

all: ${LOG}

$(LOG): mbw
	./memlad.sh | tee $@

%.png: %.log
	./plot.py $< $@

mbw: mbw.c
	cc -o $@ $<

clean:
	rm -f rd ${LOG}

png: $(PNGALL)
