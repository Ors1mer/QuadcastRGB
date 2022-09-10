CC = gcc
CFLAGS_DEV = -g -Wall
CFLAGS_INS = -s -O2

LIBS = -lusb-1.0
SRCMODULES = modules/argparser.c modules/devio.c modules/rgbmodes.c
OBJMODULES = $(SRCMODULES:.c=.o)

BINNAME = quadcastrgb
MANPATH = unix/quadcastrgb.1

quadcastrgb: main.c $(OBJMODULES)
	$(CC) $(CFLAGS_DEV) $^ $(LIBS) -o $@

install: $(OBJMODULES)
	$(CC) $(CFLAGS_INS) $^ $(LIBS) -o $(BINNAME)

%.o: %.c %.h
	$(CC) $(CFLAGS_DEV) -c $< -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CC) -MM $^ > $@

man: linux/manpage.md
	pandoc $< -s -t man -o $(MANPATH)
	gzip $(MANPATH)

usbtest: usbtest.c
	$(CC) $(CFLAGS_DEV) $^ $(LIBS) -o $@

.SILENT:

clean:
	rm -f $(OBJMODULES) $(BINNAME) tags linux/quadcastrgb.1*

run: quadcastrgb
	./$(BINNAME)

tags:
	ctags *.c modules/*.c modules/*.h
