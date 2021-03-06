/* fibc.c -- FIB related functions
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

/* Written by Dirk St�cker <stoecker@amigaworld.com>
 * UNIX version by Vesa Halttunen <vesuri@jormas.com>
 * Win32 version by Andrzej Lichnerowicz <angelo@irc.pl>
 */

#include <stdio.h>
#include <string.h>
#include "xpkmaster.h"

void getUClen(struct XpkBuffer *xbuf, int *ulen, int *clen)
{
  if(xbuf->xb_Headers.h_Glob.xsh_Flags & XPKSTREAMF_LONGHEADERS) {
    *ulen=xbuf->xb_Headers.h_Loc.xch_Long.xchl_ULen;
    *clen=xbuf->xb_Headers.h_Loc.xch_Long.xchl_CLen;
  } else {
    *ulen=xbuf->xb_Headers.h_Loc.xch_Word.xchw_ULen;
    *clen=xbuf->xb_Headers.h_Loc.xch_Word.xchw_CLen;
  }
}

int updatefib(struct XpkBuffer *xbuf)
{
  struct XpkStreamHeader *globhdr=&xbuf->xb_Headers.h_Glob;
  struct XpkFib *fib=&xbuf->xb_Fib;
  int ulen, clen;

  getUClen(xbuf, &ulen, &clen);

  fib->xf_Type=XPKTYPE_PACKED;
  fib->xf_CLen=globhdr->xsh_CLen+8;
  fib->xf_ULen=globhdr->xsh_ULen;
  fib->xf_NLen=ulen+XPK_MARGIN;
  fib->xf_CCur=xbuf->xb_CCur;
  fib->xf_UCur=xbuf->xb_UCur;
  xbuf->xb_CCur+=ROUNDLONG(clen)+xbuf->xb_Headers.h_LocSize;
  xbuf->xb_UCur+=ulen;
  fib->xf_ID=globhdr->xsh_Type;
  fib->xf_SubVersion=globhdr->xsh_SubVrs;
  fib->xf_MasVersion=globhdr->xsh_MasVrs;
  memcpy(fib->xf_Head, globhdr->xsh_Initial, 16);

  percentages(fib);
  return addseek(xbuf);
}

void percentages(struct XpkFib *fib)
{
  fib->xf_Ratio=0;
  if(fib->xf_ULen)
    fib->xf_Ratio=100 - 100 * fib->xf_CLen / fib->xf_ULen;
  if(fib->xf_Ratio < 0)
    fib->xf_Ratio=0;
  *(int *)fib->xf_Packer=fib->xf_ID;
  fib->xf_Packer[4]=0;
}
