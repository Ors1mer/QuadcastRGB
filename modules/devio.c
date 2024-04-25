/* quadcastrgb - set RGB lights of HyperX Quadcast S and DuoCast
 * File devio.c
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
#include "devio.h"

/* Microphone opening */
static int claim_dev_interface();
static void dev_search();
static int is_micro();
/* Packet transfer */
static short send_display_command();
static void display_data_arr();
#ifndef DEBUG
static void daemonize(int verbose);
#endif
#ifdef DEBUG
static void print_packet(byte_t *pck, char *str);
#endif

/* Signal handling */
volatile static sig_atomic_t nonstop = 0; /* BE CAREFUL: GLOBAL VARIABLE */
static void nonstop_reset_handler()
{
    /* No need in saving errno or setting the handler again
     * because the program just frees memory and exits */
    nonstop = 0;
}

/* Functions */
void open_micro(datpack *data_arr)
{
    fprintf(stderr, NODEV_ERR_MSG);
}

static int claim_dev_interface()
{
}

static void dev_search()
{
}

static int is_micro()
{
}

void send_packets()
{
}

#ifndef DEBUG
static void daemonize(int verbose)
{
    int pid;

    chdir("/");
    pid = fork();
    if(pid > 0)
        exit(0);
    setsid();
    pid = fork();
    if(pid > 0)
        exit(0);

    if(verbose)
        printf(PID_MSG, getpid()); /* notify the user */
    fflush(stdout); /* force clear of the buffer */
    close(0);
    close(1);
    close(2);
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
}
#endif

static void display_data_arr()
{
}

static short send_display_command()
{
}

#ifdef DEBUG
static void print_packet(byte_t *pck, char *str)
{
    byte_t *p;
    puts(str);
    for(p = pck; p < pck+PACKET_SIZE; p++) {
        printf("%02X ", (int)(*p));
        if((p-pck+1) % 16 == 0)
            puts("");
    }
    puts("");
}
#endif
