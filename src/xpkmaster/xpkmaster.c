/* xpkmaster.c -- the main xpk functions
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

#include "config.h"
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <sys/timeb.h>
#include "xpkmaster.h"
#include "texts.h"

/*
 * XpkPack() - pack a file
 */
int XpkPack(struct TagItem *tags)
{
  struct XpkBuffer *xbuf=0;
  char *buf;
  int totlen, res, chunklen;

#ifdef _DEBUG
  DebugRunTime("XpkPack");
#endif

  if(!FindTagItem(XPK_PackMethod, tags))
    return XPKERR_BADPARAMS;

  if((res=XpkOpen((struct XpkBuffer **)&xbuf, tags)))
    return res;

  if((totlen=xbuf->xb_InLen) == 0xffffffff) {
    xbuf->xb_Result=XPKERR_BADPARAMS;
    return XpkClose((struct XpkBuffer *)xbuf);
  }

  tm* ttime;
  __timeb64 tmptime;
  
  /* Start the clock */
  _ftime64(&tmptime);
  ttime = _localtime64(&tmptime.time);

  xbuf->xb_Secs=ttime->tm_sec;
  xbuf->xb_Mics=tmptime.millitm;

  xbuf->xb_Prog.xp_Type=XPKPROG_START;
  xbuf->xb_Prog.xp_ULen=totlen;
  if(callprogress(xbuf))
    return XpkClose((struct XpkBuffer *)xbuf);

  while(totlen > 0) {
    chunklen=xbuf->xb_Fib.xf_NLen;
    
    if(!(buf=(char *)hookread(xbuf, XIO_READ, NULL, chunklen)))
      break;

    if(XpkWrite((struct XpkBuffer *) xbuf, buf, chunklen))
      break;

    totlen -= chunklen;

    /* Progress report */
    xbuf->xb_Prog.xp_Type=XPKPROG_MID;
    xbuf->xb_Prog.xp_UCur += chunklen;
    xbuf->xb_Prog.xp_CCur=xbuf->xb_Fib.xf_CCur;
    if(callprogress(xbuf))
      return XpkClose((struct XpkBuffer *)xbuf);
  }

  if(xbuf->xb_Prog.xp_Type) {
    xbuf->xb_Prog.xp_Type=XPKPROG_END;
    xbuf->xb_Prog.xp_CCur += xbuf->xb_Headers.h_LocSize;
    xbuf->xb_Prog.xp_Activity=xbuf->xb_Result ?
      strings[TXT_ABORTED] : xbuf->xb_LastMsg;
    /* Call the hook one last time */
    callprogress(xbuf);
  }

  return XpkClose((struct XpkBuffer *)xbuf);
}

/*
 * XpkUnpack - unpack a file
 */
int XpkUnpack(struct TagItem *tags)
{
  struct XpkBuffer *xbuf=NULL;
  char *pointer;
  int len, res;

#ifdef _DEBUG
  DebugRunTime("XpkUnpack");
#endif
  
  if((res=XpkOpen((struct XpkBuffer **) &xbuf, tags)))
    return res;
 
  if(xbuf->xb_Flags & XMF_PACKING) {
    xbuf->xb_Result=XPKERR_BADPARAMS;
    goto Abort;
  }
  
  /* Start the clock */
  tm* ttime;
  __timeb64 tmptime;

  _ftime64(&tmptime);
  ttime = _localtime64(&tmptime.time);

  xbuf->xb_Secs=ttime->tm_sec;
  xbuf->xb_Mics=tmptime.millitm;
  
  /* Initialize progress */
  xbuf->xb_Prog.xp_Type=XPKPROG_START;
  xbuf->xb_Prog.xp_ULen=xbuf->xb_Fib.xf_ULen;
  if(callprogress(xbuf))
    goto Abort;
  
  if(!hookwrite(xbuf, XIO_TOTSIZE, NULL, xbuf->xb_Fib.xf_ULen + XPK_MARGIN)) {
#ifdef _DEBUG
    DebugError("XpkUnpack: XIO_TOTSIZE failed");
#endif
    goto Abort;
  }
  
  if(!(pointer=(char *)hookwrite(xbuf, XIO_GETBUF, 0, xbuf->xb_Fib.xf_NLen))) {
#ifdef _DEBUG
    DebugError("XpkUnpack: XIO_GETBUF failed (a)");
#endif
    goto Abort;
  }
  
#ifdef _DEBUG
  if(xbuf->xb_Result)
    DebugError("XpkUnpack: failure before unpackloop");
#endif

  while((len=XpkRead((struct XpkBuffer *)xbuf, pointer, xbuf->xb_Fib.xf_NLen))>0) {
    if(!hookwrite(xbuf, XIO_WRITE, pointer, len)) {
#ifdef _DEBUG
      DebugError("XpkUnpack: XIO_WRITE failed");
#endif
      goto Abort;
    }

    /* Progress report */
    xbuf->xb_Prog.xp_Type=XPKPROG_MID;
    xbuf->xb_Prog.xp_CCur=xbuf->xb_Fib.xf_CCur;
    xbuf->xb_Prog.xp_UCur=xbuf->xb_Fib.xf_UCur;
    if(callprogress(xbuf))
      goto Abort;

    if(!(pointer=(char *)hookwrite(xbuf, XIO_GETBUF, NULL, xbuf->xb_Fib.xf_NLen))) {
#ifdef _DEBUG
      DebugError("XpkUnpack: XIO_GETBUF failed (b)");
#endif
      goto Abort;
    }
  }

  xbuf->xb_Result=len;
#ifdef _DEBUG
  if(xbuf->xb_Result)
    DebugError("XpkUnpack: XpkRead failed with %ld", xbuf->xb_Result);
#endif

  if(xbuf->xb_Prog.xp_Type) {
    xbuf->xb_Prog.xp_Type=XPKPROG_END;
    xbuf->xb_Prog.xp_Activity=xbuf->xb_Result ? strings[TXT_ABORTED] : xbuf->xb_LastMsg;
    /* Call the hook one last time */
    callprogress(xbuf);
  }
 Abort:
  return XpkClose((struct XpkBuffer *)xbuf);
}

/*
 * XpkOpen - open a file for packing/unpacking
 */
int XpkOpen(struct XpkBuffer **xbufp, struct TagItem *tags)
{
#if defined(_DEBUG)
  DebugRunTime("XpkOpen");
#endif
  return xpkopen(xbufp, tags, 0);
}

/*
 *   XpkExamine() - inspect a compressed file
 */
int XpkExamine(struct XpkFib *fib, struct TagItem *tags)
{
  struct XpkBuffer *dummy;
  int res;
  
#if defined(_DEBUG)
  DebugRunTime("XpkExamine");
#endif

  if((res=xpkopen(&dummy, tags, 1)))
    return res;

  /* copies the entries of XpkFib
   * fib=dummy->Fib works too, but calls it's own copy-function */
  memcpy(fib, dummy, sizeof(struct XpkFib));

  return XpkClose((struct XpkBuffer *)dummy);
}

/*
 *   XpkRead() - read one chunk from a compressed file
 */
int XpkRead(struct XpkBuffer *xbuf, char *buf, unsigned int len)
{
#ifdef _DEBUG
  DebugRunTime("XpkRead: buf %08lx, size %ld, uncrunched (%ld/%ld), crunched (%ld/%ld)", buf, len,
	       xbuf->xb_Fib.xf_UCur, xbuf->xb_Fib.xf_ULen, xbuf->xb_Fib.xf_CCur,xbuf->xb_Fib.xf_CLen);

#ifdef TESTING
  printf("XpkRead: uncrunched (%lx/%lx), crunched (%lx/%lx)\n",
  	 xbuf->xb_Fib.xf_UCur, xbuf->xb_Fib.xf_ULen,
  	 xbuf->xb_Fib.xf_CCur,xbuf->xb_Fib.xf_CLen);
#endif
#endif
  
  if(!xbuf)
    return XPKERR_NOFUNC;

  if(xbuf->xb_Flags & XMF_EOF)
    return 0;

  switch(xbuf->xb_Format)
    {
      /* Unpack standard XPK */
    case XPKMODE_UPSTD: {
      struct XpkSubParams *xpar;
      HMODULE XpkSubBase=xbuf->xb_SubBase;
      int (*XpksUnpackChunk)(struct XpkSubParams *);
      XpkChunkHeader *lochdr=&(xbuf->xb_Headers.h_Loc);
      int ulen, clen, rclen, lochdrsize=xbuf->xb_Headers.h_LocSize;
      unsigned int csum;
      
      if(lochdr->xch_Word.xchw_Type == XPKCHUNK_END)
	return 0;
      
      if((hchecksum((char *) lochdr, lochdrsize))) {
#ifdef _DEBUG
	DebugError("XpkRead: hchecksum(,%ld) failed", lochdrsize);
#endif
	return(xbuf->xb_Result=XPKERR_CHECKSUM);
      }
      
      getUClen(xbuf, &ulen, &clen);
      
      rclen=ROUNDLONG(clen);
      
      if(lochdr->xch_Word.xchw_Type == XPKCHUNK_RAW) {
	if(!(hookread(xbuf, XIO_READ, buf, rclen + lochdrsize)))
	  return xbuf->xb_Result;
	
	if(!(xbuf->xb_Flags & XMF_NOCRC))
	  if((csum=cchecksum((unsigned int *)buf, rclen >>2)) != lochdr->xch_Word.xchw_CChk) {
#ifdef _DEBUG
	    DebugError("XpkRead: cchecksum(,%ld)=%lx != %lx failed", rclen >> 2, csum, (unsigned int) lochdr->xch_Word.xchw_CChk);
#endif
	    return (xbuf->xb_Result=XPKERR_CHECKSUM);
	  }
	
	memcpy(lochdr, buf + rclen, lochdrsize);

#ifndef WORDS_BIGENDIAN
	xbuf->xb_Headers.h_Loc.xch_Word.xchw_CChk=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_CChk);
	xbuf->xb_Headers.h_Loc.xch_Long.xchl_CChk=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Long.xchl_CChk);
	if(xbuf->xb_Headers.h_Glob.xsh_Flags & XPKSTREAMF_LONGHEADERS) {
	  xbuf->xb_Headers.h_Loc.xch_Long.xchl_ULen=_byteswapint32(xbuf->xb_Headers.h_Loc.xch_Long.xchl_ULen);
	  xbuf->xb_Headers.h_Loc.xch_Long.xchl_CLen=_byteswapint32(xbuf->xb_Headers.h_Loc.xch_Long.xchl_CLen);
	} else {
	  xbuf->xb_Headers.h_Loc.xch_Word.xchw_ULen=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_ULen);
	  xbuf->xb_Headers.h_Loc.xch_Word.xchw_CLen=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_CLen);
	}
#endif
      } else if(lochdr->xch_Word.xchw_Type == XPKCHUNK_PACKED) {
	xpar=&xbuf->xb_PackParam;
	if(!(xpar->xsp_InBuf=hookread(xbuf, XIO_READ, NULL, rclen + lochdrsize)))
	  return xbuf->xb_Result;
	
	if(!(xbuf->xb_Flags & XMF_NOCRC))
	  if((csum=cchecksum((unsigned int *)xpar->xsp_InBuf, rclen >> 2)) != lochdr->xch_Word.xchw_CChk) {
#ifdef _DEBUG
	    DebugError("XpkRead: cchecksum(,%ld)=%lx != %lx failed", rclen >>2 , csum, (unsigned int) lochdr->xch_Word.xchw_CChk);
#endif
	    return (xbuf->xb_Result=XPKERR_CHECKSUM);
	  }
	xbuf->xb_Flags |= XMF_INITED;
	xpar->xsp_InLen=clen;
	xpar->xsp_OutLen=ulen;
	xpar->xsp_OutBuf=buf;
	xpar->xsp_OutBufLen=ulen;
	xpar->xsp_Number=0;
	xpar->xsp_Password=xbuf->xb_Password;
	xpar->xsp_LibVersion=xbuf->xb_Headers.h_Glob.xsh_SubVrs;
	
	if((XpksUnpackChunk=(int(*)(struct XpkSubParams *))GetProcAddress(XpkSubBase, "LIBXpksUnpackChunk"))) {
	  if((xbuf->xb_Result=XpksUnpackChunk(xpar)))
	    return xbuf->xb_Result;
	} else {
	  xbuf->xb_Result=XPKERR_NOFUNC;
	  return xbuf->xb_Result;
	}
	
	memcpy(lochdr, (char *)xpar->xsp_InBuf+rclen, lochdrsize);

#ifndef WORDS_BIGENDIAN
	xbuf->xb_Headers.h_Loc.xch_Word.xchw_CChk=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_CChk);
	xbuf->xb_Headers.h_Loc.xch_Long.xchl_CChk=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Long.xchl_CChk);
	if(xbuf->xb_Headers.h_Glob.xsh_Flags & XPKSTREAMF_LONGHEADERS) {
	  xbuf->xb_Headers.h_Loc.xch_Long.xchl_ULen=_byteswapint32(xbuf->xb_Headers.h_Loc.xch_Long.xchl_ULen);
	  xbuf->xb_Headers.h_Loc.xch_Long.xchl_CLen=_byteswapint32(xbuf->xb_Headers.h_Loc.xch_Long.xchl_CLen);
	} else {
	  xbuf->xb_Headers.h_Loc.xch_Word.xchw_ULen=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_ULen);
	  xbuf->xb_Headers.h_Loc.xch_Word.xchw_CLen=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_CLen);
	}
#endif
      }
      else
	return (xbuf->xb_Result=XPKERR_CORRUPTPKD);
      
      if(updatefib(xbuf))
	return xbuf->xb_Result;
      return ulen;
    }
    
    /* Unpack unpacked file */
    case XPKMODE_UPUP: {
      unsigned int leftlen=xbuf->xb_Fib.xf_ULen - xbuf->xb_Fib.xf_CCur;
      
      if(leftlen > len)
	leftlen=len;
      else
	xbuf->xb_Flags |= XMF_EOF;
      
      if(!(hookread(xbuf, XIO_READ, buf, leftlen)))
	return xbuf->xb_Result;
      
      xbuf->xb_Fib.xf_CCur += leftlen;
      xbuf->xb_Fib.xf_UCur += leftlen;
      xbuf->xb_Fib.xf_NLen=Min(xbuf->xb_InLen -
				 xbuf->xb_Fib.xf_UCur, DEFAULTCHUNKSIZE) +
	XPK_MARGIN;
      
      return (int) leftlen;
    }
    }
  
  return xbuf->xb_Result;
}

/*
 *   XpkWrite() - write a chunk to a compressed file
 */
int XpkWrite(struct XpkBuffer *xbuf, char *buf, unsigned int ulen)
{
  HMODULE XpkSubBase=xbuf->xb_SubBase;
  int (*XpksPackChunk)(struct XpkSubParams *);
  struct XpkSubParams *xpar;
  struct Headers *head=&xbuf->xb_Headers;
  int clen, rclen, outbuflen;
  /* last unsigned long of buffer when not longword bounded */
  unsigned short end[2]={0,0};
  char type;
  char *outbuf;

#ifdef _DEBUG
  DebugRunTime("XpkWrite: buf %08lx, size %ld, uncrunched (%ld/%ld), crunched (%ld/%ld)", buf, ulen,
  xbuf->xb_Fib.xf_UCur, xbuf->xb_Fib.xf_ULen, xbuf->xb_Fib.xf_CCur,xbuf->xb_Fib.xf_CLen);
#endif

  if(!xbuf->xb_FirstChunk)
    xbuf->xb_FirstChunk=ulen;
  if(ulen > xbuf->xb_FirstChunk)
    return (xbuf->xb_Result=XPKERR_BADPARAMS);

  /* no packing? */
  if(xbuf->xb_Flags & XMF_NOPACK) {
    hookwrite(xbuf, XIO_WRITE, buf, ulen);
    xbuf->xb_Fib.xf_UCur += ulen;
    xbuf->xb_Fib.xf_CCur += ulen;
    xbuf->xb_Fib.xf_CLen += ulen;
    xbuf->xb_Fib.xf_NLen=Min(xbuf->xb_InLen - xbuf->xb_Fib.xf_UCur,
			       (int) xbuf->xb_ChunkSize);
    return xbuf->xb_Result;
  }

  /* Write the GlobHdr */
  if(!(xbuf->xb_Flags & XMF_GLOBHDR)) {
    if(!xbuf->xb_Password)
      memcpy(head->h_Glob.xsh_Initial, buf, Min(16, ulen));
    xbuf->xb_Flags |= XMF_GLOBHDR;
    if(!(hookwrite(xbuf, XIO_WRITE, &head->h_Glob,
    sizeof(struct XpkStreamHeader))))
      return xbuf->xb_Result;
    xbuf->xb_Fib.xf_CCur += sizeof(struct XpkStreamHeader);
  }

  /* Allocate the buffer */
  outbuflen=ROUNDLONG(ulen + (ulen>>5) + head->h_LocSize) + XPK_MARGIN;
  if(!(outbuf=(char *) hookwrite(xbuf, XIO_GETBUF, NULL, outbuflen)))
    return xbuf->xb_Result;

  outbuf += head->h_LocSize;
  /* compress to behind local header. This is needed by mem-out hook! */

  if(ulen < xbuf->xb_SubInfo->xi_MinPkInChunk)
    goto copychunk;

  /* Pack the chunk */
  xpar=&xbuf->xb_PackParam;
  xpar->xsp_InBuf=buf;
  xpar->xsp_InLen=ulen;
  xpar->xsp_OutBuf=outbuf;
  xpar->xsp_OutBufLen=outbuflen - head->h_LocSize;
  xpar->xsp_Number += 1;
  xpar->xsp_Mode=xbuf->xb_PackingMode;
  xpar->xsp_Password=xbuf->xb_Password;
  xpar->xsp_LibVersion=xbuf->xb_SubInfo->xi_LibVersion;

  if((XpksPackChunk=(int(*)(struct XpkSubParams *))GetProcAddress(XpkSubBase, "LIBXpksPackChunk")))
    xbuf->xb_Result=XpksPackChunk(xpar);
  else
    xbuf->xb_Result=XPKERR_NOFUNC;

  xbuf->xb_Flags |= XMF_INITED;

  type=XPKCHUNK_PACKED;
  clen=xpar->xsp_OutLen;

  if(xbuf->xb_Result == XPKERR_EXPANSION) {
    xbuf->xb_Result=0;
copychunk:
    type=XPKCHUNK_RAW;
    clen=ulen;
    outbuf=buf;
  }

  if(xbuf->xb_Result)
    return xbuf->xb_Result;

  /* Write the chunk */
  head->h_Loc.xch_Word.xchw_Type=type;
  if(head->h_Glob.xsh_Flags & XPKSTREAMF_LONGHEADERS) {
    head->h_Loc.xch_Long.xchl_ULen=ulen;
    head->h_Loc.xch_Long.xchl_CLen=clen;
  } else {
    head->h_Loc.xch_Word.xchw_ULen=(short) ulen;
    head->h_Loc.xch_Word.xchw_CLen=(short) clen;
  }

  if((rclen=clen&3)) {
    clen -= rclen;
    /* copy the remaining bytes (max 3) */
    memcpy(&end, outbuf+clen, rclen);
  }

  head->h_Loc.xch_Word.xchw_CChk=cchecksum((unsigned int *) outbuf, clen >> 2)
    ^ end[0] ^ end[1];
  /* add the rest bytes to the checksum */
  
  head->h_Loc.xch_Word.xchw_HChk=0;
  head->h_Loc.xch_Word.xchw_HChk=hchecksum((char *) &head->h_Loc, head->h_LocSize);

#ifndef WORDS_BIGENDIAN
    xbuf->xb_Headers.h_Loc.xch_Long.xchl_CChk=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Long.xchl_CChk);
    xbuf->xb_Headers.h_Loc.xch_Word.xchw_CChk=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_CChk);
    if(xbuf->xb_Headers.h_Glob.xsh_Flags & XPKSTREAMF_LONGHEADERS) {
      xbuf->xb_Headers.h_Loc.xch_Long.xchl_ULen=_byteswapint32(xbuf->xb_Headers.h_Loc.xch_Long.xchl_ULen);
      xbuf->xb_Headers.h_Loc.xch_Long.xchl_CLen=_byteswapint32(xbuf->xb_Headers.h_Loc.xch_Long.xchl_CLen);
    } else {
      xbuf->xb_Headers.h_Loc.xch_Word.xchw_ULen=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_ULen);
      xbuf->xb_Headers.h_Loc.xch_Word.xchw_CLen=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_CLen);
    }
#endif
  if(!(hookwrite(xbuf, XIO_WRITE, &head->h_Loc, head->h_LocSize)))
    return xbuf->xb_Result;
#ifndef WORDS_BIGENDIAN
    xbuf->xb_Headers.h_Loc.xch_Long.xchl_CChk=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Long.xchl_CChk);
    xbuf->xb_Headers.h_Loc.xch_Word.xchw_CChk=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_CChk);
    if(xbuf->xb_Headers.h_Glob.xsh_Flags & XPKSTREAMF_LONGHEADERS) {
      xbuf->xb_Headers.h_Loc.xch_Long.xchl_ULen=_byteswapint32(xbuf->xb_Headers.h_Loc.xch_Long.xchl_ULen);
      xbuf->xb_Headers.h_Loc.xch_Long.xchl_CLen=_byteswapint32(xbuf->xb_Headers.h_Loc.xch_Long.xchl_CLen);
    } else {
      xbuf->xb_Headers.h_Loc.xch_Word.xchw_ULen=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_ULen);
      xbuf->xb_Headers.h_Loc.xch_Word.xchw_CLen=_byteswapint16(xbuf->xb_Headers.h_Loc.xch_Word.xchw_CLen);
    }
#endif

  if(!(hookwrite(xbuf, XIO_WRITE, outbuf, clen)))
    return xbuf->xb_Result;

  if(rclen) {
    if(!(hookwrite(xbuf, XIO_WRITE, &end, 4)))
      return xbuf->xb_Result;
    clen+=4;
  }

  head->h_Glob.xsh_ULen += ulen;
  
  xbuf->xb_Fib.xf_UCur += ulen;
  xbuf->xb_Fib.xf_CCur += head->h_LocSize + clen;
  xbuf->xb_Fib.xf_CLen=xbuf->xb_Fib.xf_CCur;
  xbuf->xb_Fib.xf_NLen=Min(max(xbuf->xb_InLen, head->h_Glob.xsh_ULen) -
    xbuf->xb_Fib.xf_UCur, (int) xbuf->xb_ChunkSize);

  return xbuf->xb_Result;
}

/*
 *   XpkClose() - finish (de)compressing an XPK file
 */
int XpkClose(struct XpkBuffer *xbuf)
{
  HMODULE XpkSubBase=xbuf->xb_SubBase;
  void (*XpksPackFree)(struct XpkSubParams *);
  void (*XpksUnpackFree)(struct XpkSubParams *);

  if(!xbuf)
    return 0;

#ifdef _DEBUG
  if(xbuf->xb_Result)
    DebugError("XpkClose: failed (%ld) before XpkClose", xbuf->xb_Result);
#endif

  if(xbuf->xb_Format == XPKMODE_PKSTD) {
    struct Headers *head=&xbuf->xb_Headers;
    int outlen;

    if(!xbuf->xb_Result && !(xbuf->xb_Flags & (XMF_GLOBHDR|XMF_NOPACK))) {
      hookwrite(xbuf, XIO_WRITE, &head->h_Glob, sizeof(struct XpkStreamHeader));
#ifdef _DEBUG
      if(xbuf->xb_Result) DebugError("XpkClose: failed to write globhdr");
#endif
      xbuf->xb_Fib.xf_CCur += sizeof(struct XpkStreamHeader);
    }
    if(!xbuf->xb_Result && !(xbuf->xb_Flags & XMF_NOPACK)) {
      /* Write final chunk header */
      memset(&head->h_Loc, 0, head->h_LocSize);
      head->h_Loc.xch_Word.xchw_Type=XPKCHUNK_END;
      head->h_Loc.xch_Word.xchw_HChk=0;
      head->h_Loc.xch_Word.xchw_HChk=hchecksum((char *) &head->h_Loc,
						 head->h_LocSize);

      hookwrite(xbuf, XIO_WRITE, &head->h_Loc, head->h_LocSize);
#ifdef _DEBUG
      if(xbuf->xb_Result) DebugError("XpkClose: failed to write lochdr");
#endif
      xbuf->xb_Fib.xf_CCur += head->h_LocSize;
      outlen=xbuf->xb_Fib.xf_CCur;

      /* Write global header */
      hookwrite(xbuf, XIO_SEEK, NULL, -outlen);
#ifdef _DEBUG
      if(xbuf->xb_Result) DebugError("XpkClose: failed to reset output");
#endif

      head->h_Glob.xsh_Pack=XPK_COOKIE;
      head->h_Glob.xsh_CLen=outlen - 8;
      head->h_Glob.xsh_HChk=0;
      head->h_Glob.xsh_HChk=hchecksum((char *) &head->h_Glob,
					sizeof(struct XpkStreamHeader));

#ifndef WORDS_BIGENDIAN
      head->h_Glob.xsh_Pack=_byteswapint32(head->h_Glob.xsh_Pack);
      head->h_Glob.xsh_CLen=_byteswapint32(head->h_Glob.xsh_CLen);
      head->h_Glob.xsh_ULen=_byteswapint32(head->h_Glob.xsh_ULen);
#endif
      hookwrite(xbuf, XIO_WRITE, &head->h_Glob,
		sizeof(struct XpkStreamHeader));
#ifndef WORDS_BIGENDIAN
      head->h_Glob.xsh_Pack=_byteswapint32(head->h_Glob.xsh_Pack);
      head->h_Glob.xsh_CLen=_byteswapint32(head->h_Glob.xsh_CLen);
      head->h_Glob.xsh_ULen=_byteswapint32(head->h_Glob.xsh_ULen);
#endif

#ifdef _DEBUG
      if(xbuf->xb_Result) DebugError("XpkClose: failed updating globalhdr");
#endif

      hookwrite(xbuf, XIO_SEEK, 0, outlen - sizeof(struct XpkStreamHeader));
#ifdef _DEBUG
      if(xbuf->xb_Result) DebugError("XpkClose: failed to SEEK to end of output");
#endif
    }
    xbuf->xb_Fib.xf_CLen=xbuf->xb_Fib.xf_CCur;
    xbuf->xb_Fib.xf_ULen=xbuf->xb_Fib.xf_UCur;

    /* Shut down */
    if(xbuf->xb_Flags & XMF_INITED) {
      /* No error checking needed since PackFree doesn't have to exist */
      if((XpksPackFree=(void(*)(struct XpkSubParams *))GetProcAddress(XpkSubBase, "LIBXpksPackFree")))
	XpksPackFree(&xbuf->xb_PackParam);
    }
  } else if(xbuf->xb_Format == XPKMODE_UPSTD && xbuf->xb_Flags & XMF_INITED) {
    /* No error checking needed since UnpackFree doesn't have to exist */
    if((XpksUnpackFree=(void(*)(struct XpkSubParams *))GetProcAddress(XpkSubBase, "LIBXpksUnpackFree")))
      XpksUnpackFree(&xbuf->xb_PackParam);
  }
  if(xbuf->xb_RHook) {
    hookread(xbuf, xbuf->xb_Result ? XIO_ABORT : XIO_FREE, NULL, 0);
#ifdef _DEBUG
    if(xbuf->xb_Result) DebugError("XpkClose: failed read ABORT/FREE");
#endif
  }

  if(xbuf->xb_WHook) {
    hookwrite(xbuf, xbuf->xb_Result ? XIO_ABORT : XIO_FREE, NULL, 0);
#ifdef _DEBUG
    if(xbuf->xb_Result) DebugError("XpkClose: failed write ABORT/FREE");
#endif
  }

  /* Send information to the user */
  parsegettags(xbuf);
/*
#ifdef _DEBUG
  DebugRunTime("XpkClose: InLen %ld, CLen %ld, ULen %ld, ID %.4s", xbuf->xb_InLen,
  	       xbuf->xb_Fib.xf_CLen, xbuf->xb_Fib.xf_ULen, &xbuf->xb_Fib.xf_ID);
#endif
*/
  return freebufs(xbuf);
}
