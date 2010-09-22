/* xpk/xpk.h -- the main xpk headers
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

#ifndef _XPK_XPK_H_
#define _XPK_XPK_H_

#define XPKNAME "libxpkmaster"

/*
 * The packing/unpacking tags
 *
 * (TRUE) or (FALSE) mean the default value given by xpkmaster.library
 */
#define XPK_TagBase	(TAG_USER + ('X'<<8) + 'P')
#define XTAG(a)		(XPK_TagBase+a)

/* Caller must supply ONE of these to tell Xpk#?ack where to get data from */
/* Process an entire named file */
#define XPK_InName	  XTAG(0x01)
/* File handle - start from curposition
 * If packing partial file, must also supply InLen */
#define XPK_InFH	  XTAG(0x02)
/* Single unblocked memory buffer, must also supply InLen */
#define XPK_InBuf	  XTAG(0x03)
/* Call custom Hook to read data
 * Must also supply InLen, when hook cannot do! (not for XPK unpacking) */
#define XPK_InHook	  XTAG(0x04)

/* Caller must supply 1 of these to tell Xpk#?ackFile where to send data to */
/* Write (or overwrite) this data file */
#define XPK_OutName	  XTAG(0x10)
/* File handle - write from curpos on */
#define XPK_OutFH	  XTAG(0x11)
/* Unblocked buf - must also supply OutBufLen */
#define XPK_OutBuf	  XTAG(0x12)
/* Master allocs OutBuf - ti_Data points to buf ptr */
#define XPK_GetOutBuf	  XTAG(0x13)
 /* Callback Hook to get output buffers */
#define XPK_OutHook	  XTAG(0x14)

/* Other tags for Pack/Unpack */
/* Length of data in input buffer */
#define XPK_InLen	  XTAG(0x20)
/* Length of output buffer     */
#define XPK_OutBufLen	  XTAG(0x21)
/* ti_Data pts to long to rec OutLen */
#define XPK_GetOutLen	  XTAG(0x22)
/* ti_Data pts to long to rec OutBufLen */
#define XPK_GetOutBufLen  XTAG(0x23)
/* Password for de/encoding    */
#define XPK_Password	  XTAG(0x24)
/* ti_Data points to buffer for errmsg */
#define XPK_GetError	  XTAG(0x25)
/* Memory type for output buffer */
#define XPK_OutMemType	  XTAG(0x26)
/* Bool: Passthru unrec. fmts on unpack */
#define XPK_PassThru	  XTAG(0x27)
/* Bool: Step down packmethod if nec. */
#define XPK_StepDown	  XTAG(0x28)
/* Call this Hook between chunks */
#define XPK_ChunkHook	  XTAG(0x29)
/* Do a FindMethod before packing */
#define XPK_PackMethod	  XTAG(0x2a)
/* Chunk size to try to pack with */
#define XPK_ChunkSize	  XTAG(0x2b)
/* Packing mode for sublib to use */
#define XPK_PackMode	  XTAG(0x2c)
/* Don't overwrite existing files */
#define XPK_NoClobber	  XTAG(0x2d)
/* Skip this tag               */
#define XPK_Ignore	  XTAG(0x2e)
/* Change priority for (un)packing */
#define XPK_TaskPri	  XTAG(0x2f)
/* File name for progress report */
#define XPK_FileName	  XTAG(0x30)
/* !!! obsolete !!!            */
#define XPK_ShortError	  XTAG(0x31)
/* Query available packers     */
#define XPK_PackersQuery  XTAG(0x32)
/* Query properties of a packer */
#define XPK_PackerQuery	  XTAG(0x33)
/* Query properties of packmode */
#define XPK_ModeQuery	  XTAG(0x34)
/* Lossy packing permitted? (FALSE)*/
#define XPK_LossyOK	  XTAG(0x35)
/* Ignore checksum             */
#define XPK_NoCRC         XTAG(0x36)
/* tags added for xfdmaster support (version 4 revision 25) */
/* 16 bit key (unpack only)	*/
#define XPK_Key16	  XTAG(0x37)
/* 32 bit key (unpack only)	*/
#define XPK_Key32	  XTAG(0x38)

/* tag added to support seek (version 5) */
#define XPK_NeedSeek	  XTAG(0x39) /* turn on Seek function usage	*/

/* preference depending tags added for version 4 - their default value
 * may depend on preferences, see <xpk/xpkprefs.h> for more info */
/* Use xfdmaster.library (FALSE) */
#define XPK_UseXfdMaster  XTAG(0x40)
/* Use packers in extern dir (TRUE)*/
#define XPK_UseExternals  XTAG(0x41)
/* automatic password req.? (FALSE)*/
#define XPK_PassRequest   XTAG(0x42)
/* use prefs semaphore ? (TRUE) */
#define XPK_Preferences   XTAG(0x43)
/* automatic chunk report ? (FALSE)*/
#define XPK_ChunkReport	  XTAG(0x44)

/* tags XTAG(0x50) to XTAG(0x6F) are for XpkPassRequest -- see below */
#define XPK_MARGIN	256	/* Safety margin for output buffer	*/

/*
 * The hook function interface
 */
/* Message passed to InHook and OutHook as the ParamPacket */
struct XpkIOMsg {
  /* Read/Write/Alloc/Free/Abort */
  unsigned int xiom_Type;
  /* The mem area to read from/write to */
  char  *xiom_Ptr;
  /* The size of the read/write	*/
  int  xiom_Size;
  /* The IoErr() that occurred */
  unsigned int xiom_IOError;
  /* Reserved for future use */
  unsigned int xiom_Reserved;
  /* Hook specific, will be set to 0 by */
  unsigned int xiom_Private1;
  /* master library before first use */
  unsigned int xiom_Private2;
  unsigned int xiom_Private3;
  unsigned int xiom_Private4;
};

/* The values for XpkIoMsg->Type */
#define XIO_READ    1
#define XIO_WRITE   2
#define XIO_FREE    3
#define XIO_ABORT   4
#define XIO_GETBUF  5
#define XIO_SEEK    6
#define XIO_TOTSIZE 7

/*
 * The progress report interface
 */

/* Passed to ChunkHook as the ParamPacket */
struct XpkProgress {
  /* Type of report: XPKPROG_#? numbers	 */
  unsigned int  xp_Type;
  /* Brief name of packer being used 		 */
  char *xp_PackerName;
  /* Descriptive name of packer being used 	 */
  char *xp_PackerLongName;
  /* Packing/unpacking message		 */
  char *xp_Activity;
  /* Name of file being processed, if available */
  char *xp_FileName;
  /* Amount of packed data already processed	 */
  unsigned int	 xp_CCur;
  /* Amount of unpacked data already processed */
  unsigned int	 xp_UCur;
  /* Amount of unpacked data in file		 */
  unsigned int	 xp_ULen;
  /* Compression factor so far		 */
  int	 xp_CF;
  /* Percentage done already			 */
  unsigned int	 xp_Done;
  /* Bytes per second, from beginning of stream */
  unsigned int	 xp_Speed;
  /* For future use				 */
  unsigned int	 xp_Reserved[8];
};
#define XPKPROG_START	1	/* crunching started */
#define XPKPROG_MID	2	/* somewhere in the mid */
#define XPKPROG_END	3	/* crunching is completed */

/*
 * The file info block
 */

struct XpkFib {
  /* Unpacked, packed, archive? */
  unsigned int xf_Type;
  /* Uncompressed length      */
  unsigned int xf_ULen;
  /* Compressed length        */
  unsigned int xf_CLen;
  /* Next chunk len           */
  unsigned int xf_NLen;
  /* Uncompressed bytes so far */
  unsigned int xf_UCur;
  /* Compressed bytes so far  */
  unsigned int xf_CCur;
  /* 4 letter ID of packer    */
  unsigned int xf_ID;
  /* 4 letter name of packer  */
  char  xf_Packer[6];
  /* Required sublib version     */
  unsigned short xf_SubVersion;
  /* Required masterlib version */
  unsigned short xf_MasVersion;
  /* Password?                */
  unsigned int xf_Flags;
  /* First 16 bytes of orig. file */
  char  xf_Head[16];
  /* Compression ratio        */
  int   xf_Ratio;
  /* For future use           */
  unsigned int xf_Reserved[8];
};

/* Not packed               */
#define XPKTYPE_UNPACKED 0
/* Packed file              */
#define XPKTYPE_PACKED   1
/* Archive                  */
#define XPKTYPE_ARCHIVE  2

/* Password needed          */
#define XPKFLAGS_PASSWORD (1<< 0)
/* Chunks are dependent     */
#define XPKFLAGS_NOSEEK   (1<< 1)
/* Nonstandard file format  */
#define XPKFLAGS_NONSTD   (1<< 2)
/* defines added for xfdmaster support (version 4 revision 25) */
/* 16 bit key - for decrunching */
#define XPKFLAGS_KEY16	  (1<< 3)
/* 32 bit key - for decrunching */
#define XPKFLAGS_KEY32	  (1<< 4)

/*
 * The error messages
 */
#define XPKERR_OK	  0
/* This function not implemented	*/
#define XPKERR_NOFUNC	   -1
/* No files allowed for this function	*/
#define XPKERR_NOFILES	   -2
/* Input error happened			*/
#define XPKERR_IOERRIN	   -3
/* Output error happened		*/
#define XPKERR_IOERROUT	   -4
/* Check sum test failed		*/
#define XPKERR_CHECKSUM	   -5
/* Packed file's version newer than lib */
#define XPKERR_VERSION	   -6
/* Out of memory			*/
#define XPKERR_NOMEM	   -7
/* For not-reentrant libraries		*/
#define XPKERR_LIBINUSE	   -8
/* Was not packed with this library	*/
#define XPKERR_WRONGFORM   -9
/* Output buffer too small		*/
#define XPKERR_SMALLBUF	   -10
/* Input buffer too large		*/
#define XPKERR_LARGEBUF	   -11
/* This packing mode not supported	*/
#define XPKERR_WRONGMODE   -12
/* Password needed for decoding		*/
#define XPKERR_NEEDPASSWD  -13
/* Packed file is corrupt		*/
#define XPKERR_CORRUPTPKD  -14
/* Required library is missing		*/
#define XPKERR_MISSINGLIB  -15
/* Caller's TagList was screwed up	*/
#define XPKERR_BADPARAMS   -16
/* Would have caused data expansion	*/
#define XPKERR_EXPANSION   -17
/* Cannot find requested method		*/
#define XPKERR_NOMETHOD    -18
/* Operation aborted by user		*/
#define XPKERR_ABORTED     -19
/* Input file is truncated		*/
#define XPKERR_TRUNCATED   -20
/* Better CPU required for this library	*/
#define XPKERR_WRONGCPU    -21
/* Data are already XPacked		*/
#define XPKERR_PACKED      -22
/* Data not packed			*/
#define XPKERR_NOTPACKED   -23
/* File already exists			*/
#define XPKERR_FILEEXISTS  -24
/* Master library too old		*/
#define XPKERR_OLDMASTLIB  -25
/* Sub library too old			*/
#define XPKERR_OLDSUBLIB   -26
/* Cannot encrypt			*/
#define XPKERR_NOCRYPT     -27
/* Can't get info on that packer	*/
#define XPKERR_NOINFO      -28
/* This compression method is lossy	*/
#define XPKERR_LOSSY       -29
/* Compression hardware required	*/
#define XPKERR_NOHARDWARE  -30
/* Compression hardware failed		*/
#define XPKERR_BADHARDWARE -31
/* Password was wrong			*/
#define XPKERR_WRONGPW     -32
/* unknown error cause			*/
#define XPKERR_UNKNOWN	   -33
/* password request reached time out	*/
#define XPKERR_REQTIMEOUT  -34

/* Maximum size of an error message	*/
#define XPKERRMSGSIZE	80

/*
 *     The XpkQuery() call
 */
struct XpkPackerInfo {
  /* Brief name of the packer      */
  char  xpi_Name[24];
  /* Full name of the packer       */
  char  xpi_LongName[32];
  /* One line description of packer */
  char  xpi_Description[80];
  /* Defined below                 */
  unsigned int xpi_Flags;
  /* Max input chunk size for packing */
  unsigned int xpi_MaxChunk;
  /* Default packing chunk size    */
  unsigned int xpi_DefChunk;
  /* Default mode on 0..100 scale  */
  unsigned short xpi_DefMode;
};

/* Defines for Flags */
/* Library supplies chunk packing   */
#define XPKIF_PK_CHUNK   (1<< 0)
/* Library supplies stream packing  */
#define XPKIF_PK_STREAM  (1<< 1)
/* Library supplies archive packing */
#define XPKIF_PK_ARCHIVE (1<< 2)
/* Library supplies chunk unpacking */
#define XPKIF_UP_CHUNK   (1<< 3)
/* Library supplies stream unpacking */
#define XPKIF_UP_STREAM  (1<< 4)
/* Library supplies archive unpacking */
#define XPKIF_UP_ARCHIVE (1<< 5)
/* Uses full Hook I/O               */
#define XPKIF_HOOKIO     (1<< 7)
/* Does its own data checking       */
#define XPKIF_CHECKING   (1<<10)
/* Unpacker pre-reads the next chunkhdr */
#define XPKIF_PREREADHDR (1<<11)
/* Sub library supports encryption  */
#define XPKIF_ENCRYPTION (1<<13)
/* Sub library requires encryption  */
#define XPKIF_NEEDPASSWD (1<<14)
/* Sub library has different XpkMode's */
#define XPKIF_MODES      (1<<15)
/* Sub library does lossy compression */
#define XPKIF_LOSSY      (1<<16)
 /* unpacker does not support seeking	 */
#define XPKIF_NOSEEK	 (1<<17)

struct XpkMode {
  /* Chain to next descriptor for ModeDesc list*/
  struct XpkMode *xm_Next;
  /* Maximum efficiency handled by this mode */
  unsigned int xm_Upto;
  /* Defined below                         */
  unsigned int xm_Flags;
  /* Extra memory required during packing  */
  unsigned int xm_PackMemory;
  /* Extra memory during unpacking         */
  unsigned int xm_UnpackMemory;
  /* Approx packing speed in K per second  */
  unsigned int xm_PackSpeed;
  /* Approx unpacking speed in K per second */
  unsigned int xm_UnpackSpeed;
  /* CF in 0.1%				 */
  unsigned short xm_Ratio;
  /* Desired chunk size in K (!!) for this mode*/
  unsigned short xm_ChunkSize;
  /* 7 character mode description     */
  char  xm_Description[10];
};

/* Defines for XpkMode.Flags */
/* Timings on old standard environment, obsolete */
#define XPKMF_A3000SPEED (1<< 0)
/* Packing not heavily CPU dependent	*/
#define XPKMF_PK_NOCPU   (1<< 1)
/* Unpacking... (i.e. hardware modes)	*/
#define XPKMF_UP_NOCPU   (1<< 2)

#define MAXPACKERS 100

struct XpkPackerList {
  unsigned int xpl_NumPackers;
/* NOTE: This isn't 100% compatible with the Amiga version! */
  unsigned int xpl_Packer[MAXPACKERS];
};

/*
 * The XpkSeek() call (library version 5)
 */
#define XPKSEEK_BEGINNING	-1
#define XPKSEEK_CURRENT		0
#define XPKSEEK_END		1

/*
 * The XpkPassRequest() call (library version 4)
 */
/* which chars should be used */
#define XPK_PassChars	  XTAG(0x50)
/* buffer to write password to */
#define XPK_PasswordBuf   XTAG(0x51)
/* size of password buffer */
#define XPK_PassBufSize   XTAG(0x52)
/* pointer to unsigned short var for key data */
#define XPK_Key16BitPtr	  XTAG(0x53)
/* pointer to unsigned int var for key data*/
#define XPK_Key32BitPtr	  XTAG(0x54)
/* pointer to struct Screen */
#define XPK_PubScreen	  XTAG(0x55)
/* Text shown in Screen title */
#define XPK_PassTitle	  XTAG(0x56)
/* Timeout time of requester in seconds */
#define XPK_TimeOut	  XTAG(0x57)
/* request position and verify tags (version 4 revision 25) */
/* distance from left screen border */
#define XPK_PassWinLeft	  XTAG(0x58)
/* distance form top screen border */
#define XPK_PassWinTop	  XTAG(0x59)
/* width of requester window */
#define XPK_PassWinWidth  XTAG(0x5A)
/* height of requester window */
#define XPK_PassWinHeight XTAG(0x5B)
/* Left and Top are used as center crds */
#define XPK_PassCenter    XTAG(0x5C)
/* force user to verify password */
#define XPK_PassVerify	  XTAG(0x5D)

/* XPKPASSFF defines for XPK_PassChars. Do not use. Use XPKPASSFLG defines */
/* all numbers		*/
#define XPKPASSFF_30x39		(1<< 0)
/* chars 'A' to 'F'	*/
#define XPKPASSFF_41x46		(1<< 1)
/* chars 'a' to 'f'	*/
#define XPKPASSFF_61x66		(1<< 2)
/* chars 'G' to 'Z'	*/
#define XPKPASSFF_47x5A		(1<< 3)
/* chars 'g' to 'z'	*/
#define XPKPASSFF_67x7A		(1<< 4)
/* space character	*/
#define XPKPASSFF_20		(1<< 5)
/* special 7 bit chars */
#define XPKPASSFF_SPECIAL7BIT	(1<< 6)
/* all chars 0x20 to 0x7E without above defined */
/* upper special chars	*/
#define XPKPASSFF_C0xDE		(1<< 7)
/* lower special chars	*/
#define XPKPASSFF_DFxFF		(1<< 8)
/* special 8 bit chars	*/
#define XPKPASSFF_SPECIAL8BIT	(1<< 9)
/* all chars 0xA0 to 0xBF */

/* Control characters (0x00 to 0x1F, 0x7F and 0x80 to 0x9F) are not
 * useable. This also means carriage return, linefeed, tab stop and
 * other controls are not usable.
 */

/* flags for XPK_PassChars, XPKPASSFLG_PRINTABLE is default
 *
 * NUMERIC	: numbers
 * HEXADECIMAL	: hex numbers
 * ALPHANUMERIC	: numbers and letters
 * INTALPHANUM	: numbers and international letters
 * ASCII7	: 7 Bit ASCII
 * PRINTABLE	: all printable characters
 */
#define XPKPASSFLG_NUMERIC	XPKPASSFF_30x39
#define XPKPASSFLG_HEXADECIMAL	(XPKPASSFF_30x39|XPKPASSFF_41x46|XPKPASSFF_61x66)
#define XPKPASSFLG_ALPHANUMERIC	(XPKPASSFLG_HEXADECIMAL|XPKPASSFF_47x5A|XPKPASSFF_67x7A)
#define XPKPASSFLG_INTALPHANUM	(XPKPASSFLG_ALPHANUMERIC|XPKPASSFF_C0xDE|XPKPASSFF_DFxFF)
#define XPKPASSFLG_ASCII7	(XPKPASSFLG_ALPHANUMERIC|XPKPASSFF_SPECIAL7BIT)
#define XPKPASSFLG_PRINTABLE	(XPKPASSFLG_INTALPHANUM|XPKPASSFF_SPECIAL7BIT|XPKPASSFF_SPECIAL8BIT|XPKPASSFF_20)

/*
 * The XpkAllocObject() call (library version 4)
 *
 * use this always with library version >= 4, do NO longer allocate the
 * structures yourself
 */
#define XPKOBJ_FIB		0	/* XpkFib structure */
#define XPKOBJ_PACKERINFO	1	/* XpkPackerInfo structure */
#define XPKOBJ_MODE		2	/* XpkMode structure */
#define XPKOBJ_PACKERLIST	3	/* XpkPackerList structure */

 /*
#ifndef NO_INLINE_STDARG
#define XpkExamineTags(fib, tags...) ({unsigned int _tags[] = {tags}; XpkExamine((fib), (struct TagItem *)_tags);})
#define XpkPackTags(tags...) ({unsigned int _tags[] = {tags}; XpkPack((struct TagItem *)_tags);})
#define XpkUnpackTags(tags...) ({unsigned int _tags[] = {tags}; XpkUnpack((struct TagItem *)_tags);})
#define XpkOpenTags(xbuf, tags...) ({unsigned int _tags[] = {tags}; XpkOpen((xbuf), (struct TagItem *)_tags);})
#define XpkQueryTags(tags...) ({unsigned int _tags[] = {tags}; XpkQuery((struct TagItem *)_tags);})
#define XpkAllocObjectTags(type, tags...) ({unsigned int _tags[] = {tags}; XpkAllocObject((type), (struct TagItem *)_tags);})
#define XpkPassRequestTags(tags...) ({unsigned int _tags[] = {tags}; XpkPassRequest((struct TagItem *)_tags);})
#endif
*/

#endif /* _XPK_XPK_H_ */
