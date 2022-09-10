#include "rgbmodes.h"

static int biggest_data(struct colschemes *cs);
static int count_data(const struct colscheme *colsch);
static void fill_data(const struct colscheme *colsch, byte_t *da);
static void sequence_solid(const int *colors, byte_t *da);
static void write_hexcolor(int color, byte_t *mem);

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
        sequence_solid(colsch->colors, da);
    } else {
        fprintf(stderr, NOSUPPORT_MSG);
    }
}

static void sequence_solid(const int *colors, byte_t *da)
{
    *da = RGB_CODE; /* write code to the first byte */
    write_hexcolor(*colors, da+1); /* write RGB */
}

static void write_hexcolor(int color, byte_t *mem)
{
    int n;
    for(n = 16; n >= 0; n -= 8) {
        *mem = (byte_t)((color >> n) & 0xff);
        mem++;
    }
}
