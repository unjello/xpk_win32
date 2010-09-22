/* xpkRLEN.c -- a runlength packer library for the XPK system
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

struct XpkMode RlenMode =
{
  NULL,				/* next			*/
  100,				/* upto			*/
  XPKMF_A3000SPEED,		/* flags		*/
  0,				/* packmem		*/
  0,				/* unpackmem		*/
  140,				/* packspeed, K / sec	*/
  1043,				/* unpackspeed, K / sec	*/
  45,				/* ratio, *0.1 %	*/
  0,				/* reserved		*/
  "normal"			/* description		*/
};

static struct XpkInfo RlenInfo =
{
  1,				/* info version */
  1,				/* lib  version */
  0,				/* master vers  */
  0,				/* pad          */
  "RLEN",			/* short name   */
  "Run Length",			/* long name    */
  "Fast and simple compression usable for simple data",	/* description*/
  0x524C454E,			/* 4 letter ID  */
  XPKIF_PK_CHUNK |		/* flags        */
  XPKIF_UP_CHUNK,
  0x7fffffff,			/* max in chunk */
  0,				/* min in chunk */
  0x4004,			/* def in chunk */
  NULL,				/* pk message   */
  NULL,				/* up message   */
  NULL,				/* pk past msg  */
  NULL,				/* up past msg  */
  50,				/* def mode     */
  0,				/* pad          */
  &RlenMode			/* modes        */
};

/*
 * Returns an info structure about our packer
 */
extern "C" __declspec(dllexport) struct XpkInfo *LIBXpksPackerInfo(void)
{
  return &RlenInfo;
}

/*
 * Pack a chunk
 */
extern "C" __declspec(dllexport) int LIBXpksPackChunk(struct XpkSubParams *xpar)
{
  char *get = xpar->xsp_InBuf, *start = xpar->xsp_InBuf;
  char *end = get + xpar->xsp_InLen, *put = xpar->xsp_OutBuf;
  char *wend = put + xpar->xsp_OutBufLen;
  int run, i;

  for(;;) {
    run = get[0] == get[1] && get[1] == get[2];

    if(put + (get - start) + 4 > wend)
      return XPKERR_EXPANSION;

    if(run || get - start == 127 || get == end) {
      /* write uncompressed */
      if(get - start) {
	*put++ = get - start;
	for(i = get - start; i > 0; i--)
	  *put++ = *start++;
      }
      if(get == end) {
	*put++ = 0;
	break;
      }
      start = get;
    }

    if(run) {
      /* write compressed  */
      for(i = 3; get + i < end && get[i - 1] == get[i] && i < 127; i++);
      *put++ = -i;
      *put++ = get[0];
      get += i;
      start = get;
    }
    else
      get++;
  }
  xpar->xsp_OutLen = put - (char *)xpar->xsp_OutBuf;

  return 0;
}

extern "C" __declspec(dllexport) int LIBXpksUnpackChunk(struct XpkSubParams *xpar)
{
  char *get = xpar->xsp_InBuf, *put = xpar->xsp_OutBuf, v;
  int i;

  while((i = (char)*get++)) {
    /* Even though char SHOULD be signed on AIX it's not. */
    if(i>127)
      i-=256;

    if(i > 0)
      for(; i > 0; i--)
	*put++ = *get++;
    else
      for(i = -i, v = *get++; i > 0; i--)
	*put++ = v;
  }

  return 0;
}
