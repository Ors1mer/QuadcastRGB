OS = linux # should be overridden if necessary

CFLAGS_DEV = -g -Wall -D DEBUG -static
CFLAGS_INS = -s -O2 -Wall -static

LIBS =

SRCMODULES = modules/argparser.c modules/devio.c modules/rgbmodes.c
OBJMODULES = $(SRCMODULES:.c=.o)

BINPATH = ./quadcastrgb
DEVBINPATH = ./dev
MANPATH = man/quadcastrgb.1

BINDIR_INS = $${HOME}/.local/bin/
MANDIR_INS = $${HOME}/.local/share/man/man1/

# Packaging
BINVER = 1.0.3
DEBPKGVER = 2
DEBARCH = amd64
DEBNAME = quadcastrgb-$(BINVER)-$(DEBPKGVER)-$(DEBARCH)

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
else ifeq (rpmpkg, $(MAKECMDGOALS))
	$(CC) $(CFLAGS_INS) -c $< -o $@
else
	$(CC) $(CFLAGS_DEV) -c $< -o $@
endif


.PHONY:
install: quadcastrgb $(MANPATH).gz $(BINDIR_INS) $(MANDIR_INS)
	cp $(BINPATH) $(BINDIR_INS)
	cp $(MANPATH).gz $(MANDIR_INS)

debpkg: quadcastrgb
	mkdir deb/$(DEBNAME)
	mkdir -p deb/$(DEBNAME)/DEBIAN \
		 deb/$(DEBNAME)/usr/bin \
		 deb/$(DEBNAME)/usr/share/man/man1
	cp deb/control deb/$(DEBNAME)/DEBIAN/control
	cp $(BINPATH) deb/$(DEBNAME)/usr/bin/quadcastrgb
	cp $(MANPATH).gz deb/$(DEBNAME)/usr/share/man/man1
	dpkg --build deb/$(DEBNAME)

rpmpkg: main.c $(SRCMODULES) man/quadcastrgb.1.gz
	rpmdev-setuptree
	cp -r main.c Makefile modules $${HOME}/rpmbuild/BUILD/
	cp rpm/quadcastrgb.spec $${HOME}/rpmbuild/SPECS/
	tar -zcf $${HOME}/rpmbuild/SOURCES/quadcastrgb-${BINVER}.tgz .
	rpmbuild --ba $${HOME}/rpmbuild/SPECS/quadcastrgb.spec



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
