OS = linux # should be overridden if necessary
VERSION = 1.0.5

CFLAGS_DEV = -g -Wall -DVERSION="\"$(VERSION)"\" -D DEBUG
CFLAGS_INS = -s -O2 -DVERSION="\"$(VERSION)"\"

LIBS = -lusb-1.0

SRCMODULES = modules/argparser.c modules/devio.c modules/rgbmodes.c
OBJMODULES = $(SRCMODULES:.c=.o)

BINPATH = ./quadcastrgb
DEVBINPATH = ./dev
MANPATH = man/quadcastrgb.1

BINDIR_INS = $${HOME}/.local/bin/
MANDIR_INS = $${HOME}/.local/share/man/man1/

# Packaging
DEBPKGVER = 2
DEBARCH = amd64
DEBNAME = quadcastrgb-$(VERSION)-$(DEBPKGVER)-$(DEBARCH)

# System-dependent part
ifeq ($(OS),freebsd)
	LIBS = -lusb-1.0 -lintl # libintl requires the explicit indication
endif
ifeq ($(OS),freebsd) # thus, gcc required on FreeBSD
	CC = gcc # clang seems to be unable to find libusb & libintl
endif
ifeq ($(OS),macos) # pass this info to the source code to disable daemonization
	CFLAGS_DEV += -D OS_MAC
	CFLAGS_INS += -D OS_MAC
endif
# END

quadcastrgb: main.c $(OBJMODULES)
	$(CC) $(CFLAGS_INS) $^ $(LIBS) -o $(BINPATH)

dev: main.c $(OBJMODULES)
	$(CC) $(CFLAGS_DEV) $^ $(LIBS) -o $(DEVBINPATH)

# For directories
%/:
	mkdir -p $@
# For modules
%.o: %.c %.h
ifeq (dev, $(MAKECMDGOALS))
	$(CC) $(CFLAGS_DEV) -c $< -o $@
else
	$(CC) $(CFLAGS_INS) -c $< -o $@
endif

install: quadcastrgb $(BINDIR_INS) $(MANDIR_INS)
	cp $(BINPATH) $(BINDIR_INS)
	cp $(MANPATH).gz $(MANDIR_INS)

debpkg: quadcastrgb
	mkdir -p packages/deb/$(DEBNAME)/DEBIAN \
		 packages/deb/$(DEBNAME)/usr/bin \
		 packages/deb/$(DEBNAME)/usr/share/man/man1
	cp packages/deb/control packages/deb/$(DEBNAME)/DEBIAN/control
	cp $(BINPATH) packages/deb/$(DEBNAME)/usr/bin/quadcastrgb
	cp $(MANPATH).gz packages/deb/$(DEBNAME)/usr/share/man/man1
	dpkg --build packages/deb/$(DEBNAME)

rpmpkg: main.c $(SRCMODULES) man/quadcastrgb.1.gz
	rpmdev-setuptree
	cp -r main.c Makefile modules man $${HOME}/rpmbuild/BUILD/
	cp packages/rpm/quadcastrgb.spec $${HOME}/rpmbuild/SPECS/
	tar -zcf $${HOME}/rpmbuild/SOURCES/quadcastrgb-${VERSION}.tgz .
	rpmbuild --ba $${HOME}/rpmbuild/SPECS/quadcastrgb.spec

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CC) -MM $^ > $@

tags:
	ctags *.c $(SRCMODULES)

clean:
	rm -rf $(OBJMODULES) $(BINPATH) $(DEVBINPATH) tags deb/$(DEBNAME)
