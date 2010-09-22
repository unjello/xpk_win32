/* xPK.c -- General XPK file-to-file packer/unpacker
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "../xpkmaster/xpkmaster.h"
#include "../xpkmaster/amigalibs.h"

#define NAME		"xPK"
#define DISTRIBUTION	"(Freeware) "
#define REVISION	"4"

unsigned int chunkfunc(struct XpkProgress *);
char *tempname(char *);
char *basename(char *);
void doarg(char *);
char *dofile(char *);
void end(char *);
int isexecutable(char *);

struct Hook chunkhook = {{0}, (unsigned int (*) ()) chunkfunc};
void *XpkBase=0;
char errbuf[2048], *err=0, namebuf[2048], PrevName[2048], strbuf[2048];
struct stat *fib=0;

char usage[] =
"Usage: XPK [-efrsux] [-m method] [-p password] files\n"
"       -e = extract files (same as -u)\n"
"       -f = force packing of already packed files\n"
"       -m = four letter packing method name\n"
"       -p = encrypt/decrypt using password\n"
"       -r = recursively (un)pack files in dir\n"
"       -s = add suffix and don't delete original\n"
"       -x = pack executables only\n";

unsigned char suffix=0, force=0, unpack=0, recurse=0, depth=0, executables=0;
char *password=0, *method=0;

int main(int argc, char **argv)
{
  char *c;
  int i=1;

  if(!(fib=(struct stat*)malloc(sizeof(struct stat))))
    end("Not enough memory\n");

  if(stricmp(basename(argv[0]), "XPK"))
    method = basename(argv[0]);
  if(argc < 2 || !strcmp (argv[1], "?"))
    end(usage);

  for(; *argv[i] == '-'; i++)
    for(c = argv[i] + 1; *c; c++) {
      switch (*c) {
      case 'p': password = argv[++i]; break;
      case 'm': method = argv[++i]; break;
      case 's': suffix = 1; break;
      case 'f': force = 1; break;
      case 'e':
      case 'u':	unpack = 1; break;
      case 'r': recurse = 1; break;
      case 'x':	executables = 1; break;
      default: end(usage);
      }
      if(i >= argc)
	end(usage);
    }

  if(!method && !unpack)
    end("Need a packing method, use -m\n");

  if(i == argc)
    end(usage);

  for(; i < argc && !err; i++)
    doarg(argv[i]);

  end(err);
}

void iprint(char * s)
{
  int i;
  for(i = depth; i; --i)
    fwrite("  ", 1, 2, stdout);
  fwrite(s, 1, strlen(s), stdout);
}

void doarg(char *name)
{
  unsigned int lock;

  if(*name==-1)
    return;

  if((stat(name, fib))<0) {
    sprintf(err = errbuf, "Error %ld reading %s\n", errno, name);
    return;
  }

  if(!(fib->st_mode&_S_IFDIR)) {
    dofile(name);
  }
 else if(recurse) {
    sprintf(strbuf, "Directory %s\n", name);
    iprint(strbuf);
	std::string dir = name;
	char cwd[_MAX_PATH];

	if( _getcwd( cwd, _MAX_PATH ) == NULL ) {
		sprintf(err = errbuf, "Error %ld reading %s\n", errno, name);
		return;
	}
	_chdir(dir.c_str());

	WIN32_FIND_DATAA data;
	HANDLE h=FindFirstFileA("*.*",&data);
	bool good=h!=INVALID_HANDLE_VALUE?true:false;
	while(good) {
		std::string s(data.cFileName);
		if(s!="."&&s!="..") {
			if((stat(s.c_str(), fib))<0) {
				sprintf(err = errbuf, "Error %ld reading %s\n", errno, name);
				return;
			}
			if(s.find_first_of("xpk")!=std::string::npos||fib->st_mode&_S_IFDIR) {
				++depth;
				doarg(s.c_str());
				--depth;
			}
		}
		good=FindNextFileA(h,&data);
	}
	FindClose(h);
	_chdir(cwd);
  }
}

char *dofile(char *filename)
{
  struct XpkFib xfib = {0};
  char buf[100];
  int len;

  if(!force || unpack) {
	  TagItem __tags[] = {
		  { XPK_InName, (void*)filename },
		  { TAG_DONE, NULL }
	  };
    if(XpkExamine(&xfib, __tags)) {
      sprintf(buf, "Error examining %s\n", filename);
      iprint(buf);
      return 0;
    }
  }

  tempname(filename);

  if(!unpack) {
    if(!force && xfib.xf_Type != XPKTYPE_UNPACKED) {
      sprintf(buf, "Skipping (already packed) %s\n", filename);
      iprint(buf);
      return 0;
    }

    if(executables && !isexecutable(filename))
      return 0;

    if(suffix)
      sprintf(namebuf, "%s.xpk", filename);

	TagItem __tags[] = {
		{ XPK_InName, (void*) filename },
		{	XPK_OutName, (void*) namebuf },
		{	XPK_ChunkHook, (void*) &chunkhook },
		{	XPK_GetError, (void*) errbuf },
		{	XPK_PackMethod, (void*) method },
		{	XPK_Password, (void*) password },
		{	XPK_NoClobber, (void*)1 },
		{	TAG_DONE, NULL }
	};
    if(XpkPack(__tags))
    {
      unsigned int i = strlen(errbuf);
      errbuf[i] = '\n'; errbuf[i+1] = '\0';
      return err = errbuf;
    }
  }
  /* Unpack */
  else {
    if(xfib.xf_Type != XPKTYPE_PACKED) {
      sprintf(buf, "Skipping (already unpacked) %s\n", filename);
      iprint(buf);
      return 0;
    }

    len = strlen(filename);
    suffix = 0;
    if(len > 4 && !stricmp(filename + len - 4, ".xpk")) {
	  // if dot < 4 assume that its in amiga format ext.filename
	  // and do the reversal, else just remove trailing xpk
	  int dot = (int)strchr(filename,'.') - (int)filename;
	  if(dot<4)
		  sprintf(namebuf,"%.*s.%.*s",len-5-dot,filename+dot+1,dot,filename);
	  else {
		  strcpy(namebuf,filename);
		  namebuf[len-4] = 0;
	  }

      suffix = 1;
    }

	TagItem __tags[] = {
		{ XPK_InName, (void*) filename },
		{	XPK_FileName, (void*) filename },
		{	XPK_OutName, (void*) namebuf },
		{	XPK_ChunkHook, (void*) &chunkhook },
		{	XPK_Password, (void*) password },
		{	XPK_GetError, (void*) errbuf },
		{	XPK_NoClobber, 1 },
		{	TAG_DONE, NULL }
	};
    if(XpkUnpack(__tags))
    {
      unsigned int i = strlen(errbuf);
      errbuf[i] = '\n'; errbuf[i+1] = '\0';
      return err = errbuf;
    }
  }

  if(!suffix)
  {
    if(unlink(filename)<0)
      return err = "Cannot delete input file\n";
    if(rename(namebuf, filename)<0)
      return err = "Cannot rename tempfile\n";
  }
}

unsigned int chunkfunc(struct XpkProgress *prog)
{
  char buf[180];

  if(prog->xp_Type != XPKPROG_END)
    sprintf(buf,
	     "%4s: %-8s (%3ld%% done, %2ld%% CF, %6ld cps) %s\r",
	     prog->xp_PackerName, prog->xp_Activity, prog->xp_Done,
	     prog->xp_CF, prog->xp_Speed, prog->xp_FileName);
  else
    sprintf(buf,
	     "%4s: %-8s (%3ldK, %2ld%% CF, %6ld cps) %s",
	     prog->xp_PackerName, prog->xp_Activity, prog->xp_ULen / 1024,
	     prog->xp_CF, prog->xp_Speed, prog->xp_FileName);

  iprint(buf);

  if(prog->xp_Type == XPKPROG_END)
	  printf("      \n");

  return 0;
}

char *tempname(char *name)
{
  sprintf(namebuf, ".tmp%s", basename(name));
  return namebuf;
}

int isexecutable(char *name)
{
  struct stat testfib;

  if((stat(name, &testfib))<0)
    return 0;

  if((testfib.st_mode & _S_IEXEC))
    return 1;

  return 0;
}

void end(char * text)
{
  if(text) fwrite(text, 1, strlen(text), stdout);
  if(fib) free(fib);

  exit(text ? 10 : 0);
}

char *basename(char *path)
{
  char *ptr;

  if(path) {
    while((ptr=(char *)strpbrk(path, "/")))
      path=ptr+1;

    return path;
  } else
    return 0;
}

