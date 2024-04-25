/* quadcastrgb - set RGB lights of HyperX Quadcast S and DuoCast
 * File rgbmodes.c
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
#include "rgbmodes.h"

static int count_data(struct colscheme *colsch);
static void fill_data(struct colscheme *colsch, byte_t *da, int pckcnt,
                      int group);
static void equalize(int upper_size, int lower_size, datpack *da);
static void fillup_to(size_t copy_size, byte_t *curr, byte_t *finish);
static void set_brightness(int *color, int br);

/* Solid */
static void sequence_solid(const int *colors, byte_t *da);
/* Blink */
static unsigned int count_blink_data(struct colscheme *colsch);
static void sequence_blink_random(int speed, int dly_seg, byte_t *da);
static void sequence_blink(const struct colscheme *colsch, byte_t *da,
                           int pckcnt);
static void blink_segment_fill(int col, int col_seg, int dly_seg, byte_t **da);
static void color_fill(int color, int size, byte_t **da);
static int random_color();
/* Cycle */
static unsigned int count_cycle_data(struct colscheme *colsch);
static int get_gradient_length(const int *color, int spd);
static void sequence_cycle(const int *color, int spd, byte_t *da);
static void write_gradient(byte_t **da, int start_col, int end_col,
                           int length);
/* Wave */
static void sequence_wave(int *color, int spd, int group, byte_t *da);
static void wave_array_shift(int *color);
/* Lightning & Pulse */
static unsigned int count_lightning_data(struct colscheme *colsch);
static void sequence_lightning(const int *color, int spd, int group,
                               int synchronous, byte_t *da);
static int next_gradient_color(int color, int endcolor, unsigned int size);

/* Shared */
static void write_hexcolor(int color, byte_t *mem);
static unsigned int colarr_len(const int *arr);
static unsigned int sizeof_frames(int *color, unsigned int framesize);

#ifdef DEBUG
static void print_datpack(datpack *da, int pck_cnt);
#endif

datpack *parse_colorscheme(struct colschemes *cs, int *pck_cnt)
{
    datpack *data_arr;
    int seq_upper, seq_lower;

    seq_upper = count_data(&cs->upper);
    seq_lower = count_data(&cs->lower);
    if(seq_upper < 1 || seq_lower < 1) {
        fprintf(stderr, NOSUPPORT_MSG);
        free(cs); exit(254);
    }

    *pck_cnt = seq_upper >= seq_lower ? seq_upper : seq_lower;
    data_arr = calloc(sizeof(datpack), *pck_cnt);

    fill_data(&cs->upper, *data_arr, *pck_cnt, upper);
    fill_data(&cs->lower, *data_arr+BYTE_STEP, *pck_cnt, lower);
    equalize(seq_upper, seq_lower, data_arr);

    #ifdef DEBUG
    print_datpack(data_arr, *pck_cnt);
    #endif

    return data_arr;
}

short count_color_commands(const datpack *data_arr, int pck_cnt, int colgroup)
{
    short cnt, step = 0;
    const byte_t *b;
    if(colgroup) /* case of lower color commands */
        step = BYTE_STEP;
    cnt = (pck_cnt-1)*8;
    for(b = data_arr[pck_cnt-1]; b < data_arr[pck_cnt-1]+DATA_PACKET_SIZE;
                                                         b += 2*BYTE_STEP) {
        if(*(b+step) != RGB_CODE)
            break;
        cnt++;
    }
    return cnt;
}

static int count_data(struct colscheme *colsch)
{
    if(strequ(colsch->mode, "solid")) {
        return 1;
    } else if(strequ(colsch->mode, "blink")) {
        return count_blink_data(colsch);
    } else if(strequ(colsch->mode, "cycle") || strequ(colsch->mode, "wave")) {
        return count_cycle_data(colsch);
    } else if(strequ(colsch->mode, "lightning") ||
              strequ(colsch->mode, "pulse")) {
        return count_lightning_data(colsch);
    } else {
        return -1;
    }
}

static unsigned int count_blink_data(struct colscheme *colsch)
{
    unsigned int frame, size = 0;

    if(colsch->colors[0] == nocolor) { /* case of random colors */
        srand(time(NULL)); /* random seed (must be done only once) */
        return MAX_PCT_COUNT;
    }

    frame = 101-colsch->spd + colsch->dly;
    size = sizeof_frames(colsch->colors, frame);
    return DIV_CEIL(size, COLPAIR_PER_PCT);
}

static unsigned int count_cycle_data(struct colscheme *colsch)
{
    unsigned int size;
    /* The size of one gradient: */
    size = SPEED_RANGE(MIN_CYCL_TR, MAX_CYCL_TR, colsch->spd);
    /* The size of all colpairs: */
    size *= colarr_len(colsch->colors); 
    if(size > MAX_COLPAIR_COUNT) /* case of overflow */
        return MAX_PCT_COUNT;
    return DIV_CEIL(size, COLPAIR_PER_PCT);
}

static unsigned int count_lightning_data(struct colscheme *colsch)
{
    unsigned int frame, size = 0;
    frame = SPEED_RANGE(MIN_LGHT_BL, MAX_LGHT_BL, colsch->spd) +
            SPEED_RANGE(MIN_LGHT_UP, MAX_LGHT_UP, colsch->spd) +
            SPEED_RANGE(MIN_LGHT_DOWN, MAX_LGHT_DOWN, colsch->spd);
    size = sizeof_frames(colsch->colors, frame);
    return DIV_CEIL(size, COLPAIR_PER_PCT);
}

static unsigned int sizeof_frames(int *color, unsigned int framesize)
{
    unsigned int size = 0;
    for(; *color != nocolor && size+framesize <= MAX_COLPAIR_COUNT; color++)
        size += framesize;
    *color = nocolor; /* strip the array */
    return size;
}

static unsigned int colarr_len(const int *arr)
{
    unsigned int cnt;
    for(cnt = 0; *(arr+cnt) != nocolor; cnt++)
        {}
    return cnt;
}

static void fill_data(struct colscheme *colsch, byte_t *da, int pckcnt,
                      int group)
{
    set_brightness(colsch->colors, colsch->br);
    if(strequ(colsch->mode, "solid")) {
        sequence_solid(colsch->colors, da);
    } else if(strequ(colsch->mode, "blink")) {
        if(colsch->colors[0] == nocolor)
            sequence_blink_random(colsch->spd, colsch->dly, da);
        else
            sequence_blink(colsch, da, pckcnt);
    } else if(strequ(colsch->mode, "cycle")) {
        sequence_cycle(colsch->colors, colsch->spd, da);
    } else if(strequ(colsch->mode, "wave")) {
        sequence_wave(colsch->colors, colsch->spd, group, da);
    } else if(strequ(colsch->mode, "lightning")) {
        sequence_lightning(colsch->colors, colsch->spd, group, 0, da);
    } else if(strequ(colsch->mode, "pulse")) {
        sequence_lightning(colsch->colors, colsch->spd, group, 1, da);
    }
}

static void set_brightness(int *color, int br) 
{
    for(; color && *color != nocolor; color++) {
        int i, shift;
        byte_t rgb[3];
        for(shift = 16, i = 0; i < 3; shift -= 8, i++) {
            rgb[i] = (byte_t)((*color >> shift) & 0xff);
            rgb[i] = rgb[i]*br/100;
        }
        *color = (rgb[0] << 16) + (rgb[1] << 8) + rgb[2];
    }
}

static void equalize(int upper_size, int lower_size, datpack *da)
{
    enum { upper = 0, lower };
    byte_t *upper_end, *lower_end;
    upper_size = count_color_commands(da, upper_size, upper);
    lower_size = count_color_commands(da, lower_size, lower);
    upper_end = *da + 2*BYTE_STEP*(upper_size-1);
    lower_end = *da + 2*BYTE_STEP*(lower_size-1) + BYTE_STEP;

    if(upper_size < lower_size) {
        fillup_to(upper_size, upper_end+2*BYTE_STEP, lower_end-BYTE_STEP);
    } else if(lower_size < upper_size) {
        fillup_to(lower_size, lower_end+2*BYTE_STEP, upper_end+BYTE_STEP);
    } /* else equalizing isn't needed */
}

static void fillup_to(size_t copy_size, byte_t *curr, byte_t *finish)
{
    while(curr <= finish) {
        memcpy(curr, curr-(2*BYTE_STEP*copy_size), BYTE_STEP);
        curr += 2*BYTE_STEP;
    }
}

/* Mode-related functions */
static void sequence_solid(const int *colors, byte_t *da)
{
    *da = RGB_CODE; /* write code to the first byte */
    write_hexcolor(*colors, da+1); /* write RGB */
}

static void sequence_blink_random(int speed, int delay, byte_t *da)
{
    int colpair = 0;
    int col_seg, dly_seg;

    col_seg = RAND_COL_SEG_MIN +
              (int)(speed * (RAND_COL_SEG_MAX-RAND_COL_SEG_MIN)) / MAX_SPD;
    dly_seg = RAND_DLY_SEG_MIN +
              (int)(delay * (RAND_DLY_SEG_MAX-RAND_DLY_SEG_MIN)) / MAX_DLY;
     
    while(colpair < MAX_COLPAIR_COUNT) {
        colpair += col_seg + dly_seg;
        if(colpair > MAX_COLPAIR_COUNT) /* strip color segment if overflow */
            col_seg -= colpair - MAX_COLPAIR_COUNT;
        blink_segment_fill(random_color(), col_seg, dly_seg, &da);
    }
}

static void sequence_blink(const struct colscheme *colsch, byte_t *da,
                           int pckcnt)
{
    const int *col;
    int col_seg = 101 - colsch->spd;
    for(col = colsch->colors; *col != nocolor; col++)
        blink_segment_fill(*col, col_seg, colsch->dly, &da);
}

static void blink_segment_fill(int col, int col_seg, int dly_seg, byte_t **da)
{
    color_fill(col, col_seg, da);
    color_fill(black, dly_seg, da);
}

static void sequence_cycle(const int *color, int spd, byte_t *da)
{
    const int *first_col;
    int tr_length;
    first_col = color;
    tr_length = get_gradient_length(color, spd);
    for(; *color != nocolor; color++) {
        int tr_start, tr_end;

        tr_start = *color;
        if(*(color+1) == nocolor)
            tr_end = *first_col;
        else
            tr_end = *(color+1);

        write_gradient(&da, tr_start, tr_end, tr_length);
    }
}

static int get_gradient_length(const int *color, int spd)
{
    int color_cnt, tr_size;

    color_cnt = colarr_len(color);

    tr_size = MIN_CYCL_TR + (MAX_CYCL_TR - MIN_CYCL_TR)*(100 - spd)/100;
    if(tr_size*color_cnt > MAX_COLPAIR_COUNT)
        return MIN_CYCL_TR +
               (MAX_COLPAIR_COUNT/color_cnt - MIN_CYCL_TR)*(100 - spd)/100;
    return tr_size;
}

static void write_gradient(byte_t **da, int start_col, int end_col, int length)
{
    byte_t rgb_st[3], rgb_end[3], rgb_curr[3];
    int shift, i;
    /* Fill the arrays */
    for(shift = 16, i = 0; shift >= 0; shift -= 8, i++) {
        rgb_st[i] = (byte_t)((start_col >> shift) & 0xff);
        rgb_end[i] = (byte_t)((end_col >> shift) & 0xff);
        rgb_curr[i] = rgb_st[i]; /* the start is going to be the 1st rgb */
    }
    /* Write the transition to *da */
    for(i = 1; i <= length; i++, *da += BYTE_STEP) {
        int j;
        **da = RGB_CODE;
        (*da)++;
        for(j = 0; j < 3; j++, (*da)++) {
            **da = rgb_curr[j]; /* write R, G, or B */
            /* Alter the first RGB depending on the second and the length */
            rgb_curr[j] = (int)(rgb_st[j] +
                          ((float)(i)/(length - 1))*(rgb_end[j] - rgb_st[j]));
        }
    }
}

static void sequence_wave(int *color, int spd, int group, byte_t *da)
{
    if(group == lower)
        wave_array_shift(color);
    /* Just do the same as in the Cycle mode */
    sequence_cycle(color, spd, da);
}

static void wave_array_shift(int *color)
{
    int *tmp, first;
    first = *color;
    for(tmp = color; *(tmp+1) != nocolor; tmp++)
        *(tmp) = *(tmp+1);
    *(tmp) = first;
}

static void sequence_lightning(const int *color, int spd, int group,
                               int synchronous, byte_t *da)
{
    unsigned int bl_size, up, down; /* the sizes of sections */
    bl_size = SPEED_RANGE(MIN_LGHT_BL, MAX_LGHT_BL, spd);
    up = SPEED_RANGE(MIN_LGHT_UP, MAX_LGHT_UP, spd);
    down = SPEED_RANGE(MIN_LGHT_DOWN, MAX_LGHT_DOWN, spd);
    for(; *color != nocolor; color++) {
        if(group == lower && !synchronous)
            color_fill(black, bl_size, &da);
        write_gradient(&da, black, *color, up);
        write_gradient(&da, next_gradient_color(*color, black, down), black,
                       down);
        if(group == upper || synchronous)
            color_fill(black, bl_size, &da);
    }
}

static int next_gradient_color(int color, int endcolor, unsigned int size)
{
    byte_t rgb[3], rgb_end[3];
    int shift, i, nextcolor = 0;
    for(shift = 16, i = 0; shift >= 0; shift -= 8, i++) {
        /* Get R, G, or B values */
        rgb[i] = (byte_t)((color >> shift) & 0xff);
        rgb_end[i] = (byte_t)((endcolor >> shift) & 0xff);
        /* Perform one step */
        rgb[i] = (int)(rgb[i] +
                 ((float)(1)/(size - 1))*(rgb_end[i] - rgb[i]));
        nextcolor += (int)(rgb[i] << shift);
    }
    return nextcolor;
}

static int random_color()
{
    /* Generates a pseudorandom number from 0x1 to 0xffffff */
    return 1 + (int)(16777215.0*rand()/(RAND_MAX+1.0));
}

static void write_hexcolor(int color, byte_t *mem)
{
    int n;
    for(n = 16; n >= 0; n -= 8) {
        *mem = (byte_t)((color >> n) & 0xff);
        mem++;
    }
}

static void color_fill(int color, int size, byte_t **da)
{
    for(; size > 0; size--, *da += 2*BYTE_STEP) {
        **da = RGB_CODE;
        write_hexcolor(color, (*da)+1);
    }
}

#ifdef DEBUG
static void print_datpack(datpack *da, int pck_cnt)
{
    int i, j;
    printf(N_("Packets to be sent: %d\n"), pck_cnt);
    for(j = 0; j < pck_cnt; j++) {
        printf(N_("Packet %d:\n"), j+1);
        for(i = 0; i < DATA_PACKET_SIZE; i++) {
            printf("%02X ", (unsigned int)da[j][i]);
            if((i+1) % 4 == 0)
                printf("\t");
            if((i+1) % 8 == 0)
                puts("");
        }
        puts("");
    }
}
#endif
