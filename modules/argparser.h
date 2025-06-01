/* quadcastrgb - set RGB lights of HyperX Quadcast S and DuoCast
 * File argparser.h
 * The task of this module is to allocate a colorscheme struct
 * according to the arguments.
 * In case of [-h|--help], write help message and finish program
 * with 0 exit code.
 *
 * <----- License notice ----->
 * Copyright (C) 2022, 2023, 2024 Ors1mer
 *
 * You may contact the author by email:
 * ors1mer [[at]] ors1mer dot xyz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License ONLY.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see
 * <https://www.gnu.org/licenses/gpl-2.0.en.html>. For any questions
 * concerning the license, you can write to <licensing@fsf.org>.
 * Also, you may visit the Free Software Foundation at
 * 51 Franklin Street, Fifth Floor Boston, MA 02110 USA. 
 */
#ifndef ARGPARSER_SENTRY
#define ARGPARSER_SENTRY

#include <stdio.h> /* for fprintf */
#include <stdlib.h> /* for malloc, exit, atoi */
#include <string.h> /* for strcmp */
#include "locale_macros.h"

/* Constants */
#define COLORS_CNT 11
#define MODES_CNT 7
#define RAINBOW_CNT 10
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
#ifndef VERSION
#define VERSION "unknown"
#endif
#define VERSION_MESSAGE "quadcastrgb version " VERSION
#define HELP_MESSAGE _("Usage: quadcastrgb [-h] [-v] [-a|-u|-l] [-b bright] "\
                     "[-s speed] mode [COLORS]...\nAvailable modes: "\
                     "solid, blink, cycle, lightning, wave. Colors are hex "\
                     "numbers.\nSee 'man quadcastrgb' for details.")
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
    unsigned short pid; /* the microphone's product id */
};

/* Functions */
struct colschemes *parse_arg(int argc, const char **argv, int *verbose);
int strequ(const char *str1, const char *str2);

#endif
