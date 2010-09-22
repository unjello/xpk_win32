/* hook.c -- Hook handling functions
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
#include "xpkmaster.h"

#ifdef _DEBUG
static char * action_names[8] =
{"<zero>", "XIO_READ", "XIO_WRITE", "XIO_FREE", "XIO_ABORT", "XIO_GETBUF",
"XIO_SEEK", "XIO_TOTSIZE" };
#endif

static char *callhook(struct XpkBuffer *xbuf, unsigned int action, char *buf,
		      int size, struct XpkMasterMsg *msg, struct Hook *hook)
{
  int res;

  msg->xmm_Type=action;
  msg->xmm_Ptr=(char *)buf;
  msg->xmm_Size=size;

  if(!hook) {
    xbuf->xb_Result=XPKERR_BADPARAMS;
    return 0;
  }

  if((res=(*(regfunc) hook->h_Entry) (msg))) {
    xbuf->xb_Result=res;
#ifdef _DEBUG
    DebugError("hook%s: %s <%ld> (%ld)", msg == &xbuf->xb_RMsg ? "read" :
    "write", action_names[(action&7)], action, xbuf->xb_Result);
#endif
  }

  if(xbuf->xb_Result)
    return 0;
  else if(msg->xmm_Ptr)
    return (char *) msg->xmm_Ptr;
  else
    /* SEEK may return 0 on success! */
    return (char *) -1;
}

/*************************** read from input hook ************************/

char *hookread(struct XpkBuffer *xbuf, unsigned int action, void *buf, int size)
{
  if(action == XIO_READ || action == XIO_SEEK)
    xbuf->xb_InBufferPos += size;
  return callhook(xbuf, action, (char*)buf, size, &xbuf->xb_RMsg, xbuf->xb_RHook);
}

/*************************** write to output hook ************************/

char *hookwrite(struct XpkBuffer *xbuf, unsigned int action, void *buf, int size)
{
  return callhook(xbuf, action, (char*)buf, size, &xbuf->xb_WMsg, xbuf->xb_WHook);
}
