/* tags.c -- Tag handling functions
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
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "xpkprefs.h"
#include "xpkmaster.h"
#include "texts.h"

static int findmethod(struct XpkBuffer *xbuf, char *name);

/* Parse input/output buffer specification tags before an operation */
int parsebuftags(struct XpkBuffer *xbuf, struct TagItem *tags)
{
  struct TagItem *ti, *scan=tags;
  FILE *fh;
  void *data;

#ifdef _DEBUG
  DebugTagList("parsebuftags", tags);
#endif

  /* Pass 1 */
  while((ti=NextTagItem(&scan))) {
    data=ti->ti_Data;

    switch(ti->ti_Tag) {
    case XPK_Preferences:
      if(!data) xbuf->xb_Flags |= XMF_NOPREFS; break;
    case XPK_ChunkHook:
      xbuf->xb_ChunkHook=(struct Hook *) data;
      break;
    case XPK_NoClobber:
      if(data) xbuf->xb_Flags |= XMF_NOCLOBBER; break;
    case XPK_FileName:
      xbuf->xb_Prog.xp_FileName=FilePart((char *)data); break;
    case XPK_GetOutLen:
      xbuf->xb_GetOutLen=(unsigned int *)data; break;
    case XPK_GetOutBufLen:
      xbuf->xb_GetOutBufLen=(unsigned int *)data; break;
    case XPK_GetError:
      if(data) {
        xbuf->xb_ErrBuf=(char *)data;
        *(xbuf->xb_ErrBuf)=0;
      }
      break;
    case XPK_GetOutBuf:
      xbuf->xb_Flags |= XMF_GETOUTBUF;
      xbuf->xb_WMsg.xmm_Flags |= XIO_GETOUTBUF;
      xbuf->xb_WMsg.xmm_BufOfs=0;
      xbuf->xb_PackParam.xsp_Flags |= XSF_PREVCHUNK;
      xbuf->xb_WHook=&memouthook;
      xbuf->xb_GetOutBuf=(char **)data;
      break;
    case XPK_PackMethod: /* first pass, because of XPK_PackMode */
      xbuf->xb_Flags |= XMF_PACKING;
      findmethod(xbuf, (char *)data);
      break;
    case XPK_NeedSeek:
      if(data)
	xbuf->xb_Flags |= XMF_SEEK;
      break;
    }
  }

  /* may be an error in findmethod */
  if(xbuf->xb_Result)
    return xbuf->xb_Result;

  /* get default preferences settings */
  xbuf->xb_Flags |= XMF_EXTERNALS;

  /* Pass 2 */
  while((ti=NextTagItem(&tags))) {
    data=ti->ti_Data;

    switch(ti->ti_Tag) {
      /* Ways to specify input data */
    case XPK_InName:
      xbuf->xb_RMsg.xmm_FileName=FilePart((char *)data);
      if(!(data=(FILE *)fopen((char *)data, "rb")))
 	return (xbuf->xb_Result=XPKERR_IOERRIN);
      xbuf->xb_RMsg.xmm_Flags |= XMF_PRIVFH;
      /* no break, as following is needed too */
    case XPK_InFH:
      xbuf->xb_RMsg.xmm_FH=(FILE *)data;
      xbuf->xb_RHook=&fhinhook;
      break;
    case XPK_InBuf:
      xbuf->xb_RMsg.xmm_Buf=(char *) data;
      xbuf->xb_RMsg.xmm_BufOfs=0;
      xbuf->xb_RHook=&meminhook;
      break;
    case XPK_InLen:
      xbuf->xb_InLen=xbuf->xb_RMsg.xmm_Len=(unsigned int)data; break;
    case XPK_InHook:
      xbuf->xb_RHook=(struct Hook *)data;
      break;

      /* Ways to specify output data */
    case XPK_OutName:
      if(xbuf->xb_Flags & XMF_NOCLOBBER)
	if((fh=fopen((char *) data, "rb"))) {
	  fclose(fh);
	  return (xbuf->xb_Result=XPKERR_FILEEXISTS);
	}
      xbuf->xb_WMsg.xmm_FileName=(char *)data;
      if(!(data=(FILE *)fopen((char *)data, "wb"))) {
        xbuf->xb_WMsg.xmm_FileName=0;
 	return (xbuf->xb_Result=XPKERR_IOERROUT);
      }
      xbuf->xb_WMsg.xmm_Flags |= XMF_PRIVFH;
    case XPK_OutFH:
      xbuf->xb_WMsg.xmm_FH=(FILE *)data;
      xbuf->xb_WHook=&fhouthook;
      break;
    case XPK_OutBuf:
      xbuf->xb_WMsg.xmm_Buf=(char *) data;
      xbuf->xb_WMsg.xmm_BufOfs=0;
      xbuf->xb_WHook=&memouthook;
      xbuf->xb_PackParam.xsp_Flags |= XSF_PREVCHUNK;
      break;
    case XPK_OutBufLen:
      xbuf->xb_WMsg.xmm_BufLen=(unsigned int)data;
      break;
    case XPK_OutHook:
      xbuf->xb_WHook=(struct Hook *) data;
      break;

      /* Other junk */
    case XPK_Password:
      xbuf->xb_Password=(char *)data;
      break;
    case XPK_Key16:
      xbuf->xb_PassKey16=(unsigned short)data;
      xbuf->xb_Flags |= XMF_KEY16;
      break; 
    case XPK_Key32:
      xbuf->xb_PassKey32=(unsigned int)data;
      xbuf->xb_Flags |= XMF_KEY32;
      break;
    case XPK_PassThru:
      if(data)
	xbuf->xb_Flags |= XMF_PASSTHRU;
      break;
    case XPK_UseXfdMaster:
      if(data)
        xbuf->xb_Flags |= XMF_XFD;
      else
        xbuf->xb_Flags &= ~XMF_XFD;
      break;
      /* currently not used */
    case XPK_UseExternals:
      if(data)
        xbuf->xb_Flags |= XMF_EXTERNALS;
      else
        xbuf->xb_Flags &= ~XMF_EXTERNALS;
      break;
    case XPK_PassRequest:
      if(data)
        xbuf->xb_Flags |= XMF_AUTOPASSWD;
      else
        xbuf->xb_Flags &= ~XMF_AUTOPASSWD;
      break;
    case XPK_ChunkReport:
      if(data && !xbuf->xb_ChunkHook && !(xbuf->xb_Flags & XMF_NOPREFS))
        xbuf->xb_Flags |= XMF_AUTOPRHOOK;
      break;
    case XPK_OutMemType:
      xbuf->xb_WMsg.xmm_MemType=(unsigned int)data;
      break;
    case XPK_ChunkSize:
      /* This may get adjusted later */
      xbuf->xb_ChunkSize=ROUNDLONG((unsigned int)data);
      break;
    case XPK_PackMode:
      xbuf->xb_PackingMode=(unsigned short)data;
      break;
    case XPK_TaskPri:
      SetPriorityClass(GetCurrentProcess(), (DWORD)data);
      xbuf->xb_Flags |= XMF_OWNTASKPRI;
      break;
    case XPK_StepDown:
      xbuf->xb_PackParam.xsp_Flags |= XSF_STEPDOWN;
      break;
    case XPK_LossyOK:
      xbuf->xb_Flags |= XMF_LOSSYOK;
      break;
    case XPK_NoCRC:
      xbuf->xb_Flags |= XMF_NOCRC;
      break;
    }
  }

  if(xbuf->xb_Flags & XMF_PACKING)
    xbuf->xb_PackParam.xsp_Flags &= ~XSF_PREVCHUNK;

  if(xbuf->xb_Password && !*xbuf->xb_Password)
    xbuf->xb_Password=0;

  if(!xbuf->xb_Prog.xp_FileName)
  {
    xbuf->xb_Prog.xp_FileName=xbuf->xb_RMsg.xmm_FileName;
    if(!(xbuf->xb_Flags & XMF_PACKING) && xbuf->xb_WMsg.xmm_FileName)
      xbuf->xb_Prog.xp_FileName=FilePart(xbuf->xb_WMsg.xmm_FileName);
  }

  if(!xbuf->xb_Prog.xp_FileName)
    xbuf->xb_Prog.xp_FileName="";

  return (xbuf->xb_Result=0);
}

int parseerrortags(struct TagItem *tags, int err)
{
  struct TagItem *ti;

  while((ti=NextTagItem(&tags))) {
    if(ti->ti_Tag == XPK_GetError)
      XpkFault(err, 0, (char *) ti->ti_Data, XPKERRMSGSIZE);
  }
  return err;
}

/* Parse XPK_Get... tags after an operation is finished */
void parsegettags(struct XpkBuffer *xbuf)
{
  if(xbuf->xb_GetOutBuf)
    *xbuf->xb_GetOutBuf=xbuf->xb_WMsg.xmm_Buf;

  if(xbuf->xb_GetOutLen) {
    if(xbuf->xb_Flags & XMF_PACKING)
      *xbuf->xb_GetOutLen=xbuf->xb_Fib.xf_CLen;
    else
      *xbuf->xb_GetOutLen=xbuf->xb_Fib.xf_ULen;
  }

  if(xbuf->xb_GetOutBufLen)
    *xbuf->xb_GetOutBufLen=xbuf->xb_WMsg.xmm_BufLen;

  if(xbuf->xb_ErrBuf)
    XpkFault(xbuf->xb_Result, 0, xbuf->xb_ErrBuf, XPKERRMSGSIZE);
}

/* Find a compression library/method given a name */
static int findmethod(struct XpkBuffer *xbuf, char *name)
{
  unsigned int id;

  /* Try to use the first four letters as the ID */
  if(!name || !*name)
    return XPKERR_BADPARAMS;
  else if((id=idfromname(name)) == USER_COOKIE)
    xbuf->xb_PackingMode=100;
  else if(!opensub(xbuf, id))
    return xbuf->xb_Result;
  else
    xbuf->xb_PackingMode=xbuf->xb_SubInfo->xi_DefMode;

  /* note: invalid add-ons aren't checked here */
  if(name[4] == '.')
    xbuf->xb_PackingMode=strtoul(name + 5, 0, 10);
  return 0;
}

char *FilePart(char *path)
{
  char *ptr;

  if(path) {
    while((ptr=strpbrk(path, "/"))) {
      path=ptr+1;
    }

    return path;
  } else
    return 0;
}

struct TagItem *FindTagItem(Tag tagValue, struct TagItem *tagList)
{
  while((tagList)) {
    if(tagList->ti_Tag==tagValue)
      return tagList;

    tagList++;
  }
  return 0;
}

struct TagItem *NextTagItem(struct TagItem **tagItemPtr)
{
  struct TagItem *current;

  if((current=*tagItemPtr)) {
    switch(current->ti_Tag) {
    case TAG_SKIP:
      *tagItemPtr+=(unsigned int)(current->ti_Data)+1;
      break;
    case TAG_IGNORE:
      *tagItemPtr+=1;
      break;
    case TAG_MORE:
      *tagItemPtr=(struct TagItem *)current->ti_Data;
      break;
    case TAG_DONE:
      *tagItemPtr=0;
      break;
    default:
      *tagItemPtr+=1;
      break;
    }
  }
  return current;
}


