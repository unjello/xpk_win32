/* seek.c -- Implementation of XpkSeek
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

#include <stdlib.h>
#include "xpkmaster.h"

/* get chunk header and and seek data when necessary */
int addseek(struct XpkBuffer *xbuf)
{
  int ulen;
  struct SeekDataList *sdl;

  if(!(xbuf->xb_Flags & XMF_SEEK))
    return 0;

  ulen = xbuf->xb_UCur;

  /* check if already included */
  sdl = xbuf->xb_SeekDataList;
  while(sdl && sdl->sdl_Next)
    sdl = sdl->sdl_Next;
  if(sdl && sdl->sdl_Data[sdl->sdl_Used-1].sd_ULen >= ulen)
    /* already done, so we quit */
    return 0;
  if(!sdl || sdl->sdl_Used == SEEKENTRYNUM)
  {
    struct SeekDataList *sdl2;
    if(!(sdl2 = (struct SeekDataList *)
    calloc(sizeof(struct SeekDataList), 1)))
      return (xbuf->xb_Result = XPKERR_NOMEM);
    if(!xbuf->xb_SeekDataList)
      xbuf->xb_SeekDataList = sdl2;
    else
      sdl->sdl_Next = sdl2;
    sdl = sdl2;
  }

  sdl->sdl_Data[sdl->sdl_Used].sd_FilePos = xbuf->xb_InBufferPos;
  sdl->sdl_Data[sdl->sdl_Used].sd_ULen = ulen;
  sdl->sdl_Data[(sdl->sdl_Used)++].sd_CLen = xbuf->xb_CCur;

#ifdef _DEBUG
  DebugRunTime("addseek: added entry (pos %ld, ulen %ld)",
  xbuf->xb_InBufferPos, ulen);
#endif

  return 0;
}

void freeseek(struct XpkBuffer *xbuf)
{
  struct SeekDataList *sdl, *sdl2;
  
  sdl = xbuf->xb_SeekDataList;
  while(sdl)
  {
    sdl2 = sdl->sdl_Next;
    free(sdl);
    sdl = sdl2;
  }
  xbuf->xb_SeekDataList = 0;
}

int doseek(struct XpkBuffer *xbuf, int pos)
{
  int ulen, clen;
  XpkChunkHeader *lochdr = &(xbuf->xb_Headers.h_Loc);
  struct SeekDataList *sdl;

  if(pos > xbuf->xb_Fib.xf_ULen)
    return XPKERR_BADPARAMS;

  if(xbuf->xb_Format == XPKMODE_UPUP)
  {
    xbuf->xb_Flags &= ~XMF_EOF;

    if(!(hookread(xbuf, XIO_SEEK, 0, pos - xbuf->xb_InBufferPos)))
      return xbuf->xb_Result;

    xbuf->xb_Fib.xf_CCur = xbuf->xb_Fib.xf_UCur = xbuf->xb_InBufferPos;
    xbuf->xb_Fib.xf_NLen = Min(xbuf->xb_InLen - xbuf->xb_Fib.xf_UCur,
    DEFAULTCHUNKSIZE) + XPK_MARGIN;

    return 0;
  }

  for(sdl = xbuf->xb_SeekDataList; sdl; sdl = sdl->sdl_Next) {
    int i;
    for(i = 0; i < sdl->sdl_Used; ++i) {
      if(sdl->sdl_Data[i].sd_ULen > pos) {
        if(!(hookread(xbuf, XIO_SEEK, 0, sdl->sdl_Data[i].sd_FilePos -
        xbuf->xb_Headers.h_LocSize - xbuf->xb_InBufferPos)))
	  return xbuf->xb_Result;
        if(!(hookread(xbuf, XIO_READ, lochdr, xbuf->xb_Headers.h_LocSize)))
          return xbuf->xb_Result;
        getUClen(xbuf, &ulen, &clen);
        xbuf->xb_UCur = sdl->sdl_Data[i].sd_ULen - ulen;
        xbuf->xb_CCur = sdl->sdl_Data[i].sd_CLen - clen;
        updatefib(xbuf);
        return (int)(pos - xbuf->xb_Fib.xf_UCur);
      }
    }
  }

  /* This is called when a forward seek is needed and we do not have the seek
     entries */
  while(xbuf->xb_UCur <= pos) {
    if(lochdr->xch_Word.xchw_Type == XPKCHUNK_END)
      return XPKERR_BADPARAMS;

    getUClen(xbuf, &ulen, &clen);
    if(!(hookread(xbuf, XIO_SEEK, 0, ROUNDLONG(clen))))
      return xbuf->xb_Result;
    if(!(hookread(xbuf, XIO_READ, lochdr, xbuf->xb_Headers.h_LocSize)))
      return xbuf->xb_Result;
    if(updatefib(xbuf))
      return xbuf->xb_Result;
  }
  return (int) (pos - xbuf->xb_Fib.xf_UCur);
}

/*
 *   XpkSeek() - move around on a compressed file
 */

/* Return codes < 0 are error codes. Codes >= 0 are file position */
int XpkSeek(struct XpkBuffer *xbuf, int dist, int mode)
{
  int err = XPKERR_BADPARAMS;

  if((xbuf->xb_Format != XPKMODE_UPSTD && xbuf->xb_Format != XPKMODE_UPUP) ||
     (xbuf->xb_SubInfo->xi_Flags & (XPKIF_NOSEEK|XPKIF_PREREADHDR)))
    err = XPKERR_NOFUNC;
  else if(xbuf->xb_Flags & XMF_SEEK) {
    switch(mode) {
    case XPKSEEK_CURRENT:
      err = doseek(xbuf, xbuf->xb_Fib.xf_UCur + dist);
      break;
    case XPKSEEK_BEGINNING:
      err = doseek(xbuf, dist);
      break;
    case XPKSEEK_END:
      err = doseek(xbuf, xbuf->xb_Fib.xf_ULen + dist);
      break;
      /*  default: err = XPKERR_BADPARAMS; break; */
    }
#ifdef _DEBUG
    DebugRunTime("XpkSeek: position after seek (pos %ld, ulen %ld)",
		 xbuf->xb_InBufferPos, xbuf->xb_Fib.xf_UCur);
#endif
  }

  return err;
}
