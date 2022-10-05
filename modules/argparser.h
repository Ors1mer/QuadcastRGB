/* argparser.h
 * The task of this module is to allocate a colorscheme struct
 * according to the arguments.
 * In case of [-h|--help] write help message and finish program
 * with 0 exit code.
 */
#ifndef ARGPARSER_SENTRY
#define ARGPARSER_SENTRY

#include <stdio.h> /* for fprintf */
#include <stdlib.h> /* for malloc, exit, atoi */
#include <string.h> /* for strcmp */
#include "locale_macros.h"

/* Constants */
#define COLORS_CNT 11
#define MODES_CNT 5
#define RAINBOW_CNT 9
#define MAX_BR_SPD_DLY 100
#define SPD_DEFAULT 81
#define DLY_DEFAULT 10

enum hexcolors {
    red = 0xf20000,
    black = 0,
    nocolor = -1
};

enum arg_exitcodes { success, argerr }; /* exitcodes */

enum diode_group { all, upper, lower }; /* state values */

/* Messages */
#define HELP_MESSAGE _("Usage: quadcastrgb [-h] [-v] [-a|-u|-l] [-b bright] "\
                     "[-s speed] mode [COLORS]...\nAvailable modes: "\
                     "solid, blink, cycle, lightning, wave. Colors are hex "\
                     "numbers.\nSee 'man quadcastrgb' for details.\n")
#define BADARG_MSG   _("Unknown option: %s\n")
#define NOPARAM_LONG_MSG _("%s: no parameter(s) specified\n")
#define NOPARAM_SHORT_MSG _("%s: no parameter or it isn't a natural number\n")
#define BS_BADPARAM_MSG _("%s: the parameter must be an integer 0-100\n")
#define NOMODE_MSG _("No mode specified (solid|blink|cycle|lightning|wave)\n")

/* Structs */
struct colscheme {
    const char *mode;
    int colors[COLORS_CNT];
    int br;
    int spd; /* ignored in solid */
    int dly; /* blink-only */
};

struct colschemes {
    struct colscheme upper; /* for the upper diode */
    struct colscheme lower; /* for the lower diodes */
};

/* Functions */
struct colschemes *parse_arg(int argc, const char **argv, int *verbose);
int strequ(const char *str1, const char *str2);

#endif
