/* texts.c -- all library texts
 * Copyright (C) 1996-2003 authors
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

#include <sys/types.h>
#include "texts.h"

char *strings[LOCALE_STRINGCNT] = {
"Packing",
"Packed",
"Unpacking",
"Unpacked",
"Reading",
"Read",
"Aborted",
"Enter password",
"Enter %ld bit key",
"Preferences-configurable packing and encrypting.",
"Retype for verification",
};

char *XpkErrs[LOCALE_ERRSTRINGCNT] =
{
  "OK",
  "Feature not implemented in selected library",
  "Function may not be used with files",
  "Error reading input",
  "Error writing output",
  "Check sum failure",		/* 5 */
  "Library too old for this file",
  "Out of memory",
  "Library already in use",
  "Can't find decompressor for this format",
  "Output buffer too small",	/* 10 */
  "Input buffer too large",
  "This packing mode not supported",
  "Password required",
  "Input file is corrupt",
  "Can't find required XPK library",	/* 15 */
  "Bad internal parameters",
  "Data cannot be compressed",
  "Requested compression method not found",
  "Operation aborted by user",
  "Input file truncated",	/* 20 */
  "MC68020 or better required",
  "Data already compressed",
  "Data not compressed",
  "Output file already exists",
  "Master library too old",	/* 25 */
  "Sub library too old",
  "This library cannot encrypt",
  "Can't get info",
  "This compression method is lossy",
  "Compression hardware required",	/* 30 */
  "Compression hardware failed",
  "Password incorrect",
  "Unknown error",
  "Automatic request time out",
};
