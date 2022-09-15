CC = gcc
CFLAGS_DEV = -g -Wall
CFLAGS_INS = -s -O2

LIBS = -lusb-1.0
SRCMODULES = modules/argparser.c modules/devio.c modules/rgbmodes.c
OBJMODULES = $(SRCMODULES:.c=.o)

BINPATH = ./quadcastrgb
DEVBINPATH = ./dev
MANPATH = nix/quadcastrgb.1

BINDIR_INS = $${HOME}/.local/bin/
MANDIR_INS = $${HOME}/.local/share/man/man1/

quadcastrgb: main.c $(OBJMODULES)
	$(CC) $(CFLAGS_INS) $^ $(LIBS) -o $(BINPATH)

dev: main.c $(OBJMODULES)
	$(CC) $(CFLAGS_DEV) $^ $(LIBS) -o $(DEVBINPATH)

install: quadcastrgb $(MANPATH).gz $(BINDIR_INS) $(MANDIR_INS)
	cp $(BINPATH) $(BINDIR_INS)
	cp $(MANPATH).gz $(MANDIR_INS)

# For directories
$${HOME}/%/:
	mkdir -p $@
# For modules
%.o: %.c %.h
	$(CC) $(CFLAGS_DEV) -c $< -o $@

man: nix/manpage.md
	pandoc $< -s -t man -o $(MANPATH)
	gzip $(MANPATH)

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CC) -MM $^ > $@

.SILENT:
clean:
	rm -f $(OBJMODULES) $(BINPATH) $(DEVBINPATH) tags

run: quadcastrgb
	$(BINPATH)

tags:
	ctags *.c $(SRCMODULES)
