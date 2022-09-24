#include "rgbmodes.h"

static int biggest_data(struct colschemes *cs);
static int count_data(const struct colscheme *colsch);
static void fill_data(const struct colscheme *colsch, byte_t *da);
static void sequence_solid(const int *colors, int bright, byte_t *da);
static void sequence_blink_random(int bright, int speed,
                                  int delay, byte_t *da);
static void sequence_blink(const struct colscheme *colsch, byte_t *da);

static void write_hexcolor(int color, int bright, byte_t *mem);

datpack *parse_colorscheme(struct colschemes *cs, int *pck_cnt)
{
    datpack *data_arr;
    *pck_cnt = biggest_data(cs);
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
    } else if(strequ(colsch->mode, "blink")) {
        int cnt = 0;
        const int *col;
        if(colsch->colors[0] == nocolor) /* case of random colors */
            return MAX_PCT_COUNT;
        for(col = colsch->colors; *col != nocolor; col++)
            cnt += 101-colsch->spd + colsch->dly;
        /* Ceil rounding: */
        return cnt/COLPAIR_PER_PCT + (cnt % COLPAIR_PER_PCT != 0);
    } else {
        return -1;
    }
}

static void fill_data(const struct colscheme *colsch, byte_t *da)
{
    if(strequ(colsch->mode, "solid")) {
        sequence_solid(colsch->colors, colsch->br, da);
    } else {
        if(colsch->colors[0] == nocolor)
            sequence_blink_random(colsch->br, colsch->spd, colsch->dly, da);
        sequence_blink(colsch, da);
    }
}

static void sequence_solid(const int *colors, int bright, byte_t *da)
{
    *da = RGB_CODE; /* write code to the first byte */
    write_hexcolor(*colors, bright, da+1); /* write RGB */
}

static void sequence_blink_random(int bright, int speed, int delay, byte_t *da)
{
    /* To be written */
    return ;
}

static void sequence_blink(const struct colscheme *colsch, byte_t *da)
{
    /* To be written */
    return ;
}

static void write_hexcolor(int color, int bright, byte_t *mem)
{
    int n;
    for(n = 16; n >= 0; n -= 8) {
        *mem = (byte_t)( (((color >> n) & 0xff)*bright) / 100 );
        mem++;
    }
}
