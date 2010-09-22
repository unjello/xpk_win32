/* query.c -- Implementation of XpkQuery
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
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include "xpkmaster.h"
#include "texts.h"
#include <windows.h>


/*
 *  Information query
 */
static const struct XpkMode USERMode={ 0,100,0,0,0,50,500,500,0,"user"};
static struct XpkInfo USERInfo={ 1,0,0,1,"USER","User", 0, 0x55534552,
				 XPKIF_PK_CHUNK|XPKIF_UP_CHUNK,
				 DEFAULTCHUNKSIZE, 10, DEFAULTCHUNKSIZE,0,0,
				 0,0,100,0,const_cast<XpkMode*>(&USERMode),{0,0,0,0,0,0}};

bool insert_packer(XpkPackerList* plist, unsigned id) {
	int i;
	for(i=plist->xpl_NumPackers; i > 0 && plist->xpl_Packer[i - 1] > id; i--)
		plist->xpl_Packer[i]=plist->xpl_Packer[i-1];

	plist->xpl_Packer[i] = id;
	return ++plist->xpl_NumPackers == MAXPACKERS ? true:false;
}

int XpkQuery(struct TagItem *tags)
{
  struct TagItem *ti, *ti2=tags;
  /* use prefs, default is true */
  unsigned int packmode=101, packmethod=0, prefs=1;
  int error=0;
  struct XpkPackerInfo *pinfo=0;
  struct XpkPackerList *plist=0;
  struct XpkMode *pmode=0;
  HMODULE XpkSubBase=0;
  struct XpkInfo *sinfo=0;
  typedef struct XpkInfo *(*XpksPackerInfo_t)();
  XpksPackerInfo_t XpksPackerInfo;
  char libname[SUBLIBNAME_SIZE];
  struct stat stbuf;

#ifdef _DEBUG
  DebugTagList("XpkQuery", tags);
#endif

  while((ti=NextTagItem(&ti2))) {
    switch(ti->ti_Tag) {
    case XPK_PackersQuery:
      plist=(struct XpkPackerList *) ti->ti_Data;
      break;
    case XPK_ModeQuery:
      pmode=(struct XpkMode *) ti->ti_Data;
      break;
    case XPK_PackerQuery:
      pinfo=(struct XpkPackerInfo *) ti->ti_Data;
      break;
    case XPK_PackMethod:
      packmethod=idfromname((char *) ti->ti_Data);
      break;
    case XPK_PackMode:
      packmode=(unsigned int)ti->ti_Data;
      break;
    case XPK_Preferences:
      prefs=(unsigned int)ti->ti_Data;
      break;
    }
  }
  
	if(plist) {
		memset(plist, 0, sizeof(struct XpkPackerList));

		WIN32_FIND_DATAA data;
		HANDLE h=FindFirstFileA("*.dll",&data);
		bool good=h!=INVALID_HANDLE_VALUE?true:false;
		while(good) {
			std::string s(data.cFileName);
			if(!s.compare(0,4,"xpk_")) {
				insert_packer(plist,idfromname((char*)s.substr(5,4).c_str()));
			}
			good=FindNextFileA(h,&data);
		}
		FindClose(h);
	}
	  /* add USER mode */
	if(prefs) {
		if(plist->xpl_NumPackers == MAXPACKERS)
			--plist->xpl_NumPackers;
#ifdef WORDS_BIGENDIAN
        plist->xpl_Packer[plist->xpl_NumPackers++]=USER_COOKIE;
#else
        plist->xpl_Packer[plist->xpl_NumPackers++]=_byteswapint32(USER_COOKIE);
#endif
     } else if(packmethod) {
#ifdef WORDS_BIGENDIAN
		if(packmethod == USER_COOKIE) {
#else
		if(packmethod == _byteswapint32(USER_COOKIE)) {
#endif
			sinfo=&USERInfo;
			sinfo->xi_Description=strings[TXT_USER_DESCRIPTION];
		} else {
			sprintf(libname, SUBLIBNAME_STRING, (char*)&packmethod);
			if(!(XpkSubBase=LoadLibrary(libname)))
				error=XPKERR_MISSINGLIB;

			if((XpksPackerInfo=(XpksPackerInfo_t)GetProcAddress(XpkSubBase,"LIBXpksPackerInfo"))) {
				if(!(sinfo=XpksPackerInfo()))
					error=XPKERR_MISSINGLIB;
			} else
				error=XPKERR_MISSINGLIB;
		}
    
		if(!error) {
			if(pinfo) {
				sprintf(pinfo->xpi_Name, "%.23s", sinfo->xi_Name);
				sprintf(pinfo->xpi_LongName, "%.31s", sinfo->xi_LongName);
				sprintf(pinfo->xpi_Description, "%.79s", sinfo->xi_Description);
				pinfo->xpi_Flags=sinfo->xi_Flags;
				pinfo->xpi_MaxChunk=sinfo->xi_MaxPkInChunk;
				pinfo->xpi_DefChunk=sinfo->xi_DefPkInChunk;
				pinfo->xpi_DefMode=sinfo->xi_DefMode;
			} else if(pmode) {
				struct XpkMode* m=sinfo->xi_ModeDesc;
		        if(packmode == 101)
					packmode=sinfo->xi_DefMode;
	
				while(m && m->xm_Upto < packmode)
					m=m->xm_Next;
	
				if(m) {
					memcpy(pmode, m, sizeof(struct XpkMode));
					/* force C-string for these dumb persons who can't count */
					pmode->xm_Description[9]=0;
					pmode->xm_Next=0;
				} else
					error=XPKERR_NOINFO;
			} else
				error=XPKERR_BADPARAMS;
		} 
		
		if(XpkSubBase)
			FreeLibrary(XpkSubBase);
	} else
		error=XPKERR_BADPARAMS;

	return parseerrortags(tags, error);
}
