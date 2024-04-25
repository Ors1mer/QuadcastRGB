/* quadcastrgb - set RGB lights of HyperX Quadcast S and DuoCast
 * File locale_macros.h
 * Stuff for proper work of gettext in order to achieve
 * multi-language support.
 * This is a header without the source file (because it isn't needed).
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
/*
#include <locale.h>
#include <libintl.h>
*/

/* Constants */
/*
#define LOCALEBASEDIR "../lang"
#define TEXTDOMAIN "hyperrgb"
*/

/* Macros */
#define _(STR) (STR) /* the string is language-dependent */
#define N_(STR) (STR) /* it isn't */
