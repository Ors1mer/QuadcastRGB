#include "rgbmodes.h"

static int biggest_data(struct colschemes *cs);
static int count_data(struct colscheme *colsch);
static int count_blink_data(struct colscheme *colsch);
static void fill_data(const struct colscheme *colsch, byte_t *da, int pckcnt);

/* Solid */
static void sequence_solid(const int *colors, int bright, byte_t *da);

/* Blink */
static void sequence_blink_random(int bright, int speed, int delay,
                                  byte_t *da);
static void sequence_blink(const struct colscheme *colsch, byte_t *da,
                           int pckcnt);
static void blink_color_fill(int color, int size, int bright, byte_t *da);

/* Shared */
static void write_hexcolor(int color, int bright, byte_t *mem);

#ifdef DEBUG
static void print_datpack(datpack *da, int pck_cnt);
#endif

datpack *parse_colorscheme(struct colschemes *cs, int *pck_cnt)
{
    datpack *data_arr;
    *pck_cnt = biggest_data(cs);
    data_arr = calloc(sizeof(datpack), *pck_cnt);

    fill_data(&cs->upper, *data_arr, *pck_cnt);
    fill_data(&cs->lower, *data_arr+BYTE_STEP, *pck_cnt);

    #ifdef DEBUG
    print_datpack(data_arr, *pck_cnt);
    #endif

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

static int count_data(struct colscheme *colsch)
{
    if(strequ(colsch->mode, "solid")) {
        return 1;
    } else if(strequ(colsch->mode, "blink")) {
        return count_blink_data(colsch);
    } else {
        return -1;
    }
}

static int count_blink_data(struct colscheme *colsch)
{
    int cnt = 0;
    int *col;

    if(colsch->colors[0] == nocolor) /* case of random colors */
        return MAX_PCT_COUNT;

    for(col = colsch->colors; *col != nocolor; col++) {
        if(cnt + 101-colsch->spd + colsch->dly > MAX_COLPAIR_COUNT) {
            *col = nocolor; /* strip the sequence to avoid overflow */
            break;
        }
        cnt += 101-colsch->spd + colsch->dly;
    }

    /* Ceil rounding: */
    return cnt/COLPAIR_PER_PCT + (cnt % COLPAIR_PER_PCT != 0);
}

static void fill_data(const struct colscheme *colsch, byte_t *da, int pckcnt)
{
    if(strequ(colsch->mode, "solid")) {
        sequence_solid(colsch->colors, colsch->br, da);
    } else if(strequ(colsch->mode, "blink")) {
        if(colsch->colors[0] == nocolor)
            sequence_blink_random(colsch->br, colsch->spd, colsch->dly, da);
        sequence_blink(colsch, da, pckcnt);
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

static void sequence_blink(const struct colscheme *colsch, byte_t *da,
                           int pckcnt)
{
    const int *col;
    int count = 0;
    int col_seg = 101-colsch->spd; /* size of colorful segment */
    for(col = colsch->colors; *col != nocolor; col++) {
        blink_color_fill(*col, col_seg, colsch->br, da);
        da += 2*BYTE_STEP*col_seg;
        blink_color_fill(black, colsch->dly, 0, da);
        da += 2*BYTE_STEP*colsch->dly;
        count += col_seg + colsch->dly;
    }
}

static void blink_color_fill(int color, int size, int bright, byte_t *da)
{
    int i = 0;
    for(; i < size; i++) {
        *da = RGB_CODE;
        write_hexcolor(color, bright, da+1);
        da += 2*BYTE_STEP;
    }
}

static void write_hexcolor(int color, int bright, byte_t *mem)
{
    int n;
    for(n = 16; n >= 0; n -= 8) {
        *mem = (byte_t)( (((color >> n) & 0xff)*bright) / 100 );
        mem++;
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
