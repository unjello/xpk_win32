/* hook_mem.c -- Memory IO hooks
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
#include <memory.h>
#include "xpkmaster.h"

/* Read from memory hook */
static int meminfunc(struct XpkMasterMsg *msg)
{
  char *bufpos;
  int ofs;

  bufpos=(char *)msg->xmm_Buf+msg->xmm_BufOfs;

  switch (msg->xmm_Type)
  {
  case XIO_READ:
    if(msg->xmm_BufOfs+msg->xmm_Size>msg->xmm_Len)
      return XPKERR_TRUNCATED;
    msg->xmm_BufOfs+=msg->xmm_Size;
    if(!msg->xmm_Ptr)
      msg->xmm_Ptr=bufpos;
    else if(bufpos!=msg->xmm_Ptr)
      memcpy(msg->xmm_Ptr, bufpos, msg->xmm_Size);
    break;
  case XIO_SEEK:
    ofs=msg->xmm_BufOfs+msg->xmm_Size;
    if((ofs<0)||(ofs>msg->xmm_Len))
      return XPKERR_IOERRIN;
    /* preSEEK position. */
    msg->xmm_Size=msg->xmm_BufOfs;
    msg->xmm_BufOfs=ofs;
    break;
    /* case XIO_ABORT:
     * case XIO_FREE: */
  case XIO_TOTSIZE:
    /* always needed */
    return XPKERR_BADPARAMS;
    break;
  }

  return 0;
}

struct Hook meminhook={ {0}, (unsigned int (*) ()) meminfunc, 0, 0};

/* Write to memory hook */
static int memoutfunc(struct XpkMasterMsg *msg)
{
  char *bufpos=(char *)msg->xmm_Buf+msg->xmm_BufOfs;
  int ofs;

  switch (msg->xmm_Type)
  {
  case XIO_SEEK:
    ofs=msg->xmm_BufOfs+msg->xmm_Size;
    if((ofs < 0) || (ofs>msg->xmm_BufLen))
      return XPKERR_IOERROUT;
    /* preSEEK position. */
    msg->xmm_Size=msg->xmm_BufOfs;
    msg->xmm_BufOfs=ofs;
    break;
  case XIO_TOTSIZE:
    if(msg->xmm_Flags & XIO_GETOUTBUF) {
      if(!(msg->xmm_Buf=(char *) calloc(msg->xmm_Size, 1)))
	return XPKERR_NOMEM;
      msg->xmm_BufLen=msg->xmm_Size;
    } else if(!msg->xmm_Buf)
      return XPKERR_SMALLBUF;
    break;
  case XIO_GETBUF:
    if(msg->xmm_BufOfs+msg->xmm_Size>msg->xmm_BufLen)
      return XPKERR_SMALLBUF;
    msg->xmm_Ptr=bufpos;
    break;
  case XIO_WRITE:
    if(msg->xmm_BufOfs+msg->xmm_Size>msg->xmm_BufLen)
      return XPKERR_SMALLBUF;
    if(msg->xmm_Ptr && (msg->xmm_Ptr != bufpos))
      memcpy(bufpos, msg->xmm_Ptr, msg->xmm_Size);
    msg->xmm_BufOfs+=msg->xmm_Size;
    break;
  case XIO_ABORT:
    if((msg->xmm_Flags & XIO_GETOUTBUF) && msg->xmm_Buf) {
      free(msg->xmm_Buf);
      msg->xmm_Buf=0;
    }
    break;
  }

  return 0;
}

struct Hook memouthook={{0}, (unsigned int (*)()) memoutfunc, 0, 0};
