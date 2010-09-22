/* objects.c -- object allocation functions and prefs sem get funcs
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
#include <sys/types.h>
#include "xpk.h"
#include "xpkprefs.h"
#include "xpkmaster.h"

static int GetXpkObjectSize(unsigned int type)
{
  switch(type) {
  case XPKOBJ_FIB:
    return sizeof(struct XpkFib);
    break;
  case XPKOBJ_PACKERINFO:
    return sizeof(struct XpkPackerInfo);
    break;
  case XPKOBJ_MODE:
    return sizeof(struct XpkMode);
    break;
  case XPKOBJ_PACKERLIST:
    return sizeof(struct XpkPackerList);
    break;
  };
  return 0;
}

char *XpkAllocObject(unsigned int type, struct TagItem *tags)
{
  int size;
  
  if((size=GetXpkObjectSize(type)))
    return (char*)calloc(size, 1);

  return 0;
}

void XpkFreeObject(unsigned int type, char *object)
{
  int size;
  
  if((size=GetXpkObjectSize(type)))
    free(object);
}
