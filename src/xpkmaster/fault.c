/* fault.c -- Error message generators
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

#include <stdio.h>
#include <string.h>
#include "xpkmaster.h"
#include "texts.h"

char XpkPrintFault(int code, char *header)
{
  int ret;
   
  if(code>0 || code<MINERROR)
    code=XPKERR_UNKNOWN;
 
  if(header)
    ret=printf("%s: %s\n", header, XpkErrs[-code]);
  else
    ret=printf("%s\n",XpkErrs[-code]);
  
  if(ret==-1)
    /* error */
    return 0;
  else
    /* ok */
    return -1;
}

unsigned int XpkFault(int code, char *header, char *buffer, unsigned int size)
{
  unsigned int ssize=0;

  if(size > 1 && buffer) {
    char *string;

    if(code > 0 || code < MINERROR)
      code=XPKERR_UNKNOWN;

    string=XpkErrs[-code];

    /* remove 1 for 0-byte from size */
    if((ssize=strlen(string)) > --size)
      ssize=size;
    size -= ssize;

    if(header && (code=strlen(header)) + 2 <= size) {
      memcpy(buffer, header, code);
      buffer[code++]=':';
      buffer[code++]=' ';
      buffer += code;
    } else
      code=0;

    memcpy(buffer, string, ssize);
    buffer[ssize]=0;
    ssize += code;
  }

  return ssize;
}
