/* xbuf.c -- Xpk buffer handling
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
#include <time.h>
#include "xpkmaster.h"

/************************* alloc and init xbuf ***************************/

struct XpkBuffer *initxbuf(void)
{
  struct XpkBuffer *xbuf;

  if(!(xbuf=(struct XpkBuffer *) calloc(sizeof(struct XpkBuffer), 1)))
    return 0;

  /* Save the original task priority in case we change it during an
     operation */
  xbuf->xb_Priority=GetPriorityClass(GetCurrentProcess());
  xbuf->xb_InLen=0xFFFFFFFF;

  return xbuf;
}

/* Free buffers */
int freebufs(struct XpkBuffer *xbuf)
{
  int error=xbuf->xb_Result;

  /* Free any open sub-library */
  closesub(xbuf);
  freeseek(xbuf);

  if(xbuf->xb_Flags & XMF_OWNTASKPRI)
    SetPriorityClass(GetCurrentProcess(), xbuf->xb_Priority);
  if(xbuf->xb_Flags & XMF_OWNPASSWORD)
    free(xbuf->xb_Password);
  
  /* Finally, free the handle itself */
  free(xbuf);
  return error;
}
