# Entry point for FreeImage makefiles
# Default to 'make -f Makefile.gnu' for Linux and for unknown OS.
#
MAKEFILE = gnu

default:
	$(MAKE) -f Makefile.$(MAKEFILE)

all:
	$(MAKE) -f Makefile.$(MAKEFILE) all

dist:
	$(MAKE) -f Makefile.$(MAKEFILE) dist

install:
	$(MAKE) -f Makefile.$(MAKEFILE) install

clean:
	$(MAKE) -f Makefile.$(MAKEFILE) clean

