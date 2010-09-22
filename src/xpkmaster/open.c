/* open.c -- Opening and initialisation routines for XPK files
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
#include <memory.h>
#include "xpkprefs.h"
#include "xpkmaster.h"
#include "texts.h"

static const struct XpkInfo DONTInfo={ 1,0,0,1,"DONT","Copy", 0, 0x55534552,
				       XPKIF_PK_CHUNK|XPKIF_UP_CHUNK,
				       DEFAULTCHUNKSIZE, 1, DEFAULTCHUNKSIZE,
				       0,0,0,0,100,0,0,{0,0,0,0,0,0} };
/* XpkMode is not initialized! Should not be used anywhere */

static int xpkopenwrite(struct XpkBuffer **, struct TagItem *);
static int GetPrefsPacker(struct XpkBuffer *);
static int GetPassword(struct XpkBuffer *, struct TagItem *, unsigned int);
static struct XpkTypeData *BufRecog(unsigned int, struct XpkBuffer *);

int xpkopen(struct XpkBuffer **xbufp, struct TagItem *tags, unsigned int examine)
{
  struct XpkBuffer *xbuf;
  struct XpkStreamHeader *globhdr;
  struct XpkFib *fib;

#if defined(_DEBUG) 
  DebugRunTime("xpkopen");
#endif

  if(!(*xbufp=xbuf=initxbuf()))
    return parseerrortags(tags, XPKERR_NOMEM);

  globhdr=&xbuf->xb_Headers.h_Glob;
  fib=&xbuf->xb_Fib;

  if(parsebuftags(xbuf, tags))
    goto Abort;

  /* Call pack open function */
  if(xbuf->xb_Flags & XMF_PACKING)
    return xpkopenwrite(xbufp, tags);

  /* Read first longword */
  if(!hookread(xbuf, XIO_READ, globhdr, 4)) {
    if(xbuf->xb_Result != XPKERR_TRUNCATED)
      goto Abort;
    /* else handle now as uncompressed file */
  }
  
#ifndef WORDS_BIGENDIAN
  globhdr->xsh_Pack=_byteswapint32(globhdr->xsh_Pack);
#endif

  /* Standard XPK file */
  if(globhdr->xsh_Pack == XPK_COOKIE) {
    /* size of extended header if present */
    unsigned short exthlen=0;
    void *XpkSubBase;

    xbuf->xb_Format=XPKMODE_UPSTD;

    /* Read rest of the global header */
    if(!hookread(xbuf, XIO_READ, (char *)globhdr + 4, sizeof(struct XpkStreamHeader) - 4))
      goto Abort;
    
    if(hchecksum((char *) globhdr, sizeof(struct XpkStreamHeader))) {
      xbuf->xb_Result=XPKERR_CHECKSUM;
      goto Abort;
    }
    
    if(globhdr->xsh_Flags & XPKSTREAMF_LONGHEADERS)
      xbuf->xb_Headers.h_LocSize=sizeof(struct XpkChunkHdrLong);
    else
      xbuf->xb_Headers.h_LocSize=sizeof(struct XpkChunkHdrWord);

    if(globhdr->xsh_Flags & XPKSTREAMF_EXTHEADER) {
      if(!hookread(xbuf, XIO_READ, &exthlen, sizeof(unsigned short)))
	goto Abort;
      if(!hookread(xbuf, XIO_READ, 0, exthlen))
	goto Abort;
      /* for unwinding while XpkExamine */
      exthlen += sizeof(unsigned short);
    }

#ifndef WORDS_BIGENDIAN
    globhdr->xsh_CLen=_byteswapint32(globhdr->xsh_CLen);
    globhdr->xsh_ULen=_byteswapint32(globhdr->xsh_ULen);
#endif

    /* first lochdr */
    if(!hookread(xbuf, XIO_READ, &xbuf->xb_Headers.h_Loc,
		 xbuf->xb_Headers.h_LocSize))
      goto Abort;

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

    fib->xf_CCur=sizeof(struct XpkStreamHeader);
    if(updatefib(xbuf))
      goto Abort;
    xbuf->xb_InLen=fib->xf_CLen;

    if(!(XpkSubBase=opensub(xbuf, globhdr->xsh_Type)))
      goto Abort;

    if(globhdr->xsh_SubVrs > xbuf->xb_SubInfo->xi_LibVersion) {
      xbuf->xb_Result=XPKERR_OLDSUBLIB;
      goto Abort;
    }

    xbuf->xb_Prog.xp_Activity=xbuf->xb_SubInfo->xi_UnpackMsg ?
      xbuf->xb_SubInfo->xi_UnpackMsg : strings[TXT_UNPACKING_UPPER];
    xbuf->xb_Prog.xp_PackerName=xbuf->xb_SubInfo->xi_Name;
    xbuf->xb_LastMsg=xbuf->xb_SubInfo->xi_UnpackedMsg ?
      xbuf->xb_SubInfo->xi_UnpackedMsg : strings[TXT_UNPACKED];

    if(globhdr->xsh_Flags & XPKSTREAMF_PASSWORD)
      fib->xf_Flags |= XPKFLAGS_PASSWORD;

    if(examine && !hookread(xbuf, XIO_SEEK, 0,
			    -(sizeof(struct XpkStreamHeader) +
			      xbuf->xb_Headers.h_LocSize+exthlen)))
      goto Abort;

    goto Exit;
  }

  if(!hookread(xbuf, XIO_SEEK, 0, -xbuf->xb_RMsg.xmm_Size))
    /* redo last read bytes */
    goto Abort;

  if(xbuf->xb_InLen == 0xFFFFFFFF) {
    /* get input length */
    if(!hookread(xbuf, XIO_TOTSIZE, 0, 0))
      goto Abort;
    else if(xbuf->xb_RMsg.xmm_Size)
      xbuf->xb_InLen=xbuf->xb_RMsg.xmm_Size;
  }

  fib->xf_CLen=xbuf->xb_InLen;

  /* Unpacked file */
  if(examine || xbuf->xb_Flags & XMF_PASSTHRU) {
    xbuf->xb_Format=XPKMODE_UPUP;

    fib->xf_Type=XPKTYPE_UNPACKED;
    fib->xf_ULen=xbuf->xb_InLen;
    fib->xf_NLen=Min(DEFAULTCHUNKSIZE, xbuf->xb_InLen) + XPK_MARGIN;
    fib->xf_ID=ROW_OF_MINUS;

    xbuf->xb_Prog.xp_Activity=strings[TXT_READING];
    xbuf->xb_Prog.xp_PackerName="Master";
    xbuf->xb_LastMsg=strings[TXT_READ];
    
    /* if != 0 it was XPKERR_TRUNCATED */
    xbuf->xb_Result=XPKERR_OK;

    goto Exit;
  }

  /* Can't unpack, can't passthru */
  xbuf->xb_Result=XPKERR_NOTPACKED;

 Abort:
  *xbufp=0;
  return XpkClose((struct XpkBuffer *)xbuf);
  
 Exit:
  if(!examine && (((fib->xf_Flags & XPKFLAGS_PASSWORD) &&
		   !xbuf->xb_Password) ||
		  ((fib->xf_Flags & XPKFLAGS_KEY16) &&
		   !(xbuf->xb_Flags & XMF_KEY16)) ||
		  ((fib->xf_Flags & XPKFLAGS_KEY32) &&
		   !(xbuf->xb_Flags & XMF_KEY32))) &&
     (xbuf->xb_Result=GetPassword(xbuf, tags, 0)))
    goto Abort;
  
  return XPKERR_OK;
}

/***************************** Open for packing *************************/
static int xpkopenwrite(struct XpkBuffer **xbufp, struct TagItem *tags)
{
  struct XpkBuffer *xbuf=*xbufp;
  struct XpkStreamHeader *globhdr	= &xbuf->xb_Headers.h_Glob;
  void *XpkSubBase;
  int res;

  xbuf->xb_Format=XPKMODE_PKSTD;

  if(xbuf->xb_InLen == 0xFFFFFFFF && xbuf->xb_RHook) {
    /* get input length */
    if(!hookread(xbuf, XIO_TOTSIZE, 0, 0))
      return xbuf->xb_Result;
    else if(xbuf->xb_RMsg.xmm_Size)
      xbuf->xb_InLen=xbuf->xb_RMsg.xmm_Size;
  }

  xbuf->xb_Fib.xf_ULen=xbuf->xb_InLen;

  if(!(XpkSubBase=xbuf->xb_SubBase) &&  /* Do we know the sublib? */
     (xbuf->xb_Result=GetPrefsPacker(xbuf)))
    goto Abort; /* no sublib and no prefs packer finder */

  if(xbuf->xb_Password && !(xbuf->xb_SubInfo->xi_Flags & XPKIF_ENCRYPTION)) {
    xbuf->xb_Result=XPKERR_NOCRYPT;
    goto Abort;
  }

  if(!xbuf->xb_Password && (xbuf->xb_SubInfo->xi_Flags & XPKIF_NEEDPASSWD)) {
    /* automatic password requester */
    if((xbuf->xb_Result=GetPassword(xbuf, tags, 1)))
      goto Abort;
  }

  if(!(xbuf->xb_Flags & XMF_LOSSYOK) &&
     xbuf->xb_SubInfo->xi_Flags & XPKIF_LOSSY) {
    xbuf->xb_Result=XPKERR_LOSSY;
    goto Abort;
  }

  if(xbuf->xb_PackingMode > 100)	/* Is packing mode valid? */
    xbuf->xb_PackingMode=100;		/* Use max */

  if(xbuf->xb_InLen != 0xFFFFFFFF) {
    if(!hookwrite(xbuf, XIO_TOTSIZE, 0, ROUNDLONG
		  (xbuf->xb_InLen + (xbuf->xb_InLen >> 5)) + (XPK_MARGIN<<1)))
      goto Abort;
  }

  /* Find the chunk size */
  if((xbuf->xb_ChunkSize == 0) &&
  ((xbuf->xb_ChunkSize=xbuf->xb_SubInfo->xi_DefPkInChunk) == 0))
    xbuf->xb_ChunkSize=DEFAULTCHUNKSIZE;
  if(xbuf->xb_ChunkSize < xbuf->xb_SubInfo->xi_MinPkInChunk)
    xbuf->xb_ChunkSize=xbuf->xb_SubInfo->xi_MinPkInChunk;
  if((xbuf->xb_SubInfo->xi_MaxPkInChunk) &&
  (xbuf->xb_ChunkSize > xbuf->xb_SubInfo->xi_MaxPkInChunk))
    xbuf->xb_ChunkSize=xbuf->xb_SubInfo->xi_MaxPkInChunk;

  /* Prepare global header */
  globhdr->xsh_Pack=0;		/* Initialize the global header */
  globhdr->xsh_Type=xbuf->xb_SubID;

  /* (0xFFFF-XPK_MARGIN) and some bytes security */
  if(xbuf->xb_ChunkSize > 65000)
    globhdr->xsh_Flags |= XPKSTREAMF_LONGHEADERS;
  if(xbuf->xb_Password)
    globhdr->xsh_Flags |= XPKSTREAMF_PASSWORD;

  xbuf->xb_Headers.h_LocSize=globhdr->xsh_Flags & XPKSTREAMF_LONGHEADERS
    ? sizeof (struct XpkChunkHdrLong)
    : sizeof (struct XpkChunkHdrWord);

  /* Read first 16 bytes */
  memset(globhdr->xsh_Initial, 0xff, 16);

  xbuf->xb_Prog.xp_Activity=xbuf->xb_SubInfo->xi_PackMsg ?
  xbuf->xb_SubInfo->xi_PackMsg : strings[TXT_PACKING_UPPER];
  xbuf->xb_Prog.xp_PackerName=xbuf->xb_SubInfo->xi_Name;
  xbuf->xb_LastMsg=xbuf->xb_SubInfo->xi_PackedMsg ?
  xbuf->xb_SubInfo->xi_PackedMsg : strings[TXT_PACKED];

Abort:
  xbuf->xb_Fib.xf_NLen=Min(xbuf->xb_InLen - xbuf->xb_Fib.xf_UCur,
  xbuf->xb_ChunkSize);

  if((res=xbuf->xb_Result))
    res=XpkClose((struct XpkBuffer *)xbuf);

  return res;
}

typedef struct XpkTypeData * (*RecogFunc) (char *, char *, unsigned int, unsigned int, struct TagItem *);

static int GetPrefsPacker(struct XpkBuffer *xbuf)
{
  int ret=XPKERR_UNKNOWN;

#ifdef NOT_IMPLEMENTED
  unsigned int bufsize;
  struct XpkTypeData *td=0;

  if((xbuf->xb_Flags & XMF_NOPREFS) || !xbuf->xb_RHook)
    return XPKERR_BADPARAMS;

  bufsize=Min(xbuf->xb_InLen, sem->xps_RecogSize);

  if(sem->xps_RecogFunc && (td=BufRecog(bufsize, xbuf)) ==
  (struct XpkTypeData *) 0xFFFFFFFF)
    td=BufRecog(xbuf->xb_InLen, xbuf, sem);

  if(!td || td == (struct XpkTypeData *) 0xFFFFFFFF)
    td=sem->xps_MainPrefs ? sem->xps_MainPrefs->xmp_DefaultType : 0;

  if(td) {
    if(td->xtd_Flags & XTD_NoPack) {
      xbuf->xb_Flags |= XMF_NOPACK;
      xbuf->xb_SubInfo=(struct XpkInfo *) &DONTInfo;
      ret=XPKERR_OK;
    }
    else if(!(td->xtd_Flags & XTD_ReturnError)) {
      void *XpkSubBase;
      struct XpkInfo *subinfo;
      struct XpkInfo *(*XpksPackerInfo)();

      if((XpkSubBase=opensub(xbuf, td->xtd_StdID))) {
	ret=XPKERR_OK;
	if((XpksPackerInfo=dlsym(XpkSubBase, "LIBXpksPackerInfo"))) {
	  subinfo=XpksPackerInfo();

	  xbuf->xb_ChunkSize=td->xtd_ChunkSize;
	  xbuf->xb_PackingMode=( td->xtd_Mode ? td->xtd_Mode :
				 subinfo->xi_DefMode);
	  printf("%s\n", XpksPackerInfo()->xi_Description);
	} else
	  ret=XPKERR_NOFUNC;
      }
    }
    else
      ret=XPKERR_NOMETHOD;
  }

  if(td->xtd_Memory && td->xtd_MemorySize)
    free(td->xtd_Memory);

#endif
  return ret;
}

static int GetPassword(struct XpkBuffer *xbuf, struct TagItem *tags,
unsigned int verify)
{
  if(xbuf->xb_Flags & XMF_AUTOPASSWD) {
    if(xbuf->xb_Fib.xf_Flags & XPKFLAGS_KEY32) {
		TagItem __tags[] = {
			{ XPK_Key32BitPtr, &xbuf->xb_PassKey32 },
			{ TAG_MORE, tags },
			{ TAG_DONE, NULL }
		};
      xbuf->xb_Result=XpkPassRequest(__tags);
      xbuf->xb_Flags |= XMF_KEY32;
    } else if(xbuf->xb_Fib.xf_Flags & XPKFLAGS_KEY16) {
		TagItem __tags[] = {
			{ XPK_Key16BitPtr, &xbuf->xb_PassKey16 },
			{ TAG_MORE, tags},
			{ TAG_DONE,NULL }
		};
      xbuf->xb_Result=XpkPassRequest(__tags);
      xbuf->xb_Flags |= XMF_KEY16;
    } else {
      if(!(xbuf->xb_Password=(char *) malloc(AUTO_PASS_SIZE)))
	return XPKERR_NOMEM;
      xbuf->xb_PasswordSize=AUTO_PASS_SIZE;
      xbuf->xb_Flags |= XMF_OWNPASSWORD; /* must be freed later */
	  TagItem __tags[] = {
			{ XPK_PasswordBuf,xbuf->xb_Password },
			{ XPK_PassBufSize,(void*)xbuf->xb_PasswordSize },
			{ XPK_PassVerify, (void*)verify },
			{ TAG_MORE, tags },
			{ TAG_DONE, NULL }
	  };
      xbuf->xb_Result=XpkPassRequest(__tags);
    }
    
    return xbuf->xb_Result;
  }
  return XPKERR_NEEDPASSWD;
}

static struct XpkTypeData *BufRecog(unsigned int bufsize, struct XpkBuffer *xbuf)
{
  char *bufptr;
  struct XpkTypeData *ret=0;
  struct TagItem tag[]={
  { XPK_FileName, 0},
  { XPK_PackMode, 0},
  { TAG_DONE, 0}};

  tag[0].ti_Data=xbuf->xb_Prog.xp_FileName;
  tag[1].ti_Data=(void *)xbuf->xb_PackingMode;

  if((bufptr=(char *) hookread(xbuf, XIO_READ, 0, bufsize))) {
#ifdef NOT_IMPLEMENTED
    ret=(((RecogFunc) sem->xps_RecogFunc) (bufptr, xbuf->xb_RMsg.xmm_FileName,
					   bufsize, xbuf->xb_InLen, tag));
#endif
    hookread(xbuf, XIO_SEEK, 0, -bufsize);
  }
  return ret;
}
