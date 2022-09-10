/* devio.h
 * Device input/output.
 * The device is microphone HyperX Quadcast S
 * distinguished by the VID & PID.
 */
#ifndef DEVIO_SENTRY
#define DEVIO_SENTRY

#include <libusb-1.0/libusb.h>
#include "locale_macros.h"
#include "rgbmodes.h" /* for datpack & byte_t types */

/* Constants */
#define DEV_VID 0x0951 /* vendor ID */
#define DEV_PID 0x171f /* product ID */
#define DEV_EPOUT 0x00 /* control endpoint OUT */
#define DEV_EPIN 0x80 /* contorl endpoint IN */
/* Packet info */
#define PACKET_SIZE 64 /* bytes */
#define DATA_HEADER 0x53
#define SIZE_HEADER 0x23

#define TIMEOUT 1000 /* one second per packet */
#define BMREQUEST_TYPE_OUT 0x21
#define BREQUEST_OUT 9
#define BMREQUEST_TYPE_IN 0xa1
#define BREQUEST_IN 1
#define WVALUE 0x0300
#define WINDEX 0x0000
/* Messages */
#define DEVLIST_ERR_MSG _("Couldn't get the list of USB devices.\n")
#define NODEV_ERR_MSG _("HyperX Quadcast S isn't connected.\n")
#define OPEN_ERR_MSG _("Couldn't open the microphone.\n")
#define TRANSFER_ERR_MSG _("Couldn't transfer a packet!\n")
#define FOOTER_ERR_MSG _("Footer packet error: %s\n")
#define HEADER_ERR_MSG _("Header packet error: %s\n")
#define SIZEPCK_ERR_MSG _("Size packet error: %s\n")
#define DATAPCK_ERR_MSG _("Data packet error: %s\n")
/* Errcodes */
enum {
    libusberr = 2,
    nodeverr,
    devopenerr,
    transfererr
};

/* Functions */
libusb_device_handle *open_micro(datpack *data_arr);
void send_packets(libusb_device_handle *handle,
                  datpack *data_arr, int pck_cnt);

#endif
