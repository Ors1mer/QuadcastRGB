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
#include <unistd.h> /* for usleep */
#include <fcntl.h> /* for daemonization */
#include <signal.h> /* for signal handling */

#include "locale_macros.h"

#include "devio.h"

/* Constants */
#define DISPLAY_MODE_SLEEP_TIME 55*1000 /* microsec */
#define QS2S_DISPLAY_SLEEP_TIME 700 /* microsec */

#define DEV_EPOUT 0x00 /* control endpoint OUT */
#define DEV_EPIN 0x80 /* control endpoint IN */
#define QS2S_EDP_OUT 0x06 /* interrupt endpoint OUT on Interface 1 */
#define QS2S_EDP_IN 0x85 /* interrupt endpoint IN on Interface 1 */

/* Packet info */
#define MAX_PCT_CNT 90
#define PACKET_SIZE 64 /* bytes */

#define HEADER_CODE 0x04
#define DISPLAY_CODE 0xf2
#define PACKET_CNT 0x01

#define INTR_EP_IN 0x82
#define INTR_LENGTH 8

#define QS2S_RESPONSE_CODE 0xff

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
#define INTERRUPT_CMD_ERR_MSG _("USB Interrupt command error on " \
                                                       "endpoint 0x%02x: %s\n")
#define INTERRUPT_RSP_ERR_MSG _("USB Interrupt response error on " \
                                                       "endpoint 0x%02x: %s\n")
#define PID_MSG _("Started with pid %d\n")
/* Error codes */
enum {
    libusberr = 2,
    nodeverr,
    devopenerr,
    transfererr
};

/* For open_mic */
#define FREE_AND_EXIT() \
    libusb_free_device_list(devs, 1); \
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

/* Vendor IDs */
#define DEV_VID_KINGSTON      0x0951
#define DEV_VID_HP            0x03f0
/* Product IDs */
const unsigned short product_ids_kingston[] = {
    0x171f
};
const unsigned short product_ids_hp[] = {
    0x0f8b,
    0x028c,
    0x048c,
    0x068c,
    0x098c,         /* Duocast */
    QUADCAST_2S_PID /* Quadcast 2S */
};

/* Microphone opening */
static int claim_dev_interface(libusb_device_handle *handle);
static libusb_device *dev_search(libusb_device **devs, ssize_t cnt);
static int is_compatible_mic(libusb_device *dev);
static void get_dev_vid_pid(libusb_device *dev, unsigned short *vid,
                           unsigned short *pid);
/* Packet transfer */
static short send_display_command(byte_t *packet,
                                  libusb_device_handle *handle);
static int qs2s_send_display_command(byte_t *packet,
                                                 libusb_device_handle *handle);
static void display_data_arr(libusb_device_handle *handle,
                             const byte_t *colcommand, const byte_t *end);
static void qs2s_display_data_arr(libusb_device_handle *handle,
                                          const byte_t *data_arr, int pck_cnt);
static int send_interrupt_with_rsp(libusb_device_handle *handle, byte_t *pck,
                                                        byte_t out, byte_t in);
static int qs2s_rsp_check(const byte_t *cmd, const byte_t *rsp);
#ifdef OS_MAC
static hid_device *hidapi_open_qs2s(void);
static int hidapi_send_interrupt_with_rsp(hid_device *dev, byte_t *pck);
static void hidapi_qs2s_display_data_arr(hid_device *dev,
                                         const byte_t *data_arr, int pck_cnt);
static int hidapi_qs2s_send_display_command(byte_t *packet, hid_device *dev);
#endif
#if !defined(DEBUG) && !defined(OS_MAC)
static void daemonize(int verbose);
#endif
#ifdef DEBUG
static void print_packet(byte_t *pck, char *str);
#endif

/* Signal handling */
volatile static sig_atomic_t nonstop = 0; /* BE CAREFUL: GLOBAL VARIABLE */
static void nonstop_reset_handler(int s)
{
    /* No need in saving errno or setting the handler again
     * because the program just frees memory and exits */
    nonstop = 0;
}

/* Functions */
struct mic_handle open_mic(unsigned short *pid)
{
    struct mic_handle mh;
    libusb_device **devs;
    libusb_device *mic_dev = NULL;
    ssize_t dev_count;
    short errcode;
    memset(&mh, 0, sizeof(mh));

#ifdef OS_MAC
    /* On macOS, try HIDAPI first for QS2S to avoid needing root */
    mh.hid = hidapi_open_qs2s();
    if(mh.hid) {
        *pid = QUADCAST_2S_PID;
        mh.is_hidapi = 1;
        return mh;
    }
    #ifdef DEBUG
    fprintf(stderr, "HIDAPI: could not open Quadcast 2S; "
                    "falling back to libusb (may need root).\n");
    #endif
#endif

    errcode = libusb_init(NULL);
    if(errcode) {
        perror("libusb_init");
        exit(libusberr);
    }
    dev_count = libusb_get_device_list(NULL, &devs);
    HANDLE_ERR(dev_count < 0, DEVLIST_ERR_MSG);
    mic_dev = dev_search(devs, dev_count);
    HANDLE_ERR(!mic_dev, NODEV_ERR_MSG);
    get_dev_vid_pid(mic_dev, NULL, pid);
    errcode = libusb_open(mic_dev, &mh.usb);
    if(errcode) {
        fprintf(stderr, "%s\n%s", libusb_strerror(errcode), OPEN_ERR_MSG);
        FREE_AND_EXIT();
    }
    errcode = claim_dev_interface(mh.usb);
    if(errcode) {
        libusb_close(mh.usb); FREE_AND_EXIT();
    }
    libusb_free_device_list(devs, 1);
    return mh;
}

static int claim_dev_interface(libusb_device_handle *handle)
{
    int errcode;
#ifdef OS_MAC
    /* macOS: auto_detach may silently fail inside libusb if the process
     * lacks the com.apple.vm.device-access entitlement and isn't root.
     * Manually detach kernel drivers and continue even if detach fails
     * (claim may still succeed if the driver doesn't exclusively lock). */
    int iface;
    for (iface = 0; iface <= 1; iface++) {
        if (libusb_kernel_driver_active(handle, iface) == 1) {
            errcode = libusb_detach_kernel_driver(handle, iface);
            if (errcode != LIBUSB_SUCCESS &&
                                          errcode != LIBUSB_ERROR_NOT_FOUND) {
                fprintf(stderr, "detach_kernel_driver(%d): %s\n",
                        iface, libusb_strerror(errcode));
            }
        }
    }
#else
    libusb_set_auto_detach_kernel_driver(handle, 1);
#endif

    errcode = libusb_set_configuration(handle, 1);
    if (errcode != LIBUSB_SUCCESS && errcode != LIBUSB_ERROR_BUSY) {
        fprintf(stderr, "set_configuration(1): %s\n",
                                                    libusb_strerror(errcode));
        /* Non-fatal: proceed to claim attempt */
    }

    errcode = libusb_claim_interface(handle, 0);
    if (errcode != LIBUSB_SUCCESS) {
        fprintf(stderr, "claim_interface(0): %s\n",
                                                    libusb_strerror(errcode));
        return 1;
    }
    errcode = libusb_claim_interface(handle, 1);
    if (errcode != LIBUSB_SUCCESS) {
        fprintf(stderr, "claim_interface(1): %s\n",
                                                    libusb_strerror(errcode));
        return 1;
    }
    return 0;
}

static libusb_device *dev_search(libusb_device **devs, ssize_t cnt)
{
    libusb_device **dev;
    for(dev = devs; dev < devs+cnt; dev++) {
        if(is_compatible_mic(*dev))
            return *dev;
    }
    return NULL;
}

static int is_compatible_mic(libusb_device *dev)
{
    int i, arr_size;
    const unsigned short *product_id_arr;
    unsigned short vid, pid;
    get_dev_vid_pid(dev, &vid, &pid);

    if (vid == DEV_VID_KINGSTON) {
        product_id_arr = product_ids_kingston;
        arr_size = sizeof(product_ids_kingston)/sizeof(*product_id_arr);
    } else if (vid == DEV_VID_HP) {
        product_id_arr = product_ids_hp;
        arr_size = sizeof(product_ids_hp)/sizeof(*product_id_arr);
    } else {
        return 0;
    }

    #ifdef DEBUG
    printf("Valid vendor found: %04x\nTrying product ids:\n", vid);
    #endif
    for (i = 0; i < arr_size; i++) {
        #ifdef DEBUG
        printf("\t%04x\n", product_id_arr[i]);
        #endif
        if (pid == product_id_arr[i])
            return 1;
    }
    return 0;
}

static void get_dev_vid_pid(libusb_device *dev, unsigned short *vid,
                           unsigned short *pid)
{
    struct libusb_device_descriptor descr;
    libusb_get_device_descriptor(dev, &descr);

    if (vid)
        *vid = descr.idVendor;
    if (pid)
        *pid = descr.idProduct;
}

void send_packets(struct mic_handle *mh, const datpack *data_arr,
                  int pck_cnt, int verbose, int pid)
{
    #ifdef DEBUG
    puts("Entering display mode...");
    #endif
    #if !defined(DEBUG) && !defined(OS_MAC)
    daemonize(verbose);
    #endif

    signal(SIGINT, nonstop_reset_handler);
    signal(SIGTERM, nonstop_reset_handler);

    /* The loop runs until a signal handler resets the variable */
    nonstop = 1; /* set to 1 only here */
    if(pid == QUADCAST_2S_PID) {
#ifdef OS_MAC
        if(mh->is_hidapi) {
            if(pck_cnt > QS2S_SOLID_PKT_CNT) {
                int num_frames = pck_cnt / QS2S_SOLID_PKT_CNT;
                while(nonstop) {
                    int frame;
                    for(frame = 0; frame < num_frames && nonstop; frame++) {
                        hidapi_qs2s_display_data_arr(mh->hid,
                            *data_arr + frame * QS2S_SOLID_PKT_CNT
                                             * DATA_PACKET_SIZE,
                            QS2S_SOLID_PKT_CNT);
                        usleep(QS2S_CYCLE_FRAME_DELAY);
                    }
                }
            } else {
                while(nonstop)
                    hidapi_qs2s_display_data_arr(mh->hid, *data_arr, pck_cnt);
            }
        } else
#endif
        {
            if(pck_cnt > QS2S_SOLID_PKT_CNT) {
                int num_frames = pck_cnt / QS2S_SOLID_PKT_CNT;
                while(nonstop) {
                    int frame;
                    for(frame = 0; frame < num_frames && nonstop; frame++) {
                        qs2s_display_data_arr(mh->usb,
                            *data_arr + frame * QS2S_SOLID_PKT_CNT
                                             * DATA_PACKET_SIZE,
                            QS2S_SOLID_PKT_CNT);
                        usleep(QS2S_CYCLE_FRAME_DELAY);
                    }
                }
            } else {
                while(nonstop)
                    qs2s_display_data_arr(mh->usb, *data_arr, pck_cnt);
            }
        }
    } else {
        short command_cnt;
        command_cnt = count_color_commands(data_arr, pck_cnt, 0);
        while(nonstop)
            display_data_arr(mh->usb, *data_arr,
                                            *data_arr+2*BYTE_STEP*command_cnt);
    }
}

#if !defined(DEBUG) && !defined(OS_MAC)
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
        usleep(DISPLAY_MODE_SLEEP_TIME);
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

static void qs2s_display_data_arr(libusb_device_handle *handle,
                                           const byte_t *data_arr, int pck_cnt)
{
    int pck = 0, errcode;
    byte_t header_packet[PACKET_SIZE], packet[PACKET_SIZE];

    memset(header_packet, 0, PACKET_SIZE);
    header_packet[0] = QS2S_DISPLAY_CODE;
    header_packet[1] = QS2S_PACKET_CNT_CODE;
    header_packet[2] = pck_cnt;
    errcode = qs2s_send_display_command(header_packet, handle);
    usleep(QS2S_DISPLAY_SLEEP_TIME);

    if(errcode)
        nonstop = 0;
    for(; pck < pck_cnt && nonstop; pck++) {
        memcpy(packet, data_arr + pck*DATA_PACKET_SIZE, DATA_PACKET_SIZE);
        errcode = send_interrupt_with_rsp(handle, packet, QS2S_EDP_OUT,
                                                                  QS2S_EDP_IN);
        if(errcode) {
            nonstop = 0; break;
        }
        #ifdef DEBUG
        print_packet(packet, "Data:");
        #endif
        usleep(QS2S_DISPLAY_SLEEP_TIME);
    }
}

static int qs2s_send_display_command(byte_t *packet,
                                                  libusb_device_handle *handle)
{
    int errcode;
    errcode = send_interrupt_with_rsp(handle, packet, QS2S_EDP_OUT,
                                                                  QS2S_EDP_IN);
    #ifdef DEBUG
    print_packet(packet, "Header display:");
    #endif
    return errcode;
}

static int send_interrupt_with_rsp(libusb_device_handle *handle, byte_t *pck,
                                                         byte_t out, byte_t in)
{ /* use in QS2S protocol only */
    int errcode, o_transferred, i_transferred;
    byte_t rsp[PACKET_SIZE];

    errcode = libusb_interrupt_transfer(handle, out, pck,
                                         PACKET_SIZE, &o_transferred, TIMEOUT);
    if(errcode) {
        fprintf(stderr, INTERRUPT_CMD_ERR_MSG, out, libusb_strerror(errcode));
        return errcode;
    }
    if(o_transferred != PACKET_SIZE) {
        fprintf(stderr, "Short command transfer on EDP %x: %d/%d\n", out,
                                                   o_transferred, PACKET_SIZE);
    }
    errcode = libusb_interrupt_transfer(handle, in, rsp,
                                         PACKET_SIZE, &i_transferred, TIMEOUT);
    if(errcode) {
        fprintf(stderr, INTERRUPT_RSP_ERR_MSG, in, libusb_strerror(errcode));
        return errcode;
    }
    if(i_transferred != PACKET_SIZE) {
        fprintf(stderr, "Short response transfer on EDP %x: %d/%d\n", in,
                                                   i_transferred, PACKET_SIZE);
    }

    return qs2s_rsp_check(pck, rsp);
}

static int qs2s_rsp_check(const byte_t *cmd, const byte_t *rsp)
{
    if (rsp[0] != QS2S_RESPONSE_CODE) {
        fprintf(stderr, "Response code mismatch: %x instead of %x\n", rsp[0],
                                                           QS2S_RESPONSE_CODE);
        return 1;
    } else if (rsp[14] != cmd[0]) {
        fprintf(stderr, "Response command mismatch: %x instead of %x\n",
                                                              rsp[14], cmd[0]);
        return 2;
    }
    return 0;
}

#ifdef OS_MAC
static hid_device *hidapi_open_qs2s(void)
{
    struct hid_device_info *devs, *cur;
    hid_device *dev = NULL;
    unsigned short vids[] = { DEV_VID_HP, DEV_VID_KINGSTON };
    char dev_path[512] = {0};
    int v, attempt;

    hid_init();
    hid_darwin_set_open_exclusive(0);

    /* Find the HID collection on Interface 1 (Generic Desktop / Mouse)
     * which contains the vendor-defined QS2S protocol with report IDs.
     * macOS 26+ exposes each USB interface's HID collections as separate
     * IOHIDDevices; the QS2S LED commands use report ID 0x44 which lives
     * on Interface 1 (PrimaryUsagePage=1 / Mouse), not Interface 0. */
    for(v = 0; v < 2 && !dev_path[0]; v++) {
        devs = hid_enumerate(vids[v], QUADCAST_2S_PID);
        for(cur = devs; cur; cur = cur->next) {
            #ifdef DEBUG
            printf("HIDAPI enum: vid=%04x pid=%04x iface=%d "
                   "usage_page=0x%04x usage=0x%04x path=%s\n",
                   cur->vendor_id, cur->product_id,
                   cur->interface_number, cur->usage_page,
                   cur->usage, cur->path);
            #endif
            if(cur->usage_page == 0x0001 && cur->usage == 0x0002) {
                snprintf(dev_path, sizeof(dev_path), "%s", cur->path);
                break;
            }
        }
        hid_free_enumeration(devs);
    }

    if(!dev_path[0]) {
        hid_exit();
        return NULL;
    }

    /* The device may need time to reset after a previous session.
     * Probe with a real QS2S command and close/reopen until
     * the device actually responds. */
    for(attempt = 0; attempt < 10; attempt++) {
        byte_t probe_buf[PACKET_SIZE + 1] = {0};
        byte_t rsp[PACKET_SIZE];
        int written, nread;

        dev = hid_open_path(dev_path);
        if(!dev) {
            #ifdef DEBUG
            printf("HIDAPI: failed to open (attempt %d)\n", attempt + 1);
            #endif
            usleep(3000000);
            continue;
        }

        /* Probe: QS2S display header with 0 data packets.
         * pck[0] is the report ID (0x44 = QS2S_DISPLAY_CODE) which
         * maps to the vendor-defined report on Interface 1. */
        probe_buf[0] = QS2S_DISPLAY_CODE;
        probe_buf[1] = QS2S_PACKET_CNT_CODE;

        written = hid_write(dev, probe_buf, PACKET_SIZE);
        if(written >= 0) {
            nread = hid_read_timeout(dev, rsp, PACKET_SIZE, TIMEOUT);
            if(nread > 0) {
                #ifdef DEBUG
                printf("HIDAPI: device ready after %d attempt(s)\n",
                       attempt + 1);
                #endif
                usleep(200000); /* let device settle */
                return dev;
            }
        }

        #ifdef DEBUG
        printf("HIDAPI: device not ready (attempt %d)%s\n", attempt + 1,
               written < 0 ? " [write failed]" : " [read timeout]");
        #endif
        hid_close(dev);
        dev = NULL;
        usleep(3000000);
    }

    hid_exit();
    return NULL;
}

static int hidapi_send_interrupt_with_rsp(hid_device *dev, byte_t *pck)
{
    byte_t buf[PACKET_SIZE];
    byte_t rsp[PACKET_SIZE];
    int written, nread;

    /* pck[0] is the report ID (e.g. 0x44 for QS2S display commands).
     * Interface 1 uses numbered reports; the first byte of every QS2S
     * packet is naturally the report ID on the wire. */
    buf[0] = pck[0];
    memcpy(buf + 1, pck + 1, PACKET_SIZE - 1);

    written = hid_write(dev, buf, PACKET_SIZE);
    if(written < 0) {
        if(nonstop)
            fprintf(stderr, "HIDAPI write error: %ls\n", hid_error(dev));
        return -1;
    }

    nread = hid_read_timeout(dev, rsp, PACKET_SIZE, TIMEOUT);
    if(nread == 0) {
        /* First command after warmup may still timeout; retry once */
        usleep(200000);
        written = hid_write(dev, buf, PACKET_SIZE);
        if(written >= 0)
            nread = hid_read_timeout(dev, rsp, PACKET_SIZE, TIMEOUT);
    }
    if(nread < 0) {
        if(nonstop)
            fprintf(stderr, "HIDAPI read error: %ls\n", hid_error(dev));
        return -1;
    }
    if(nread == 0) {
        if(nonstop)
            fprintf(stderr, "HIDAPI read timeout\n");
        return -1;
    }
    if(nread != PACKET_SIZE) {
        fprintf(stderr, "Short HIDAPI response: %d/%d\n", nread, PACKET_SIZE);
    }

    return qs2s_rsp_check(pck, rsp);
}

static int hidapi_qs2s_send_display_command(byte_t *packet, hid_device *dev)
{
    int errcode;
    errcode = hidapi_send_interrupt_with_rsp(dev, packet);
    #ifdef DEBUG
    print_packet(packet, "Header display (HIDAPI):");
    #endif
    return errcode;
}

static void hidapi_qs2s_display_data_arr(hid_device *dev,
                                         const byte_t *data_arr, int pck_cnt)
{
    int pck = 0, errcode;
    byte_t header_packet[PACKET_SIZE], packet[PACKET_SIZE];

    memset(header_packet, 0, PACKET_SIZE);
    header_packet[0] = QS2S_DISPLAY_CODE;
    header_packet[1] = QS2S_PACKET_CNT_CODE;
    header_packet[2] = pck_cnt;
    errcode = hidapi_qs2s_send_display_command(header_packet, dev);
    usleep(QS2S_DISPLAY_SLEEP_TIME);

    if(errcode)
        nonstop = 0;
    for(; pck < pck_cnt && nonstop; pck++) {
        memcpy(packet, data_arr + pck*DATA_PACKET_SIZE, DATA_PACKET_SIZE);
        errcode = hidapi_send_interrupt_with_rsp(dev, packet);
        if(errcode) {
            nonstop = 0; break;
        }
        #ifdef DEBUG
        print_packet(packet, "Data (HIDAPI):");
        #endif
        usleep(QS2S_DISPLAY_SLEEP_TIME);
    }
}
#endif

void close_mic(struct mic_handle *mh)
{
#ifdef OS_MAC
    if(mh->is_hidapi) {
        hid_close(mh->hid);
        hid_exit();
        return;
    }
#endif
    libusb_release_interface(mh->usb, 0);
    libusb_release_interface(mh->usb, 1);
    libusb_close(mh->usb);
    libusb_exit(NULL);
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
