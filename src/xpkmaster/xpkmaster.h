/* xpkmaster.h -- Master library global definitions, declarations and protos
 * Copyright (C) 1996-1999 Dirk Stöcker
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

#ifndef _XPKMASTER_H_
#define _XPKMASTER_H_

#include <stdio.h>
#include <sys/types.h>
#include "amigalibs.h"
#include "xpk.h"
#include "xpksub.h"
#include "config.h"
#include <windows.h>

/* dll export */
#if defined(XPKMASTER_EXPORTS)
#define XPK_API __declspec(dllexport)
#else
#define XPK_API __declspec(dllimport)
#endif

/* AUTOCONF */
#define XPK_MAJOR_VERSION 0
#define XPK_MINOR_VERSION 0
#define XPK_MICRO_VERSION 1

typedef unsigned int (*regfunc) (void *);
#define ROUNDLONG(x)	 (((x)+3)&(~3)) /* round to next longword */
#define DEFAULTCHUNKSIZE 0x8000

#ifndef Min
  #define Min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
	#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

/* The structure used for I/O, special master library version */
struct XpkMasterMsg {
  unsigned int xmm_Type;    /* Read/Write/Alloc/Free/Abort		*/
  char     *xmm_Ptr;	 /* The mem area to read from/write to	*/
  int       xmm_Size;	 /* The size of the read/write		*/
  unsigned int xmm_IOError; /* The IoErr() that occurred		*/
  unsigned int xmm_Reserved;/* Reserved for future use		*/
  char     *xmm_Buf;	 /* Specific to the internal hooks	*/
  int       xmm_Error;	 /* The XPKERR that occurred		*/
  unsigned int xmm_BufLen;
  unsigned int xmm_BufOfs;
  unsigned int xmm_Len;
  unsigned int xmm_Flags;
  FILE     *xmm_FH;
  unsigned int xmm_MemType;
  char     *xmm_FileName;
};

#define XIO_GETOUTBUF 1		/* flag for xmm_Flags - allocate buffer	*/

/* These structures define the file format for compressed streams */
struct XpkStreamHeader {
  unsigned int xsh_Pack;
  unsigned int xsh_CLen;
  unsigned int xsh_Type;
  unsigned int xsh_ULen;
  char  xsh_Initial[16];
  char  xsh_Flags;
  char  xsh_HChk;
  char  xsh_SubVrs;
  char  xsh_MasVrs;
};

#define XPK_COOKIE	0x58504b46 	/* 'XPKF' - ID for xsh_Pack */
#define ROW_OF_MINUS	0x2d2d2d2d	/* '----' */
#define XFD_COOKIE	0x58464444	/* 'XFDD' */
#define USER_COOKIE	0x55534552	/* 'USER' */

#define XPKSTREAMF_LONGHEADERS  0x01	/* Use XpkLongLocHeaders	*/
#define XPKSTREAMF_PASSWORD     0x02	/* This file encoded		*/
#define XPKSTREAMF_EXTHEADER    0x04	/* Extended globhdr		*/

struct XpkChunkHdrWord {
  char  xchw_Type;
  char  xchw_HChk;
  unsigned short xchw_CChk;
  unsigned short xchw_CLen;
  unsigned short xchw_ULen;
};

struct XpkChunkHdrLong {
  char  xchl_Type;
  char  xchl_HChk;
  unsigned short xchl_CChk;
  unsigned int xchl_CLen;
  unsigned int xchl_ULen;
};

typedef union {
  struct XpkChunkHdrLong xch_Long;
  struct XpkChunkHdrWord xch_Word;
} XpkChunkHeader;

#define XPKCHUNK_RAW	0x00 /* raw copy of source */
#define XPKCHUNK_PACKED	0x01 /* packed data */
#define XPKCHUNK_END	0x0F /* empty end Chunk */

struct Headers {
  struct XpkStreamHeader h_Glob;
  XpkChunkHeader	 h_Loc;
  unsigned int		 h_LocSize;
};

#define XPKMODE_UPUP  1
#define XPKMODE_UPSTD 2
#define XPKMODE_UPPP  3
#define XPKMODE_UPXFD 4	
#define XPKMODE_PKSTD 20

#define AUTO_PASS_SIZE	50

struct SeekData {
  unsigned int sd_FilePos;
  unsigned int sd_ULen;
  unsigned int sd_CLen;
};

#define SEEKENTRYNUM	10

struct SeekDataList {
  struct SeekDataList *	sdl_Next;
  unsigned int		sdl_Used;
  struct SeekData	sdl_Data[SEEKENTRYNUM];
};

/* This is what XPK "handles" really point to */
struct XpkBuffer {
  struct XpkFib	xb_Fib;		/* file info about this file		*/
  unsigned int  xb_PackingMode;	/* desired packing efficiency, 0..100   */
  struct Headers xb_Headers;	/* global and local file header 	*/
  unsigned int    xb_Format;	/* type of file				*/
  int          xb_Result;	/* error code from last call		*/
  char        *xb_ErrBuf;	/* Where user wants the error		*/
  char       **xb_GetOutBuf;	/* Where user wants the out buf addr	*/
  unsigned int*xb_GetOutLen;	/* Where user wants the output len	*/
  unsigned int*xb_GetOutBufLen;	/* Where user wants the out buf len	*/
  unsigned int xb_Secs;		/* Start time, the seconds		*/
  unsigned int xb_Mics;		/* Start time, the micros		*/
  struct Hook *xb_RHook;	/* input data hook			*/
  struct Hook *xb_WHook;	/* output data hook			*/
  struct Hook *xb_ChunkHook;	/* Hook to call between chunks		*/
  char        *xb_Password;	/* password for de/encoding		*/
  unsigned int xb_PasswordSize;	/* password buffer size for own password*/
  unsigned int    xb_PassKey32;	/* password key, 32 bit			*/
  unsigned short    xb_PassKey16;	/* password key, 16 bit			*/
  int          xb_Priority;	/* task pri during packing		*/
  unsigned int    xb_SubID;	/* currently active sub ID		*/
  unsigned int xb_ChunkSize;	/* Chunk size to use for packing	*/
  unsigned int xb_FirstChunk;	/* First chunk size - largest chunk	*/
  unsigned int xb_Flags;	/* private for xpkmaster		*/
  unsigned int xb_InLen;	/* Number of bytes to (un)pack		*/
  unsigned int xb_UCur;		/* Current Uncrunched size (next chunk)	*/
  unsigned int xb_CCur;		/* Current Crunched size (next chunk)	*/
  unsigned int xb_InBufferPos;	/* buffer position for seek		*/
  char        *xb_LastMsg;	/* The last progress message		*/
  struct xfdBufferInfo *xb_xfd; /* xfdBufferInfo			*/
  struct XpkInfo *xb_SubInfo;	/* Info of current open sub-lib		*/
  HMODULE	xb_SubBase;	/* Currently open sub-library		*/
  struct XpkMasterMsg xb_RMsg;	/* Parameters for reading		*/
  struct XpkMasterMsg xb_WMsg;	/* Parameters for writing		*/
  struct XpkSubParams xb_PackParam;/* Parameters to (Un)PackChunk()	*/
  struct XpkProgress  xb_Prog;	/* Parameters to progress report	*/
  struct SeekDataList *xb_SeekDataList; /* this is used for seek	*/
};

/* Values for MasterFlags */
#define XMF_PRIVFH	(1<< 0)	/* We opened the FH, so we close it.	*/
#define XMF_PACKING	(1<< 1)	/* This is a packing operation.		*/
#define XMF_PASSTHRU	(1<< 2)	/* Pass uncompressed data through	*/
#define XMF_GETOUTBUF	(1<< 3)	/* Get output buffer when size known	*/
#define XMF_NOCLOBBER	(1<< 4)	/* Don't overwrite			*/
#define XMF_EOF		(1<< 5)	/* End of file				*/
#define XMF_INITED	(1<< 6)	/* Sublib buffers have been allocted	*/
#define XMF_GLOBHDR	(1<< 7)	/* GlobHdr is already written		*/
#define XMF_LOSSYOK	(1<< 8)	/* Lossy compression permitted		*/
#define XMF_OWNTASKPRI	(1<< 9) /* Altered task pri, restore		*/
#define XMF_NOCRC	(1<<10)	/* Do not check the checksum on decomp	*/
#define XMF_NOPREFS	(1<<11) /* Are prefs allowed ?			*/
#define XMF_XFD		(1<<12) /* Is XFD unpacking allowed ?		*/
#define XMF_EXTERNALS	(1<<13) /* Is extern allowed ?			*/
#define XMF_AUTOPASSWD	(1<<14) /* Automatic password			*/
#define XMF_AUTOPRHOOK  (1<<15) /* Automatic Progress hook		*/
#define XMF_NOPACK	(1<<16) /* destination file equals source	*/
#define XMF_OWNPASSWORD (1<<17) /* free password buffer later !!!	*/
#define XMF_KEY16	(1<<18) /* got a 16 bit key (needed, as key may */
#define XMF_KEY32	(1<<19) /* got a 32 bit key  be 0 - no check)   */
#define XMF_SEEK	(1<<20)	/* we need to built seek buffers	*/

#ifdef _DEBUG
/* debug.c */
void DebugError(char *, ...);
void DebugRunTime(char *, ...);
void DebugTagList(char *, struct TagItem *);
#endif

/* hook_fh.c */
extern struct Hook fhinhook;
extern struct Hook fhouthook;
/* hook_mem.c */
extern struct Hook meminhook;
extern struct Hook memouthook;

/* seek.c */
int addseek(struct XpkBuffer *);
/* progress.c */
int callprogress(struct XpkBuffer *);			
/* checksum.c */
unsigned short cchecksum(unsigned int *, unsigned int);
unsigned char hchecksum(char *, unsigned int);
/* sublibs.c */
void *opensub(struct XpkBuffer *, unsigned int);
void closesub(struct XpkBuffer *);
unsigned int idfromname(char *);
/* xbuf.h */
int freebufs(struct XpkBuffer *);
void freeseek(struct XpkBuffer *);
struct XpkBuffer *initxbuf(void);
/* objects.c */
void getUClen(struct XpkBuffer *, int *, int *);
/* hook.c */
char *hookread(struct XpkBuffer *, unsigned int, void *, int);
char *hookwrite(struct XpkBuffer *, unsigned int, void *, int);
/* tags.c */
int parsebuftags(struct XpkBuffer*, struct TagItem *);
int parseerrortags(struct TagItem *, int);
void parsegettags(struct XpkBuffer *);
char *FilePart(char *);
struct TagItem *FindTagItem(Tag, struct TagItem *);
struct TagItem *NextTagItem(struct TagItem **);
/* fib.c */
void percentages(struct XpkFib *);
int updatefib(struct XpkBuffer *);
/* open.c */
int	xpkopen(struct XpkBuffer **, struct TagItem *, unsigned int);
/* byteswap.c */
unsigned short _byteswapint16(unsigned short);
unsigned int _byteswapint32(unsigned int);

XPK_API int XpkExamine(struct XpkFib *, struct TagItem *);
XPK_API int XpkPack(struct TagItem *);
XPK_API int XpkUnpack(struct TagItem *);
XPK_API int XpkOpen(struct XpkBuffer **, struct TagItem *);
XPK_API int XpkRead(struct XpkBuffer *, char *, unsigned int);
XPK_API int XpkWrite(struct XpkBuffer *, char *, unsigned int);
XPK_API int XpkSeek(struct XpkBuffer *, int, int);
XPK_API int XpkClose(struct XpkBuffer *);
XPK_API int XpkQuery(struct TagItem *);
XPK_API char *XpkAllocObject(unsigned int, struct TagItem *);
XPK_API void XpkFreeObject(unsigned int, char *);
XPK_API char XpkPrintFault(int, char *);
XPK_API unsigned int XpkFault(int, char *, char *, unsigned int);
XPK_API int XpkPassRequest(struct TagItem *);
#ifdef NO_INLINE_STDARG
XPK_API int XpkPassRequestTags(Tag tag, ...);
#endif

#endif /* XPKMASTER_XPKMASTER_H */
