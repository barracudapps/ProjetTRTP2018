MAKE=make

all: mk clear

mk:
	cd src && $(MAKE)
	cd tests && $(MAKE)

clear:
	cd src && $(MAKE) clean

clean:
	rm receiver sender
	cd tests && $(MAKE) clean

.PHONY: clean
