/* QUADCASTRGB; file main.c
 * The program is spread under the GPLv3 license.
 * The author is Ors1mer.
 */
#include <stdio.h>
#include "modules/locale_macros.h"
#include "modules/argparser.h"
#include "modules/rgbmodes.h"
#include "modules/devio.h"

#define LOCALESETUP() \
    setlocale(LC_CTYPE, ""); \
    setlocale(LC_MESSAGES, ""); \
    bindtextdomain(TEXTDOMAIN, LOCALEBASEDIR); \
    textdomain(TEXTDOMAIN)

#define VERBOSE_PRINT(V, MSG) \
    if(V) \
        puts(MSG)

#define VERBOSE1_ARG _("Arguments parsed successfully.")
#define VERBOSE2_COL _("Assembling data packets.")
#define VERBOSE3_MIC _("Opening the microphone descriptor.")
#define VERBOSE4_PKT _("Sending packets.")
#define VERBOSE5_END _("Done.")

int main(int argc, const char **argv)
{
    struct colschemes *cs;
    datpack *data_arr;
    libusb_device_handle *handle;
    int verbose = 0, data_packet_cnt;
    LOCALESETUP();
    /* Parse arguments */
    cs = parse_arg(argc, argv, &verbose);
    VERBOSE_PRINT(verbose, VERBOSE1_ARG);
    /* Create data packets */
    VERBOSE_PRINT(verbose, VERBOSE2_COL);
    data_arr = parse_colorscheme(cs, &data_packet_cnt);
    free(cs);
    /* Open the microphone */
    VERBOSE_PRINT(verbose, VERBOSE3_MIC);
    handle = open_micro(data_arr); /* data_arr for freeing memory */
    /* Send packets */
    VERBOSE_PRINT(verbose, VERBOSE4_PKT);
    send_startup_packets(handle, data_arr, data_packet_cnt);
    send_packets(handle, data_arr, data_packet_cnt);
    free(data_arr);
    libusb_close(handle);
    libusb_exit(NULL);
    VERBOSE_PRINT(verbose, VERBOSE5_END);
    return 0;
}
