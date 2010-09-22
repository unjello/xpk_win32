/* sublibs.c -- Handling of xpk sublibraries
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

#include "config.h"
#include <stdio.h>
#include "xpkmaster.h"
#include "texts.h"

typedef struct XpkInfo *(*XpksPackerInfo_t)();

/* Open sublib from ID */
void *opensub(struct XpkBuffer *xbuf, unsigned int ID)
{
  HMODULE XpkSubBase;
  XpksPackerInfo_t XpksPackerInfo;
  char libname[SUBLIBNAME_SIZE];

  /* Do nothing if we already have what we want */
  if((xbuf->xb_SubBase) && (xbuf->xb_SubID==ID))
    return xbuf->xb_SubBase;

  closesub(xbuf);

	xbuf->xb_SubID = ID;
	sprintf(libname, SUBLIBNAME_STRING, (char*)&xbuf->xb_SubID);
	if(!(xbuf->xb_SubBase = XpkSubBase = LoadLibrary(libname))) 
		xbuf->xb_Result = XPKERR_MISSINGLIB;
	else {
		XpksPackerInfo=(XpksPackerInfo_t)GetProcAddress(XpkSubBase,"LIBXpksPackerInfo");

		if(((xbuf->xb_SubInfo=XpksPackerInfo())->xi_MasterVersion) > XPK_MAJOR_VERSION) {
			xbuf->xb_Result = XPKERR_OLDMASTLIB;
			closesub(xbuf);
	    }
	}
	
	return xbuf->xb_SubBase;
}

/* Close any open sublibrary */
void closesub(struct XpkBuffer *xbuf)
{
  if(xbuf->xb_SubBase) {
#ifdef _DEBUG
    DebugRunTime("closesub: closing lib %.4s", &xbuf->xb_SubID);
#endif
    FreeLibrary(xbuf->xb_SubBase);
    xbuf->xb_SubBase = 0;
  }
}

/* Get ID number from string */
static char xpkupper(char c)
{
  if(c>='a' && c<='z')
    c-='a'-'A';

  return c;
}

unsigned int idfromname(char *name)
{
  unsigned int i, j=0;

  for(i=4; i; i--) {
    j<<=8;
    j+=xpkupper(*(name++));
  }

#ifndef WORDS_BIGENDIAN
  j=_byteswapint32(j);
#endif

  return j;
}
