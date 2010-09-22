/* xpk/xpksub.h -- xpk sublibrary headers
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

#ifndef _XPK_XPKSUB_H_
#define _XPK_XPKSUB_H_

#ifndef _XPK_XPK_H_
#include "xpk.h"
#endif

/*
 *                     The XpkInfo structure
 */

/* Sublibs return this structure to xpkmaster when asked nicely
 * This is version 1 of XpkInfo. It's not #define'd because we don't want
 * it changing automatically with recompiles - you've got to actually update
 * your code when it changes. */
struct XpkInfo {
  unsigned short     xi_XpkInfoVersion; /* Version number of this structure     */
  unsigned short     xi_LibVersion;     /* The version of this sublibrary       */
  unsigned short     xi_MasterVersion;  /* The required master lib version      */
  unsigned short     xi_ModesVersion;   /* Version number of mode descriptors   */
  char         *xi_Name;           /* Brief name of the packer, 20 char max*/
  char         *xi_LongName;       /* Full name of the packer   30 char max*/
  char         *xi_Description;    /* Short packer desc., 70 char max      */
  unsigned int     xi_ID;             /* ID the packer goes by (XPK format)   */
  unsigned int  xi_Flags;          /* Defined below                        */
  unsigned int  xi_MaxPkInChunk;   /* Max input chunk size for packing     */
  unsigned int  xi_MinPkInChunk;   /* Min input chunk size for packing     */
  unsigned int  xi_DefPkInChunk;   /* Default packing chunk size           */
  char         *xi_PackMsg;        /* Packing message, present tense       */
  char         *xi_UnpackMsg;      /* Unpacking message, present tense     */
  char         *xi_PackedMsg;      /* Packing message, past tense          */
  char         *xi_UnpackedMsg;    /* Unpacking message, past tense        */
  unsigned short     xi_DefMode;        /* Default mode number                  */
  unsigned short     xi_Pad;            /* for future use                       */
  struct XpkMode *xi_ModeDesc;     /* List of individual descriptors       */
  unsigned int     xi_Reserved[6];    /* Future expansion - set to zero       */
};

/* defines for Flags: see xpk.h, XPKIF_xxxxx */

/*
 *                     The XpkSubParams structure
 */
struct XpkSubParams {
  char *xsp_InBuf;      /* The input data               */
  unsigned int   xsp_InLen;      /* The number of bytes to pack  */
  char *xsp_OutBuf;     /* The output buffer            */
  unsigned int   xsp_OutBufLen;  /* The length of the output buf */
  unsigned int   xsp_OutLen;     /* Number of bytes written      */
  unsigned int   xsp_Flags;      /* Flags for master/sub comm.   */
  unsigned int   xsp_Number;     /* The number of this chunk     */
  unsigned int   xsp_Mode;       /* The packing mode to use      */
  char          *xsp_Password;   /* The password to use          */
  unsigned short      xsp_LibVersion; /* SublibVersion used to pack   */
  unsigned short      xsp_Pad;        /* Reserved; don't use          */
  unsigned int      xsp_Arg[3];     /* Reserved; don't use          */
  unsigned int      xsp_Sub[4];     /* Sublib private data          */
};

/*
 * xsp_LibVersion is the version number of the sublibrary used to pack
 * this chunk. It can be used to create backwards compatible sublibraries
 * with a totally different fileformat.
 */
#define XSF_STEPDOWN   1  /* May reduce pack eff. to save mem   */
#define XSF_PREVCHUNK  2  /* Previous chunk available on unpack */

#endif /* _XPK_XPKSUB_H_ */
