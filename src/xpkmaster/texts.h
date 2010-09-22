/* texts.h -- Defines for all texts of the library
 * Copyright (C) 1996-1999 Dirk Stöcker
 * This file is part of the xpk package.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be <,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA.
 */

/* Written by Dirk Stöcker <stoecker@amigaworld.com>
 * UNIX version by Vesa Halttunen <vesuri@jormas.com>
 * Win32 version by Andrzej Lichnerowicz <angelo@irc.pl>
 */

#ifndef _TEXTS_H_
#define _TEXTS_H_

#define LOCALE_STRINGCNT		 11
#define LOCALE_STRINGSTART		  0
#define LOCALE_ERRSTRINGCNT		 35
#define LOCALE_ERRSTRINGSTART		200

#define MINERROR (1-LOCALE_ERRSTRINGCNT)

extern char *strings[LOCALE_STRINGCNT];
extern char *XpkErrs[LOCALE_ERRSTRINGCNT];

#define SUBLIBNAME_STRING	"xpk_%.4s.dll"
#define SUBLIBNAME_SIZE		14

#define TXT_PACKING_UPPER		  0
#define TXT_PACKED			  1
#define TXT_UNPACKING_UPPER		  2
#define TXT_UNPACKED			  3
#define TXT_READING			  4
#define TXT_READ			  5
#define TXT_ABORTED			  6

#define TXT_REQ_PASSWORD		  7
#define TXT_REQ_KEY			  8
#define TXT_USER_DESCRIPTION		  9
#define	TXT_VERIFY_PASS			 10

#endif /* XPKMASTER_TEXTS_H */
