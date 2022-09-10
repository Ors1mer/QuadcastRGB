argparser.o: modules/argparser.c modules/argparser.h \
 modules/locale_macros.h
devio.o: modules/devio.c modules/devio.h modules/locale_macros.h \
 modules/rgbmodes.h modules/argparser.h
rgbmodes.o: modules/rgbmodes.c modules/rgbmodes.h modules/argparser.h \
 modules/locale_macros.h
