/* hook_fh.c -- uses new style register definition
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
#include <stdlib.h>
#include <errno.h>
#include "xpkmaster.h"

static int allociobuf(struct XpkMasterMsg *msg);
static void freeiobuf(struct XpkMasterMsg *msg);

/* Read from file handle hook */
static int fhinfunc(struct XpkMasterMsg *msg)
{
  int wanted, epos;
  int i;

  switch(msg->xmm_Type) {
  case XIO_SEEK:
    i=msg->xmm_Size;
    /* FIX: ERROR CHECKING NEEDED */
    msg->xmm_Size=ftell(msg->xmm_FH);
    if((fseek(msg->xmm_FH, i, SEEK_CUR))<0)
      return XPKERR_IOERRIN;
    break;
  case XIO_GETBUF:
    if(allociobuf(msg))
      return XPKERR_NOMEM;
    msg->xmm_Ptr=msg->xmm_Buf;
    break;
  case XIO_READ:
    if(!msg->xmm_Ptr) {
      if(allociobuf(msg))
        return XPKERR_NOMEM;
      msg->xmm_Ptr=msg->xmm_Buf;
    }
    wanted = msg->xmm_Size;
    if((msg->xmm_Size = fread(msg->xmm_Ptr, 1, wanted, 
			      msg->xmm_FH)) != wanted)
      return msg->xmm_Size > 0 ? XPKERR_TRUNCATED : XPKERR_IOERRIN;
    break;
  case XIO_ABORT:
  case XIO_FREE:
    freeiobuf(msg);
    if(msg->xmm_Flags & XMF_PRIVFH) {
      fclose(msg->xmm_FH);
      msg->xmm_FH = 0;
    }
    break;
  case XIO_TOTSIZE:
    wanted=ftell(msg->xmm_FH);
    fseek(msg->xmm_FH, 0, SEEK_END);
    epos=ftell(msg->xmm_FH);
    fseek(msg->xmm_FH, wanted, SEEK_SET);

    if(wanted<0 || epos<0)
      return XPKERR_IOERRIN;
    msg->xmm_Size = epos-wanted;
    break;
  }
  return 0;
}

struct Hook fhinhook = { {0}, (unsigned int (*)()) fhinfunc, 0, 0 };

/* Write to file handle hook */
static int fhoutfunc(struct XpkMasterMsg *msg)
{
  int wanted;

  switch(msg->xmm_Type)
  {
  case XIO_GETBUF:
    if(allociobuf(msg))
      return XPKERR_NOMEM;
    msg->xmm_Ptr = msg->xmm_Buf;
    break;
  case XIO_WRITE:
    wanted = msg->xmm_Size;
    if((msg->xmm_Size = fwrite(msg->xmm_Ptr, 1, wanted,
			       msg->xmm_FH)) != wanted)
      return XPKERR_IOERROUT;
    break;
  case XIO_SEEK:
    if((msg->xmm_Size = fseek(msg->xmm_FH, msg->xmm_Size, SEEK_CUR))<0)
      return XPKERR_IOERROUT;
    break;
  case XIO_ABORT:
  case XIO_FREE:
    if(msg->xmm_Flags & XMF_PRIVFH) {
      fclose(msg->xmm_FH);
      msg->xmm_FH = 0;
    }
    freeiobuf(msg);
    if(msg->xmm_Type == XIO_ABORT && msg->xmm_FileName)
      unlink(msg->xmm_FileName);
    break;
  }
  return 0;
}

struct Hook fhouthook = { {0}, (unsigned int (*) ()) fhoutfunc,0 ,0};

/* Free file handle I/O buffer */
static void freeiobuf(struct XpkMasterMsg *msg)
{
  /* clear buffer, when exists */
  if(msg->xmm_BufLen) {
    free(msg->xmm_Buf);
    msg->xmm_BufLen = 0;
  }
}

/* Allocate file handle I/O buffer */
static int allociobuf(struct XpkMasterMsg *msg)
{
  unsigned int buflen = msg->xmm_Size;

  /* buffer is large enough */
  if(msg->xmm_BufLen >= buflen)
    return 0;

  /* clear old buffer */
  freeiobuf(msg);

  /* get new one */
  if(!(msg->xmm_Buf = (char *)calloc(buflen, 1)))
    return XPKERR_NOMEM;

  /* set correct data */
  msg->xmm_BufLen = buflen;

#ifdef _DEBUG
  DebugRunTime("allociobuf: allocated buffer at %lx of size %ld",
  msg->xmm_Buf, msg->xmm_BufLen);
#endif

  /* all ok */
  return 0;
}
