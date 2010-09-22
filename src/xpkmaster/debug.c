/* debug.c -- the debug stuff
 * Copyright (C) 1996-2003 authors
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "xpk.h"
#include "xpkmaster.h"
#include <io.h>

typedef void (*putchtype) ();

#define FLAG_ERROR	(1<<0)
#define FLAG_RUNTIME	(1<<1)
#define FLAG_TAGLIST	(1<<2)

extern int KPutChar(int);
extern int DPutChar(int);
extern void DoDebug(char mode, char *fmt, void *data);

static void normfunc(char c, unsigned int pd)
{
  char d=c;
  if(c)
    _write(pd, &d, 1);
}

void DebugTagList(char *fmt, struct TagItem *taglist)
{
  DoDebug(FLAG_TAGLIST, fmt, taglist);
}

void DebugError(char *format, ...)
{
  DoDebug(FLAG_RUNTIME, format, (char *)((unsigned int)(&format)+sizeof(char *)));
}

void DebugRunTime(char *format, ...)
{
  DoDebug(FLAG_ERROR, format, (char *)((unsigned int)(&format)+sizeof(char *)));
}

void DoDebug(char mode, char *fmt, void *data)
{
  FILE *fh=0;
  unsigned int i, Flags = 0;
  char *Mode;
  void (*function)(char, unsigned int) = 0;

  if(!(Mode=getenv("XPKDEBUG"))) {
    Mode=(char *)calloc(5, sizeof(char));
    Mode[0]='F';
    Mode[1]='E';
    Mode[2]='R';
    Mode[3]='T';
  }
  
  for(i=1; Mode[i] && i < 5; ++i) {
    switch(Mode[i])
    {
      case 'E':
	Flags |= FLAG_ERROR; break;
      case 'R':
	Flags |= FLAG_RUNTIME; break;
      case 'T':
	Flags |= FLAG_TAGLIST; break;
    }
  }

  mode &= Flags;

  if(mode) {
    switch(Mode[0]) {
    case 'F':
      if((fh = fopen("XpkMaster.Out", "a+"))) {
        fseek(fh, 0, SEEK_END);
        function = normfunc;
      }
      break;
    }
    if(function) {
      i = (unsigned int) GetCurrentProcessId();
      fprintf(fh, "XpkM(%d):", i);
      fprintf(fh, fmt, data);
      fprintf(fh, "\n");

      if(mode & FLAG_TAGLIST) {
        struct TagItem *ti;
        while((ti = NextTagItem((struct TagItem **) &data))) {
          unsigned int *i[2], dmode = 0;
          char *s;
         
          /* dmode == 1 - BOOL data, dmode == 2 - unknown */
	  switch(ti->ti_Tag) {
	  case XPK_InName:	  s = "XPK_InName, \"%s\" ($%08lx)"; break;
	  case XPK_InFH:	  s = "XPK_InFH, $%08lx"; break;
	  case XPK_InBuf:	  s = "XPK_InBuf, $%08lx"; break;
	  case XPK_InHook:	  s = "XPK_InHook, $%08lx"; break;
	  case XPK_OutName:	  s = "XPK_OutName, \"%s\" ($%08lx)"; break;
	  case XPK_OutFH:	  s = "XPK_OutFH, %08lx"; break;
	  case XPK_OutBuf:	  s = "XPK_OutBuf, $%08lx"; break;
	  case XPK_GetOutBuf:	  s = "XPK_GetOutBuf, $%08lx"; break;
	  case XPK_OutHook:	  s = "XPK_OutHook, $%08lx"; break;
	  case XPK_InLen:	  s = "XPK_InLen, %lu"; break;
	  case XPK_OutBufLen:	  s = "XPK_OutBufLen, %lu"; break;
	  case XPK_GetOutLen:	  s = "XPK_GetOutLen, $%08lx"; break;
	  case XPK_GetOutBufLen:  s = "XPK_GetOutBufLen, $%08lx"; break;
	  case XPK_Password:	  s = "XPK_Password, \"%s\" ($%08lx)"; break;
	  case XPK_GetError:	  s = "XPK_GetError, $%08lx"; break;
	  case XPK_OutMemType:	  s = "XPK_OutMemType, $%08lx"; break;
	  case XPK_PassThru:	  s = "XPK_PassThru, %s"; dmode = 1; break;
	  case XPK_StepDown:	  s = "XPK_StepDown, %s"; dmode = 1; break;
	  case XPK_ChunkHook:	  s = "XPK_ChunkHook, $%08ld"; break;
	  case XPK_PackMethod:	  s = "XPK_PackMethod, \"%s\" ($%08lx)"; break;
	  case XPK_ChunkSize:	  s = "XPK_ChunkSize, %lu"; break;
	  case XPK_PackMode:	  s = "XPK_PackMode, %lu"; break;
	  case XPK_NoClobber:	  s = "XPK_NoClobber, %s"; dmode = 1; break;
	  case XPK_Ignore:	  s = "XPK_Ignore"; break;
	  case XPK_TaskPri:	  s = "XPK_TaskPri, $ld"; break;
	  case XPK_FileName:	  s = "XPK_FileName, \"%s\" ($%08lx)"; break;
	  case XPK_ShortError:	  s = "XPK_ShortError, %s"; dmode = 1; break;
	  case XPK_PackersQuery:  s = "XPK_PackersQuery, $%08lx"; break;
	  case XPK_PackerQuery:	  s = "XPK_PackerQuery, $%08lx"; break;
	  case XPK_ModeQuery:     s = "XPK_ModeQuery, $%08lx"; break;
	  case XPK_LossyOK:	  s = "XPK_LossyOK, %s"; dmode = 1; break;
	  case XPK_NoCRC:	  s = "XPK_NoCRC, $%08lx"; break;
	  case XPK_Key16:	  s = "XPK_Key32, $%04lx"; break;
	  case XPK_Key32:	  s = "XPK_Key32, $%08lx"; break;
	  case XPK_NeedSeek:	  s = "XPK_NeedSeek, %s"; dmode = 1; break;
          case XPK_UseXfdMaster:  s = "XPK_UseXfdMaster, %s"; dmode = 1; break;
          case XPK_UseExternals:  s = "XPK_UseExternals, %s"; dmode = 1; break;
          case XPK_PassRequest:   s = "XPK_PassRequest, %s"; dmode = 1; break;
	  case XPK_Preferences:	  s = "XPK_Preferences, %s"; dmode = 1; break;
	  case XPK_ChunkReport:	  s = "XPK_ChunkReport, %s"; dmode = 1; break;
	  case XPK_PassChars:	  s = "XPK_PassChars, $%08lx"; break;
	  case XPK_PasswordBuf:   s = "XPK_PasswordBuf, $%08lx"; break;
	  case XPK_PassBufSize:	  s = "XPK_PassBufSize, %lu"; break;
	  case XPK_Key16BitPtr:	  s = "XPK_Key16BitPtr, $%08lx"; break;
	  case XPK_Key32BitPtr:	  s = "XPK_Key32BitPtr, $%08lx"; break;
	  case XPK_PubScreen:	  s = "XPK_PubScreen, $%08lx"; break;
	  case XPK_PassTitle:	  s = "XPK_PassTitle, \"%s\" ($%08lx)"; break;
	  case XPK_TimeOut:	  s = "XPK_TimeOut, %lu"; break;
	  case XPK_PassWinLeft:	  s = "XPK_PassWinLeft, %lu"; break;
	  case XPK_PassWinTop:	  s = "XPK_PassWinTop, %lu"; break;
	  case XPK_PassWinWidth:  s = "XPK_PassWinWidth, %lu"; break;
	  case XPK_PassWinHeight: s = "XPK_PassWinHeight, %lu"; break;
	  case XPK_PassCenter:	  s = "XPK_PassCenter, %s"; dmode = 1; break;
	  case XPK_PassVerify:	  s = "XPK_PassVerify, %s"; dmode = 1; break;
	  default:		  s = "$%08lx, $%08lx"; dmode = 2; break;
	  }
	  if(dmode == 1)
            i[0] = ti->ti_Data ? (unsigned int *) "TRUE" : (unsigned int *) "FALSE";
          else if(dmode == 2) {
	    i[0] = (unsigned int *)ti->ti_Tag;
	    i[1] = (unsigned int *)ti->ti_Data;
	  } else
            i[0] = i[1] = (unsigned int *)ti->ti_Data;

          fprintf(fh, "   ");
          fprintf(fh, s, i);
          fprintf(fh, "\n");
        }
        fprintf(fh, "   TAG_DONE\n");
      }
    }

    if(fh)
      fclose(fh);
  }
}

