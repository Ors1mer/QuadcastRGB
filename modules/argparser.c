/* quadcastrgb - set RGB lights of HyperX Quadcast S and DuoCast
 * File argparser.c
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
#include "argparser.h"

/* Static declarations */
static void set_arg(const char ***arg_pp, const char **argv_end,
                    struct colschemes *cs, int *state, int *verbose);
static void set_br_spd_dly(const char **arg_p, const char **argv_end,
                           int state, struct colschemes *cs);
static void set_mode(const char ***arg_pp, const char **argv_end,
                     int state, struct colschemes *cs);
static void set_colors(const char ***arg_pp, const char **argv_end,
                       int state, struct colschemes *cs);
static void write_default_cols(struct colschemes *cs, int state);
/* Bool functions */
static int no_opt_param(const char **arg_p, const char **argv_end);
static int is_color(const char **arg_p, const char **argv_end);
static int ishexnumber(const char *str);
static int is_number(const char *str);
static int is_mode(const char *str);

#define WRITE_PARAM(TYPE, FUNNAME) \
    static void FUNNAME(TYPE *u, TYPE *l, TYPE value, int state) \
    { \
    if(state == upper) \
        *u = value; \
    else if(state == lower) \
        *l = value; \
    else \
        *u = *l = value; \
    } \

WRITE_PARAM(int, write_int_param)
WRITE_PARAM(const char *, write_str_param)

/* Const arrays */
const char *modes[MODES_CNT] = {
    "solid", "blink", "cycle", "wave", "lightning", "pulse", "visualizer"
};
static const int rainbow[RAINBOW_CNT] = {
    0xff0000, 0xff009e, 0xcd00ff,
    0x2b00ff, 0x0068ff, 0x00ffff,
    0x00ff67, 0x32ff00, 0xceff00,
    nocolor
};

/* Functions */
struct colschemes *parse_arg(int argc, const char **argv, int *verbose)
{
    struct colschemes *cs = malloc(sizeof(*cs));
    const char **arg_p;
    int cs_state = all;

    /* Set defaults */
    cs->upper.br = cs->lower.br = MAX_BR_SPD_DLY;
    cs->upper.spd = cs->lower.spd = SPD_DEFAULT;
    cs->upper.dly = cs->lower.dly = DLY_DEFAULT;
    cs->upper.mode = cs->lower.mode = NULL;

    for(arg_p = argv+1; arg_p < argv+argc; arg_p++)
        set_arg(&arg_p, argv+argc-1, cs, &cs_state, verbose);

    if(!(cs->upper.mode)) { /* any chosen group sets also the other */
        fprintf(stderr, NOMODE_MSG);
        free(cs); exit(argerr);
    }

    return cs;
}

int strequ(const char *str1, const char *str2)
{
    return (0 == strcmp(str1, str2));
}

/* Changes all given parameters except argv_end */
static void set_arg(const char ***arg_pp, const char **argv_end,
                    struct colschemes *cs, int *state, int *verbose)
{
    if(strequ(**arg_pp, "--version")) {
        puts(VERSION_MESSAGE);
        free(cs); exit(success);
    } else if(strequ(**arg_pp, "-h") || strequ(**arg_pp, "--help")) {
        puts(HELP_MESSAGE);
        free(cs); exit(success);
    } else if(strequ(**arg_pp, "-v") || strequ(**arg_pp, "--verbose")) {
        *verbose = 1;
    } else if(strequ(**arg_pp, "-a") || strequ(**arg_pp, "--all")) {
        *state = all;
    } else if(strequ(**arg_pp, "-u") || strequ(**arg_pp, "--upper")) {
        *state = upper;
    } else if(strequ(**arg_pp, "-l") || strequ(**arg_pp, "--lower")) {
        *state = lower;
    } else if(strequ(**arg_pp, "-b") || strequ(**arg_pp, "-s") ||
                                        strequ(**arg_pp, "-d")) {
        set_br_spd_dly(*arg_pp, argv_end, *state, cs);
        (*arg_pp)++; /* skip option's parameter */
    } else if(is_mode(**arg_pp)) {
        set_mode(arg_pp, argv_end, *state, cs);
        set_colors(arg_pp, argv_end, *state, cs);
    } else {
        fprintf(stderr, BADARG_MSG, **arg_pp);
        free(cs); exit(argerr);
    }
}

static int is_mode(const char *str)
{
    int i;
    for(i = 0; i < MODES_CNT; i++) {
        if(strequ(modes[i], str))
            return 1;
    }
    return 0;
}

static void set_br_spd_dly(const char **arg_p, const char **argv_end,
                           int state, struct colschemes *cs)
{
    short num;
    if(no_opt_param(arg_p, argv_end)) {
        fprintf(stderr, NOPARAM_SHORT_MSG, *arg_p);
        free(cs); exit(argerr);
    }
    num = atoi(*(arg_p+1));
    if(num > MAX_BR_SPD_DLY) {
        fprintf(stderr, BS_BADPARAM_MSG, *arg_p);
        free(cs); exit(argerr);
    }
    if(strequ(*arg_p, "-b")) {        /* brightness */
        write_int_param(&(cs->upper.br), &(cs->lower.br), num, state);
    } else if(strequ(*arg_p, "-s")) { /* speed */
        write_int_param(&(cs->upper.spd), &(cs->lower.spd), num, state);
    } else if(strequ(*arg_p, "-d")) { /* delay */
        write_int_param(&(cs->upper.dly), &(cs->lower.dly), num, state);
    }
}

static int is_number(const char *str)
{
    /* Very primitive check, but enough for no_opt_param */
    for(; *str; str++) {
        if(*str < '0' || *str > '9')
            return 0;
    }
    return 1;
}

static int no_opt_param(const char **arg_p, const char **argv_end)
{
    return (arg_p == argv_end || !(is_number(*(arg_p+1)))) ? 1 : 0;
}

static void set_mode(const char ***arg_pp, const char **argv_end,
                     int state, struct colschemes *cs)
{
    write_str_param(&(cs->upper.mode), &(cs->lower.mode), **arg_pp, state);
    if(!(cs->upper.mode) || !(cs->lower.mode)) { /* write solid to the other */
        int swap = (state == upper) ? lower : upper; /* state != all */
        write_str_param(&(cs->upper.mode), &(cs->lower.mode), modes[0], swap);
        write_int_param(cs->upper.colors, cs->lower.colors, black, swap);
        write_int_param(cs->upper.colors+1, cs->lower.colors+1, nocolor,
                        swap);
    }
}

static void set_colors(const char ***arg_pp, const char **argv_end,
                       int state, struct colschemes *cs)
{
    if(!is_color(*arg_pp+1, argv_end)) {
        write_default_cols(cs, state);
    } else {
        int col_cnt = 0;

        do {
            int hexnum;
            (*arg_pp)++;
            if(***arg_pp == '#')
                hexnum = (int)strtol(**arg_pp+1, NULL, 16);
            else
                hexnum = (int)strtol(**arg_pp, NULL, 16);

            write_int_param(&(cs->upper.colors[col_cnt]),
                            &(cs->lower.colors[col_cnt]), hexnum, state);
            col_cnt++;
        } while(is_color(*arg_pp+1, argv_end) && col_cnt < COLORS_CNT);

        write_int_param(&(cs->upper.colors[col_cnt]),
                        &(cs->lower.colors[col_cnt]), nocolor, state);
    }
}

static void write_default_cols(struct colschemes *cs, int state)
{
    const char *md = (state == upper) ? cs->upper.mode : cs->lower.mode;
    if(strequ(md, modes[2]) || strequ(md, modes[3])) { /* cycle or wave */
        int i;
        for(i = 0; i < RAINBOW_CNT; i++) {
            write_int_param(&(cs->upper.colors[i]), &(cs->lower.colors[i]),
                            rainbow[i], state);
        }
    } else if(strequ(md, modes[1])) { /* blink */
        write_int_param(cs->upper.colors, cs->lower.colors,
                        nocolor, state);
    } else { /* solid, lightning, pulse */
        write_int_param(cs->upper.colors, cs->lower.colors, red, state);
        write_int_param(cs->upper.colors+1, cs->lower.colors+1, nocolor,
                        state);
    }
}

static int ishexnumber(const char *str)
{
    if(*str == '#') /* include the "#RRGGBB" notation */
        str++;
    for(; *str; str++) {
        if(!(*str>='0' && *str<='9') && !(*str>='A' && *str<'G') &&
                                        !(*str>='a' && *str<'g')) {
            return 0;
        }
    }
    return 1;
}

static int is_color(const char **arg_p, const char **argv_end)
{
    return (arg_p <= argv_end && (ishexnumber(*arg_p)));
}

