/* quadcastrgb - change RGB mode for the microphone HyperX Quadcast S
 * File devio.h
 * Device input/output.
 * The device is microphone HyperX Quadcast S
 * distinguished by the VID & PID.
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
#ifndef DEVIO_SENTRY
#define DEVIO_SENTRY

#include <libusb-1.0/libusb.h>
#include <unistd.h> /* for usleep */
#include <fcntl.h> /* for demonization */
#include <signal.h> /* for signal handling */
#include "locale_macros.h"
#include "rgbmodes.h" /* for datpack & byte_t types, count_color_pairs, defs */

/* Constants */
/* Vendor IDs */
#define DEV_VID_NA 0x0951
#define DEV_VID_EU 0x03f0
/* Product IDs */
#define DEV_PID_NA 0x171f
#define DEV_PID_EU1 0x0f8b
#define DEV_PID_EU2 0x028c

#define DEV_EPOUT 0x00 /* control endpoint OUT */
#define DEV_EPIN 0x80 /* contorl endpoint IN */
/* Packet info */
#define MAX_PCT_CNT 90
#define PACKET_SIZE 64 /* bytes */

#define HEADER_CODE 0x04
#define STARTUP_HEADER1 0x58
#define STARTUP_HEADER2 0x56
#define STARTUP_HEADER3 0x57
#define DATA_HEADER 0x53
#define SIZE_HEADER 0x23

#define INTR_EP_IN 0x82
#define INTR_LENGTH 8

#define TIMEOUT 1000 /* one second per packet */
#define BMREQUEST_TYPE_OUT 0x21
#define BREQUEST_OUT 0x09
#define BMREQUEST_TYPE_IN 0xa1
#define BREQUEST_IN 0x01
#define WVALUE 0x0300
#define WINDEX 0x0000
/* Messages */
#define DEVLIST_ERR_MSG _("Couldn't get the list of USB devices.\n")
#define NODEV_ERR_MSG _("HyperX Quadcast S isn't connected.\n")
#define OPEN_ERR_MSG _("Couldn't open the microphone.\n")
#define BUSY_ERR_MSG _("Another program is using the microphone already. " \
                       "Stopping.\n")
#define TRANSFER_ERR_MSG _("Couldn't transfer a packet! " \
                           "The device might be busy.\n")
#define FOOTER_ERR_MSG _("Footer packet error: %s\n")
#define HEADER_ERR_MSG _("Header packet error: %s\n")
#define SIZEPCK_ERR_MSG _("Size packet error: %s\n")
#define DATAPCK_ERR_MSG _("Data packet error: %s\n")
#define PID_MSG _("Started with pid %d\n")
/* Errcodes */
enum {
    libusberr = 2,
    nodeverr,
    devopenerr,
    transfererr
};

/* Functions */
libusb_device_handle *open_micro(datpack *data_arr);
void send_packets(libusb_device_handle *handle, datpack *data_arr,
                  int pck_cnt, int verbose);
#endif
