/* quadcastrgb - change RGB mode for the microphone HyperX Quadcast S
 * File devio.c
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
#include "devio.h"

/* For open_micro */
#define FREE_AND_EXIT() \
    libusb_free_device_list(devs, 1); \
    free(data_arr); \
    libusb_exit(NULL); \
    exit(libusberr)

#define HANDLE_ERR(CONDITION, MSG) \
    if(CONDITION) { \
        fprintf(stderr, MSG); \
        FREE_AND_EXIT(); \
    }
/* For send_packets */
#define HANDLE_TRANSFER_ERR(ERRCODE) \
    if(ERRCODE) { \
        fprintf(stderr, TRANSFER_ERR_MSG); \
        libusb_close(handle); \
        libusb_exit(NULL); \
        free(data_arr); \
        exit(transfererr); \
    }

/* Microphone opening */
static int claim_dev_interface(libusb_device_handle *handle);
static libusb_device *dev_search(libusb_device **devs, ssize_t cnt);
static int is_micro(libusb_device *dev);
/* Packet transfer */
static short send_display_command(byte_t *packet,
                                  libusb_device_handle *handle);
static void display_data_arr(libusb_device_handle *handle,
                             const byte_t *colcommand, const byte_t *end);
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
libusb_device_handle *open_micro(datpack *data_arr)
{
    libusb_device **devs;
    libusb_device *micro_dev = NULL;
    libusb_device_handle *handle;
    ssize_t dev_count;
    short errcode;
    errcode = libusb_init(NULL);
    if(errcode) {
        perror("libusb_init");
        free(data_arr); exit(libusberr);
    }
    dev_count = libusb_get_device_list(NULL, &devs);
    HANDLE_ERR(dev_count < 0, DEVLIST_ERR_MSG);
    micro_dev = dev_search(devs, dev_count);
    HANDLE_ERR(!micro_dev, NODEV_ERR_MSG);
    errcode = libusb_open(micro_dev, &handle);
    if(errcode) {
        fprintf(stderr, "%s\n%s", libusb_strerror(errcode), OPEN_ERR_MSG);
        FREE_AND_EXIT();
    }
    errcode = claim_dev_interface(handle);
    if(errcode) {
        libusb_close(handle); FREE_AND_EXIT();
    }
    libusb_free_device_list(devs, 1);
    return handle;
}

static int claim_dev_interface(libusb_device_handle *handle)
{
    int errcode0, errcode1;
    libusb_set_auto_detach_kernel_driver(handle, 1); /* might be unsupported */
    errcode0 = libusb_claim_interface(handle, 0);
    errcode1 = libusb_claim_interface(handle, 1);
    if(errcode0 == LIBUSB_ERROR_BUSY || errcode1 == LIBUSB_ERROR_BUSY) {
        fprintf(stderr, BUSY_ERR_MSG);
        return 1;
    } else if(errcode0 == LIBUSB_ERROR_NO_DEVICE ||
                                          errcode1 == LIBUSB_ERROR_NO_DEVICE) {
        fprintf(stderr, OPEN_ERR_MSG);
        return 1;
    }
    return 0;
}

static libusb_device *dev_search(libusb_device **devs, ssize_t cnt)
{
    libusb_device **dev;
    for(dev = devs; dev < devs+cnt; dev++) {
        if(is_micro(*dev))
            return *dev;
    }
    return NULL;
}

static int is_micro(libusb_device *dev)
{
    struct libusb_device_descriptor descr; /* no freeing needed */
    libusb_get_device_descriptor(dev, &descr);
    if(descr.idVendor == DEV_VID_NA && descr.idProduct == DEV_PID_NA) {
        return 1;
    } else if(descr.idVendor == DEV_VID_EU) {
        if(descr.idProduct == DEV_PID_EU1 || descr.idProduct == DEV_PID_EU2
                                          || descr.idProduct == DEV_PID_EU3)
            return 1;
    }
    return 0;
}

void send_packets(libusb_device_handle *handle, datpack *data_arr,
                  int pck_cnt, int verbose)
{
    short command_cnt;
    #ifdef DEBUG
    puts("Entering display mode...");
    #endif
    #ifndef DEBUG
    daemonize(verbose);
    #endif
    command_cnt = count_color_commands(data_arr, pck_cnt, 0);
    signal(SIGINT, nonstop_reset_handler);
    signal(SIGTERM, nonstop_reset_handler);
    /* The loop works until a signal handler resets the variable */
    nonstop = 1; /* set to 1 only here */
    while(nonstop)
        display_data_arr(handle, *data_arr, *data_arr+2*BYTE_STEP*command_cnt);
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

static void display_data_arr(libusb_device_handle *handle,
                             const byte_t *colcommand, const byte_t *end)
{
    short sent;
    byte_t *packet;
    byte_t header_packet[PACKET_SIZE] = {
        HEADER_CODE, DISPLAY_CODE, 0, 0, 0, 0, 0, 0, PACKET_CNT, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    packet = calloc(PACKET_SIZE, 1);
    while(colcommand < end && nonstop) {
        sent = send_display_command(header_packet, handle);
        if(sent != PACKET_SIZE) {
            nonstop = 0; break; /* finish program in case of any errors */
        }
        memcpy(packet, colcommand, 2*BYTE_STEP);
        sent = libusb_control_transfer(handle, BMREQUEST_TYPE_OUT,
                   BREQUEST_OUT, WVALUE, WINDEX, packet, PACKET_SIZE, TIMEOUT);
        if(sent != PACKET_SIZE) {
            nonstop = 0; break;
        }
        #ifdef DEBUG
        print_packet(packet, "Data:");
        #endif
        colcommand += 2*BYTE_STEP;
        usleep(1000*55);
    }
    free(packet);
}

static short send_display_command(byte_t *packet, libusb_device_handle *handle)
{
    short sent;
    sent = libusb_control_transfer(handle, BMREQUEST_TYPE_OUT, BREQUEST_OUT,
                                 WVALUE, WINDEX, packet, PACKET_SIZE,
                                 TIMEOUT);
    #ifdef DEBUG
    print_packet(packet, "Header display:");
    if(sent != PACKET_SIZE)
        fprintf(stderr, HEADER_ERR_MSG, libusb_strerror(sent));
    #endif
    return sent;
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
