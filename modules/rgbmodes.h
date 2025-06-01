/* quadcastrgb - set RGB lights of HyperX Quadcast S and DuoCast
 * File rgbmodes.h
 * Assembles data packets from "colorschemes" structure.
 * parse_colorscheme returns pointer to the array of data packets.
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
#ifndef RGBMODES_SENTRY
#define RGBMODES_SENTRY

#include "argparser.h" /* for struct colschemes, strequ, enums */

/* Constants */
#define MAX_PCT_COUNT 90
#define COLPAIR_PER_PCT 8
#define MAX_COLPAIR_COUNT (COLPAIR_PER_PCT * MAX_PCT_COUNT)

#define DATA_PACKET_SIZE 64
#define BYTE_STEP 4 /* used to skip some part of bytes in a packet */
#define RGB_CODE 0x81

/* Macros */
#define DIV_CEIL(X, Y) (((X)/(Y)) + ((X)%(Y) != 0))
#define SPEED_RANGE(MIN, MAX, SPD) MIN + (MAX - MIN)*(100-SPD)/100
/* Blink random */
#define MAX_SPD 101
#define MAX_DLY 100
#define RAND_COL_SEG_MAX 51
#define RAND_DLY_SEG_MAX 51
#define RAND_COL_SEG_MIN 5
#define RAND_DLY_SEG_MIN 2
/* Cycle */
#define MIN_CYCL_TR 12
#define MAX_CYCL_TR 128
/* Lightning */
#define MIN_LGHT_BL 1
#define MAX_LGHT_BL 9
#define MIN_LGHT_UP 3
#define MAX_LGHT_UP 10
#define MIN_LGHT_DOWN 21
#define MAX_LGHT_DOWN 131

/* Messages */
#define NOSUPPORT_MSG _("The mode is not supported yet.\n")
#define QS_2S_NOSUPPORT_MSG _("No support for %s on Quadcast 2S yet\n")

/* Types */
typedef unsigned char byte_t;
typedef byte_t datpack[DATA_PACKET_SIZE];

/* Functions */
datpack *parse_colorscheme(struct colschemes *cs, int *pck_cnt);
short count_color_commands(const datpack *data_arr, int pck_cnt, int colgroup);

#endif
