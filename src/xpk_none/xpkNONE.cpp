/* xpkNONE.c -- This library is mainly intended to demonstrate how to program a
 *              sub library
 * Copyright (C) 1996-1999 Dirk Stöcker
 * This file is part of the xpk package.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
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

#include <string.h>
#include <sys/types.h>
#include "../xpkmaster/xpksub.h"

static const struct XpkMode NONEMode = {
  0,          /* Next mode */
  100,        /* Handles up to */
  XPKMF_A3000SPEED, /* Flags */
  0,          /* Packing memory */
  0,          /* Unpacking memory */
  1918,       /* Packing speed */
  2477,       /* Unpacking speed */
  0,          /* Compression ratio */
  0,          /* Reserved */
  "normal",   /* Description */
};

static const struct XpkInfo NONEInfo = {
  1,               /* info version */
  1,               /* lib  version */
  0,               /* master vers  */
  0,               /* ModesVersion */
  "NONE"  ,        /* short name   */
  "NoPacker",      /* long name    */
  "A dummy library that does no compression", /* Description  */
  0x4E4F4E45,      /* 4 letter ID  */
  XPKIF_PK_CHUNK | /* flags        */
  XPKIF_UP_CHUNK,
  32768,           /* max in chunk */
  20,              /* min in chunk */
  32768,           /* def in chunk */
  "Writing",       /* pk message   */
  "Reading",       /* up message   */
  "Wrote",         /* pk past msg  */
  "Read",          /* up past msg  */
  50,              /* DefMode      */
  0,               /* Pad          */
  (struct XpkMode *)&NONEMode, /* ModeDesc     */
  {0,}             /* reserved     */
};

/* returns an info structure about our packer */

extern "C" __declspec(dllexport) const struct XpkInfo * LIBXpksPackerInfo(void)
{
  return &NONEInfo;
}

/* compresssion and decompression stuff */

extern "C" __declspec(dllexport) int LIBXpksPackChunk(struct XpkSubParams *xpar)
{
  return XPKERR_EXPANSION;
}

extern "C" __declspec(dllexport) int LIBXpksUnpackChunk(struct XpkSubParams *xpar)
{
  memcpy(xpar->xsp_OutBuf,xpar->xsp_InBuf,xpar->xsp_OutLen=xpar->xsp_InLen);
  return 0;
}
