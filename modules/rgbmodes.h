/* rgbmodes.h
 * Assembles data packets from "colorschemes" structure.
 * The only function returns pointer to the array of data packets.
 */
#ifndef RGBMODES_SENTRY
#define RGBMODES_SENTRY

#include <stdio.h> /* for fprintf */
#include "argparser.h" /* for struct colschemes, strequ, enums */

/* Constants */
#define MAX_PCT_COUNT 90
#define COLPAIR_PER_PCT 8
#define MAX_COLPAIR_COUNT (COLPAIR_PER_PCT * MAX_PCT_COUNT)

#define DATA_PACKET_SIZE 64
#define BYTE_STEP 4 /* Used to skip some part of bytes in a packet */
#define RGB_CODE 0x81

/* Messages */
#define NOSUPPORT_MSG _("The mode not supported yet.\n")

/* Types */
typedef unsigned char byte_t;
typedef byte_t datpack[DATA_PACKET_SIZE];

/* Functions */
datpack *parse_colorscheme(struct colschemes *cs, int *pck_cnt);

#endif
