/* checksum.c -- Simple checksum routines
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

#include "xpkmaster.h"

unsigned char hchecksum(char *ptr, unsigned int count)
{
  unsigned char sum=0;

  while(count-->0)
    sum^=*ptr++;

  return sum;
}

unsigned short cchecksum(unsigned int *ptr, unsigned int count)
{
  unsigned int sum = 0;

  while(count-->0)
    sum ^= *ptr++;

  return (unsigned short) (sum^(sum>>16));
}
