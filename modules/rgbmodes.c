/* quadcastrgb - change RGB mode for the microphone HyperX Quadcast S
 * File rgbmodes.c
 *
 * <----- License notice ----->
 * Copyright (C) 2022 Ors1mer
 *
 * You may contact the author by email:
 * ors1mer_dev [[at]] proton.me
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
#include "rgbmodes.h"

static int biggest_data(struct colschemes *cs);
static int count_data(const struct colscheme *colsch);
static void fill_data(const struct colscheme *colsch, byte_t *da);
static void sequence_solid(const int *colors, int bright, byte_t *da);
static void write_hexcolor(int color, int bright, byte_t *mem);

datpack *parse_colorscheme(struct colschemes *cs, int *pck_cnt)
{
    datpack *data_arr;
    *pck_cnt = biggest_data(cs); /* side effect: store size in variable */
    data_arr = calloc(sizeof(datpack), *pck_cnt);

    fill_data(&cs->upper, *data_arr);
    fill_data(&cs->lower, *data_arr+4);

    return data_arr;
}

static int biggest_data(struct colschemes *cs)
{
    int su = count_data(&cs->upper);
    int sl = count_data(&cs->lower);

    if(su < 1 || sl < 1) {
        fprintf(stderr, NOSUPPORT_MSG);
        free(cs); exit(254);
    }

    return (su >= sl ? su : sl);
}

static int count_data(const struct colscheme *colsch)
{
    if(strequ(colsch->mode, "solid")) {
        return 1;
    } else {
        return -1;
    }
}

static void fill_data(const struct colscheme *colsch, byte_t *da)
{
    if(strequ(colsch->mode, "solid")) {
        sequence_solid(colsch->colors, colsch->br, da);
    } else {
        fprintf(stderr, NOSUPPORT_MSG);
    }
}

static void sequence_solid(const int *colors, int bright, byte_t *da)
{
    *da = RGB_CODE; /* write code to the first byte */
    write_hexcolor(*colors, bright, da+1); /* write RGB */
}

static void write_hexcolor(int color, int bright, byte_t *mem)
{
    int n;
    for(n = 16; n >= 0; n -= 8) {
        *mem = (byte_t)( (((color >> n) & 0xff)*bright) / 100 );
        mem++;
    }
}
