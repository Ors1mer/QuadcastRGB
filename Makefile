OS = linux # should be overriden if necessary

CFLAGS_DEV = -g -Wall -D DEBUG
CFLAGS_INS = -s -O2

LIBS = -lusb-1.0

SRCMODULES = modules/argparser.c modules/devio.c modules/rgbmodes.c
OBJMODULES = $(SRCMODULES:.c=.o)

BINPATH = ./quadcastrgb
DEVBINPATH = ./dev
MANPATH = nix/quadcastrgb.1

BINDIR_INS = $${HOME}/.local/bin/
MANDIR_INS = $${HOME}/.local/share/man/man1/

# Deb packaging
DEBPKGVER = 1
DEBBINVER = 1.0.0
DEBARCH = amd64
DEBNAME = quadcastrgb-$(DEBBINVER)-$(DEBPKGVER)-$(DEBARCH)

# System-dependent part
ifeq ($(OS),freebsd)
	LIBS = -lusb-1.0 -lintl # libintl requires the explicit indicaiton
endif
ifeq ($(OS),freebsd) # thus, gcc required on FreeBSD
	CC = gcc # clang seems to be unable to find libusb & libintl
endif
# END

dev: main.c $(OBJMODULES)
	$(CC) $(CFLAGS_DEV) $^ $(LIBS) -o $(DEVBINPATH)

quadcastrgb: main.c $(OBJMODULES)
	$(CC) $(CFLAGS_INS) $^ $(LIBS) -o $(BINPATH)


# For directories
%/:
	mkdir -p $@
# For modules
%.o: %.c %.h
ifeq (quadcastrgb, $(MAKECMDGOALS))
	$(CC) $(CFLAGS_INS) -c $< -o $@
else ifeq (install, $(MAKECMDGOALS))
	$(CC) $(CFLAGS_INS) -c $< -o $@
else ifeq (debpkg, $(MAKECMDGOALS))
	$(CC) $(CFLAGS_INS) -c $< -o $@
else
	$(CC) $(CFLAGS_DEV) -c $< -o $@
endif


.PHONY:
install: quadcastrgb $(MANPATH).gz $(BINDIR_INS) $(MANDIR_INS)
	cp $(BINPATH) $(BINDIR_INS)
	cp $(MANPATH).gz $(MANDIR_INS)

man: nix/manpage.md
	pandoc $< -s -t man -o $(MANPATH)
	gzip $(MANPATH)

debpkg: quadcastrgb
	mkdir deb/$(DEBNAME)
	mkdir -p deb/$(DEBNAME)/DEBIAN \
		 deb/$(DEBNAME)/usr/bin \
		 deb/$(DEBNAME)/usr/share/man/man1
	cp deb/control deb/$(DEBNAME)/DEBIAN/control
	cp $(BINPATH) deb/$(DEBNAME)/usr/bin/quadcastrgb
	cp $(MANPATH).gz deb/$(DEBNAME)/usr/share/man/man1
	dpkg --build deb/$(DEBNAME)

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CC) -MM $^ > $@

.SILENT:
tags:
	ctags *.c $(SRCMODULES)

clean:
	rm -rf $(OBJMODULES) $(BINPATH) $(DEVBINPATH) tags deb/$(DEBNAME)
