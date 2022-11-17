/* quadcastrgb - change RGB mode for the microphone HyperX Quadcast S
 * File main.c
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

#define LIBUSB_FREE_EVERYTHING() \
    libusb_release_interface(handle, 0); \
    libusb_release_interface(handle, 1); \
    libusb_close(handle); \
    libusb_exit(NULL)

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
    send_packets(handle, data_arr, data_packet_cnt);
    /* Free all memory */
    free(data_arr);
    LIBUSB_FREE_EVERYTHING();
    VERBOSE_PRINT(verbose, VERBOSE5_END);
    return 0;
}
