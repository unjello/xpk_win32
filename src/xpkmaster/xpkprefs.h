/* xpk/xpkprefs.h -- XPK prefs stuff
 * Copyright (C) 1996-2000 authors
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

#ifndef _XPK_XPKPREFS_H_
#define _XPK_XPKPREFS_H_

#define ID_XPKT  MAKE_ID('X','P','K','T')
#define ID_XPKM  MAKE_ID('X','P','K','M')

/*
 * XpkTypeData structure
 */
/* These two cannot be set same time: */
/* filetype should not be crunched */
#define XTD_NoPack		(1<<0)
/* return error XPKERR_NOMETHOD */
#define XTD_ReturnError		(1<<1)

struct XpkTypeData {
  unsigned int  xtd_Flags;	   /* see above XTD flags */
  unsigned int  xtd_StdID;	   /* holding the ID --> 'NUKE' */
  unsigned int  xtd_ChunkSize;    /* maybe useless with external crunchers */
  unsigned short  xtd_Mode;	   /* PackMode */
  unsigned short  xtd_Version;	   /* structure version --> 0 at the moment */
  char *xtd_Password;	   /* not used at the moment */
  char *xtd_Memory;	   /* memory pointer - when should be freed by */
  unsigned int  xtd_MemorySize;   /* memory size    - receiver (xpkmaster) */
};

/*
 * XpkTypePrefs structure
 */
/* These can both be set (in loading this means File AND Name Pattern have
 * to match), but one is needed: */
/* File Pattern is given */
#define XPKT_NamePattern	(1<<0)
/* Name Pattern is given */
#define XPKT_FilePattern	(1<<1)

struct XpkTypePrefs {
  /* See above XPKT Flags */
  unsigned int           xtp_Flags;
  /* Name of this file type (for prefs program) */
  char               *xtp_TypeName;
  /* Pointer to NamePattern */
  char               *xtp_NamePattern;
  /* Pointer to FilePattern */
  char               *xtp_FilePattern;
  struct XpkTypeData *xtp_PackerData;
};

/*
 *     XpkMainPrefs structure
 */
/* Use xfdmaster.library for unpacking */
#define XPKM_UseXFD		(1<<0)
/* Use xex libraries */
#define XPKM_UseExternals	(1<<1)
/* Use the automatic password requester */
#define XPKM_AutoPassword	(1<<2)

struct XpkMainPrefs {
  /* version of structure ==> 0 */
  unsigned int	      xmp_Version;
  /* above defined XPKM flags */
  unsigned int	      xmp_Flags;
  /* sets the mode used as default */
  struct XpkTypeData *xmp_DefaultType;
  /* Timeout for password requester given in seconds, zero means no timeout */
  unsigned short	      xmp_Timeout;
};

/* The library internal defaults are:
   XPKM_UseXFD			FALSE
   XPKM_AutoPassword		FALSE
   XPKM_UseExternals		TRUE
   XTD_ReturnError		defined as default
   xmp_TimeOut			set to 120	(two minutes)

   These defaults are used, when no preferences file is given.
*/

/*
 *     XpkMasterPrefs Semaphore structure
 *
 *  find with FindSemaphore(XPKPREFSSEMNAME);
 *
 *  obtain with ObtainSemaphoreShared(),
 *  programs WRITING into the structure fields must know:
 *   - only write to them, when you created the semaphore
 *   - use ObtainSemaphore() instead of ObtainSemaphoreShared()
 */

#define XPKPREFSSEMNAME		"« XpkMasterPrefs »"

/* Defines used for xps_PrefsType. These help to find out, which preferences
 * type is used. */

#define XPREFSTYPE_STANDARD	0x58504B4D	/* 'XPKM' */
#define XPREFSTYPE_CYB		0x20435942	/* ' CYB' */

struct XpkPrefsSemaphore {
  /* AMIGA */
  /* struct SignalSemaphore 	xps_Semaphore; */
  unsigned int	       xps_Version;	   /* at the moment 0 */
  unsigned int	       xps_PrefsType;	   /* preferences type */
  unsigned char       *xps_PrefsData;	   /* preferences data */
  struct XpkMainPrefs *xps_MainPrefs;      /* defined defaults */
  unsigned int	       xps_RecogSize;	   /* needed size of Recogbuffer */
  struct XpkTypeData *(*xps_RecogFunc) (); /* Recog function */
  struct Hook         *xps_ProgressHook;   /* hook function */
  struct Task         *xps_MasterTask;	   /* Creater's task */
};

/* Use Signal(sem->xps_MasterTask, SIGBREAKF_CTRL_C); to get the installer
   program to remove the semaphore. */

/* prototype/typedef of RecogFunc:
typedef struct XpkTypeData * __asm (*RecogFunc)
	(register __a0 char * buffer,
	 register __a1 char * filename,
	 register __d0 unsigned int  buffersize,
	 register __d1 unsigned int  fullsize,
	 register __a2 struct TagItem *tags);
*/

#endif /* XPK_XPKPREFS_H */
