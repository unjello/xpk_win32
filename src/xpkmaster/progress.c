/* progress.c -- Progress report handler
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

#include <sys/types.h>
#include <time.h>
#include <sys/timeb.h>
#include "xpkprefs.h"
#include "xpkmaster.h"

int callprogress(struct XpkBuffer *xbuf)
{
  struct XpkProgress *prog = &xbuf->xb_Prog;
  struct Hook *hk = xbuf->xb_ChunkHook;

  if(hk) {
    unsigned int ucur, ulen;

    if((ucur = prog->xp_UCur) && (ulen = prog->xp_ULen)) {
      unsigned int secs;
      int mics;
      tm* ttime;
	  __timeb64 tmptime;

	  _ftime64(&tmptime);
	  ttime = _localtime64(&tmptime.time);
	  secs=ttime->tm_sec-xbuf->xb_Secs;
      mics=tmptime.millitm-xbuf->xb_Mics;

      /* 7813 = 100000 / 128, 0x1000000 = 0x100000000/256 (ULONG size),
         +1 prevents division by zero */
      if(ucur < 0x1000000)
        prog->xp_Speed = (ucur<<7) / ((secs<<7) + mics/7813 + 1);
      else
        prog->xp_Speed = ucur / ++secs;

      if(ucur > 0x01FFFFFF)
	prog->xp_Done = ucur / (ulen / 100);
      else
        prog->xp_Done = 100 * ucur / ulen;

      if(prog->xp_CCur > 0x01FFFFFF)
        prog->xp_CF = 100 - prog->xp_CCur / (ucur / 100);
      else
        prog->xp_CF = 100 - 100 * prog->xp_CCur / ucur;
    }
    if(prog->xp_CF < 0)
      prog->xp_CF = 0;

    if((*(regfunc) hk->h_Entry) (prog))
      xbuf->xb_Result = XPKERR_ABORTED;
  }

  return xbuf->xb_Result;
}
