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
        free(data_arr); \
        exit(transfererr); \
    }

static libusb_device *dev_search();
static int is_micro(libusb_device *dev);

static int send_header(libusb_device_handle *handle,
                       byte_t op_code, short size);
static int send_footer(libusb_device_handle *handle);
static void send_read(libusb_device_handle *handle);
static int send_data(libusb_device_handle *handle,
                     datpack *data_arr, int pck_cnt);
static int send_size(libusb_device_handle *handle, int colpairs);

static void print_packet(byte_t *pck, char *str);


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
        fprintf(stderr, N_("%s\n%s"), libusb_strerror(errcode), OPEN_ERR_MSG);
        FREE_AND_EXIT();
    }
    libusb_free_device_list(devs, 1);
    libusb_set_auto_detach_kernel_driver(handle, 1);
    libusb_claim_interface(handle, 0);
    libusb_claim_interface(handle, 1);
    return handle;
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
    if(descr.idVendor == DEV_VID && descr.idProduct == DEV_PID) {
        return 1;
    }
    return 0;
}

void send_packets(libusb_device_handle *handle, datpack *data_arr, int pck_cnt)
{
    short errcode;

    errcode = send_header(handle, DATA_HEADER, pck_cnt);
    HANDLE_TRANSFER_ERR(errcode);
    errcode = send_data(handle, data_arr, pck_cnt);
    HANDLE_TRANSFER_ERR(errcode);
    errcode = send_footer(handle);
    HANDLE_TRANSFER_ERR(errcode);
    errcode = send_header(handle, SIZE_HEADER, 1);
    HANDLE_TRANSFER_ERR(errcode);
    errcode = send_size(handle, count_color_commands(data_arr, pck_cnt, 0));
    HANDLE_TRANSFER_ERR(errcode);
    errcode = send_footer(handle);
    HANDLE_TRANSFER_ERR(errcode);
}

static int send_header(libusb_device_handle *handle,
                       byte_t op_code, short size)
{
    byte_t *packet;
    ssize_t sent;
    packet = calloc(PACKET_SIZE, 1);
    /* Operation codes */
    *packet = HEADER_CODE; /* the header code */
    *(packet+1) = op_code; 
    /* Size parameter */
    *(packet+8) = size;

    /* Do the transfer */
    sent = libusb_control_transfer(handle, BMREQUEST_TYPE_OUT, BREQUEST_OUT,
                                   WVALUE, WINDEX, packet, PACKET_SIZE,
                                   TIMEOUT);
    #ifdef DEBUG
    print_packet(packet, "Header:");
    send_read(handle);
    #endif
    free(packet);
    if(sent != PACKET_SIZE) {
        fprintf(stderr, HEADER_ERR_MSG, libusb_strerror(sent));
        return sent;
    }
    return 0;
}

static int send_footer(libusb_device_handle *handle)
{
    byte_t *packet;
    ssize_t sent;
    packet = calloc(PACKET_SIZE, 1);
    /* Operation codes */
    *packet = 0x04;
    *(packet+1) = 0x02; 
    /* Do the transfer */
    sent = libusb_control_transfer(handle, BMREQUEST_TYPE_OUT, BREQUEST_OUT,
                                   WVALUE, WINDEX, packet, PACKET_SIZE,
                                   TIMEOUT);
    #ifdef DEBUG
    print_packet(packet, "Footer:");
    send_read(handle);
    #endif
    free(packet);
    if(sent != PACKET_SIZE) {
        fprintf(stderr, FOOTER_ERR_MSG, libusb_strerror(sent));
        return sent;
    }
    return 0;
}

static void send_read(libusb_device_handle *handle)
{
    byte_t *packet;
    packet = calloc(PACKET_SIZE, 1);
    libusb_control_transfer(handle, BMREQUEST_TYPE_IN, BREQUEST_IN, WVALUE,
                            WINDEX, packet, PACKET_SIZE, TIMEOUT);
    #ifdef DEBUG
    print_packet(packet, "Read:");
    #endif
    free(packet);
}

static int send_data(libusb_device_handle *handle,
                     datpack *data_arr, int pck_cnt)
{
    datpack *packet;
    short sent;
    for(packet = data_arr; packet < data_arr+pck_cnt; packet++) {
        sent = libusb_control_transfer(handle, BMREQUEST_TYPE_OUT,
                                       BREQUEST_OUT, WVALUE, WINDEX,
                                       *packet, PACKET_SIZE, TIMEOUT);
        #ifdef DEBUG
        print_packet(*packet, "Data:");
        #endif
        if(sent != PACKET_SIZE) {
            fprintf(stderr, DATAPCK_ERR_MSG, libusb_strerror(sent));
            return sent;
        }
    }
    return 0;
}

static int send_size(libusb_device_handle *handle, int colpairs)
{
    byte_t *packet;
    short sent;
    packet = calloc(PACKET_SIZE, 1);
    /* Operation codes */
    *packet = 0x08;
    *(packet+PACKET_SIZE-5) = 0x28;
    *(packet+PACKET_SIZE-2) = 0xaa;
    *(packet+PACKET_SIZE-1) = 0x55;
    /* The amount of color pairs (upper and lower hex nums) */
    *(packet+PACKET_SIZE-4) = (byte_t)(colpairs & 0xff); /* 1st byte */
    *(packet+PACKET_SIZE-3) = (byte_t)((colpairs >> 8) & 0xff); /* 2nd byte */

    sent = libusb_control_transfer(handle, BMREQUEST_TYPE_OUT, BREQUEST_OUT,
                                   WVALUE, WINDEX, packet, PACKET_SIZE,
                                   TIMEOUT);
    #ifdef DEBUG
    print_packet(packet, "Size:");
    #endif
    free(packet);
    if(sent != PACKET_SIZE) {
        fprintf(stderr, SIZEPCK_ERR_MSG, libusb_strerror(sent));
        return sent;
    }
    return 0;
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
