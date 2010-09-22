/* password.c -- password requester related things
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

/* FIX: uh.. this probably doesn't quite work. An encrypting library is needed
 *      for testing so one needs to be implemented first. */

#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include "xpk.h"
#include "xpkprefs.h"
#include "texts.h"
#include "xpkmaster.h"

/* NOTE: what are these anyway? */
#define TESTSIZE	13

#define VERIFY_OFF	0
#define VERIFY_ON	1
#define VERIFY_ACTIVE	2
#define VERIFY_DONE	3

static const struct PassCharData {
  unsigned int Flag;
  char Lower;
  char Upper;
} TestField[TESTSIZE] = {
{XPKPASSFF_30x39,0x30,0x39}, {XPKPASSFF_41x46,0x41,0x46},
{XPKPASSFF_61x66,0x61,0x66}, {XPKPASSFF_47x5A,0x47,0x5A},
{XPKPASSFF_67x7A,0x67,0x7A}, {XPKPASSFF_20,   0x20,0x20},
{XPKPASSFF_SPECIAL7BIT,0x21,0x2F},
{XPKPASSFF_SPECIAL7BIT,0x3A,0x40},
{XPKPASSFF_SPECIAL7BIT,0x5B,0x60},
{XPKPASSFF_SPECIAL7BIT,0x7B,0x7E},
{XPKPASSFF_C0xDE,0xC0,0xDE}, {XPKPASSFF_DFxFF,0xDF,0xFF},
{XPKPASSFF_SPECIAL8BIT,0xA0,0xBF},
};

/* this is no public structure !! */
struct RequestData {
  unsigned int rd_BufSize;
  unsigned short rd_Verify;
  char *rd_Title;
  char *rd_GivenBuffer;
  char *rd_PassBuffer;
  char rd_ScreenTitle[80];
  char rd_KeyBuffer[9];
};

/* returns XPKERR codes */
int DoRequest(struct RequestData *rd)
{
  char *pass;
  int err;

  err=XPKERR_UNKNOWN;

  while(!(rd->rd_Verify==VERIFY_ON || rd->rd_Verify==VERIFY_DONE)) {
	// todo: PROMPT O HAS£O!
    //pass=(char *)getpass(rd->rd_Title);
    pass = "dupa";

    if(rd->rd_Verify==VERIFY_ACTIVE) {
      if(strcmp(pass, rd->rd_PassBuffer))
	err=XPKERR_WRONGPW;
      else
	rd->rd_Verify=VERIFY_DONE;
    } else {
      if(rd->rd_Verify==VERIFY_ON) {
	rd->rd_Verify=VERIFY_ACTIVE;
      }
      if(err!=XPKERR_WRONGPW) {
	strcpy(rd->rd_PassBuffer, pass);
	err=XPKERR_OK;
      }
    }
  }

  return err;
}

int XpkPassRequest(struct TagItem *ti)
{
  struct RequestData *rd;
  int mode = 0, useprefs = 1;
  struct TagItem *tags = ti;

#ifdef _DEBUG
  DebugTagList("XpkPassRequest", tags);
#endif

  if(!(rd = (struct RequestData *) calloc(sizeof(struct RequestData), 1)))
    return XPKERR_NOMEM;

  while((ti = NextTagItem(&tags)))
  {
    switch(ti->ti_Tag)
    {
    case XPK_PassChars:
      break;
    case XPK_PasswordBuf:
      rd->rd_GivenBuffer=(char *)ti->ti_Data;
      mode += 10;
      break;
    case XPK_PassBufSize:
      rd->rd_BufSize=(unsigned int)ti->ti_Data;
      break;
    case XPK_Key16BitPtr:
      rd->rd_GivenBuffer=(char *)ti->ti_Data;
      mode += 11;
      break;
    case XPK_Key32BitPtr:
      rd->rd_GivenBuffer=(char *)ti->ti_Data;
      mode += 12;
      break;
    case XPK_PubScreen:
      break;
    case XPK_PassTitle:
      rd->rd_Title = ti->ti_Data ? (char *) ti->ti_Data : "";
      break;
    case XPK_TimeOut:
      break;
    case XPK_Preferences:
      if(!ti->ti_Data) useprefs = 0;
      break;
    case XPK_PassWinLeft:
      break;
    case XPK_PassWinTop:
      break;
    case XPK_PassWinWidth:
      break;
    case XPK_PassWinHeight:
      break;
    case XPK_PassCenter:
      break;
    case XPK_PassVerify:
      rd->rd_Verify = (ti->ti_Data ? VERIFY_ON : VERIFY_OFF);
      break;
    };
  }

  if(!mode || (mode > 12) || (mode == 10 && !rd->rd_BufSize) ||
     !rd->rd_GivenBuffer) {
    free(rd);
    return XPKERR_BADPARAMS;
  }

  /* create title text */
  if(!rd->rd_Title) {
    if(mode == 10)
      rd->rd_Title = strings[TXT_REQ_PASSWORD];
    else {
      rd->rd_Title = rd->rd_ScreenTitle;
      sprintf(rd->rd_ScreenTitle, strings[TXT_REQ_KEY], (mode==11 ? 16 : 32));
    }
  }

  if(mode > 10) {
    rd->rd_BufSize = (mode == 11 ? 5 : 9);
    rd->rd_PassBuffer = rd->rd_KeyBuffer;
  } else
    rd->rd_PassBuffer = rd->rd_GivenBuffer;
  
  if(!(useprefs = DoRequest(rd))) {
    if(mode == 11)
      *((unsigned short *) rd->rd_GivenBuffer) = strtoul(rd->rd_PassBuffer, 0, 16);
    else if(mode == 12)
      *((unsigned int *) rd->rd_GivenBuffer) = strtoul(rd->rd_PassBuffer, 0, 16);
  }

  free(rd);

  return useprefs;
}

/* FIX: This is probably buggy */
int XpkPassRequestTags(Tag tag, ...)
{
  va_list args;
  struct TagItem *taglist;
  int ret, i, numargs=1;
  Tag current;

  /* Count the number of tags */
  current=tag;
  va_start(args, tag);
  while(current!=TAG_DONE) {
    va_arg(args, void *);
    current=va_arg(args, Tag);
    numargs++;
  }
  va_end(args);

  /* Allocate memory for the tag array */
  taglist=(TagItem*)calloc(numargs, sizeof(struct TagItem));

  /* Copy the args to the array */
  current=tag;
  va_start(args, tag);
  for(i=0; i<numargs; i++) {
    taglist[i].ti_Tag=current;
    taglist[i].ti_Data=va_arg(args, void *);
    current=va_arg(args, Tag);
  }
  va_end(args);

  ret=XpkPassRequest(taglist);

  free(taglist);
  return ret;
}
