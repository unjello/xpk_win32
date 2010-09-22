/* xpkSQSH.c -- an LZ based cruncher; special algorithms for 8 bit sound data;
 * fast decrunching
 * Copyright (C) 1994 John Hendrikx
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

/* Written by John Hendrikx <?>
 * Converted by Bert Jahn <wepl@kagi.com>
 * XPK library version by Vesa Halttunen <vesuri@jormas.com>
 * Win32 version by Andrzej Lichnerowicz <angelo@irc.pl>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../xpkmaster/xpksub.h"

int bfextu(unsigned char *, int, int);
int bfextu3(unsigned char *, int);
int bfexts(unsigned char *, int, int);
void unsqsh(unsigned char *, unsigned char *);

struct XpkMode SqshMode =
{
  NULL,				/* next			*/
  100,				/* upto			*/
  XPKMF_A3000SPEED,		/* flags		*/
  0,				/* packmem		*/
  0,				/* unpackmem		*/
  140,				/* packspeed, K / sec	*/
  1043,				/* unpackspeed, K / sec	*/
  45,				/* ratio, *0.1 %	*/
  0,				/* reserved		*/
  "normal"			/* description		*/
};

static struct XpkInfo SqshInfo =
{
  1,				/* info version */
  1,				/* lib  version */
  0,				/* master vers  */
  0,				/* pad          */
  "SQSH",			/* short name   */
  "Squash",			/* long name    */
  "LZ based cruncher; special algorithms for 8 bit sound data; fast decrunching",	/* description*/
  0x524C454E,			/* 4 letter ID  */
  XPKIF_PK_CHUNK |		/* flags        */
  XPKIF_UP_CHUNK,
  0x7fffffff,			/* max in chunk */
  0,				/* min in chunk */
  0x4004,			/* def in chunk */
  "Suqashing",			/* pk message   */
  "UnSquashing",		/* up message   */
  "Squashed",			/* pk past msg  */
  "UnSquashed",			/* up past msg  */
  50,				/* def mode     */
  0,				/* pad          */
  &SqshMode			/* modes        */
};

/*
 * Returns an info structure about our packer
 */
extern "C" __declspec(dllexport) struct XpkInfo* LIBXpksPackerInfo(void)
{
  return &SqshInfo;
}

/*
 * Pack a chunk
 */
extern "C" __declspec(dllexport) int LIBXpksPackChunk(struct XpkSubParams *xpar)
{
  return XPKERR_NOFUNC;
}

extern "C" __declspec(dllexport) int LIBXpksUnpackChunk(struct XpkSubParams *xpar) {
  unsigned char *src=xpar->xsp_InBuf, *dst=xpar->xsp_OutBuf;
  
  unsqsh(src,dst);
  
  return 0;
}

int bfextu(unsigned char *p,int bo,int bc) {
  int r;

  p += bo / 8;
  r = *(p++);
  r <<= 8;
  r |= *(p++);
  r <<= 8;
  r |= *p;
  r <<= bo % 8;
  r &= 0xffffff;
  r >>= 24 - bc;

  return r;
}

#define bfextu1 ((*(src + d0 / 8) >> (7 - (d0 % 8))) & 1)

int bfextu3(unsigned char *p,int bo) {
  int r;

  p += bo / 8;
  r = *(p++);
  r <<= 8;
  r |= *p;
  r >>= 13 - (bo % 8);
  r &= 7;

  return r;
}

int bfexts(unsigned char *p,int bo,int bc) {
  int r;

  p += bo / 8;
  r = *(p++);
  r <<= 8;
  r |= *(p++);
  r <<= 8;
  r |= *p;
  r <<= (bo % 8) + 8;
  r >>= 32 - bc;

  return r;
}

void unsqsh(unsigned char *src, unsigned char *dst) {
  int d0,d1,d2,d3,d4,d5,d6,a2,a5;
  unsigned char *a4,*a6;
  unsigned char a3[] = { 2,3,4,5,6,7,8,0,3,2,4,5,6,7,8,0,4,3,5,2,6,7,8,0,5,4,
	6,2,3,7,8,0,6,5,7,2,3,4,8,0,7,6,8,2,3,4,5,0,8,7,6,2,3,4,5,0 };

#ifdef debug
	fprintf(stderr,"=%02x%02x dst=%x",*src,*(src+1),dst);
#endif

	a6 = dst;
	a6 += *src++ << 8;
	a6 += *src++;
	d0 = d1 = d2 = d3 = a2 = 0;
 
	d3 = *(src++);
	*(dst++) = d3;

l6c6:	if (d1 >= 8) goto l6dc;
	if (bfextu1) goto l75a;
	d0 ++;
	d5 = 0;
	d6 = 8;
	goto l734;

l6dc:	if (bfextu1) goto l726;
	d0 ++;
	if (! bfextu1) goto l75a;
	d0 ++;
	if (bfextu1) goto l6f6;
	d6 = 2;
	goto l708;

l6f6:	d0 ++;
	if (! bfextu1) goto l706;
	d6 = bfextu3(src,d0);
	d0 += 3;
	goto l70a;

l706:	d6 = 3;
l708:	d0 ++;
l70a:	d6 = *(a3 + (8*a2) + d6 - 17);
	if (d6 != 8) goto l730;
l718:	if (d2 < 20) goto l722;
	d5 = 1;
	goto l732;

l722:	d5 = 0;
	goto l734;

l726:	d0 += 1;
	d6 = 8;
	if (d6 == a2) goto l718;
	d6 = a2;
l730:	d5 = 4;
l732:	d2 += 8;
l734:	d4 = bfexts(src,d0,d6);
	d0 += d6;
	d3 -= d4;
	*dst++ = d3;
	d5--;
	if (d5 != -1) goto l734;
	if (d1 == 31) goto l74a;
	d1 += 1;
l74a:	a2 = d6;
l74c:	d6 = d2;
	d6 >>= 3;
	d2 -= d6;
	if (dst < a6) goto l6c6;

#ifdef debug
	fprintf(stderr," dst=%p a6=%p d0=8*%x+%d->%x\n",dst,a6,d0/8,d0%8,(d0+7)/8+3);
/*	fprintf(stderr,"d1=%x d2=%x d3=%x d4=%x d5=%x d6=%x a2=%x a4=%x a5=%x\n",d1,d2,d3,d4,d5,d6,a2,a4,a5); */
#endif

	return;

l75a:	d0++;
	if (bfextu1) goto l766;
	d4 = 2;
	goto l79e;

l766:	d0++;
	if (bfextu1) goto l772;
	d4 = 4;
	goto l79e;

l772:	d0++;
	if (bfextu1) goto l77e;
	d4 = 6;
	goto l79e;

l77e:	d0++;
	if (bfextu1) goto l792;
	d0++;
	d6 = bfextu3(src,d0);
	d0 += 3;
	d6 += 8;
	goto l7a8;

l792:	d0++;
	d6 = bfextu(src,d0,5);
	d0 += 5;
	d4 = 16;
	goto l7a6;

l79e:	d0++;
	d6 = bfextu1;
	d0 ++;
l7a6:	d6 += d4;
l7a8:	if (bfextu1) goto l7c4;
	d0 ++;
	if (bfextu1) goto l7bc;
	d5 = 8;
	a5 = 0;
	goto l7ca;

l7bc:	d5 = 14;
	a5 = -0x1100;
	goto l7ca;

l7c4:	d5 = 12;
	a5 = -0x100;
l7ca:	d0++;
	d4 = bfextu(src,d0,d5);
	d0 += d5;
	d6 -= 3;
	if (d6 < 0) goto l7e0;
	if (d6 == 0) goto l7da;
	d1 -= 1;
l7da:	d1 -= 1;
	if (d1 >= 0) goto l7e0;
	d1 = 0;
l7e0:	d6 += 2;
	a4 = -1 + dst + a5 - d4;
l7ex:	*dst++ = *a4++;
	d6--;
	if (d6 != -1) goto l7ex;
	d3 = *(--a4);
	goto l74c;
}

#ifdef voi_muna

#define MatchLength 48
#define LzssCode    -4
#define FoundByt    -6
#define Hash        -8
#define OverflowLimit	-12
#define Here	-14

#define Deem0	8
#define Deem1	12
#define Deem2	14
#define Deem0b	Deem0+2
#define Deem1b	Deem1+1
#define Deem2b	Deem2+2
#define Deel0	1<<Deem0
#define Deel1	1<<Deem1
#define Deel2	1<<Deem2
#define Deel01	Deel0+Deel1
#define Deel012	Deel0+Deel1+Deel2
#define BlockLength	5

sqsh()
{
  a3=a0;                               /* Move.l	a0,a3 */
  a3[56]=a6;                           /* Move.l	a6,56(a3) */

  /*	Move.l	4.w,a6
 	Move.l	#136*1024+128,d0
	Moveq	#0,d1
	Jsr	-198(a6)	Exec - AllocMem
	Tst.l	d0
	Beq.s	NoMem
	Move.l	d0,52(a3)
  */
  if(!(d0=calloc(136*1024+128, 1)))
    goto SQSH_NoMem;
  a3[52]=d0;

  a0=a3[0];                          /*	Move.l	(a3),a0 */
  a6=d0;                             /*	Move.l	d0,a6 */
  a6+=8192+128;                      /*	Lea	8192+128(a6),a6 */
  a4=a3[8];                          /*	Move.l	8(a3),a4 */
  d7=0;                              /*	Moveq	#0,d7 */
  d7=a3[6];                          /*	Move.w	6(a3),d7 */
  a4[0]=d7;                          /*	Move.w	d7,(a4) */

  sp=a3;                             /*	Move.l	a3,-(sp) */
  SQSH_PackThisShit();               /*	Bsr.s	PackThisShit */
  a3=sp;                             /* Move.l	(sp)+,a3 */

  /*	Addq.l	#7,d6
  	Bne.s	Over */
  if((d6=d6+7))
    goto SQSH_Over;

  d7=-17;                            /*	Moveq	#-17,d7 */
  a3[16]=0;                          /*	Clr.l	16(a3) */
  goto SQSH_FreeMem2;                /*	Bra.s	FreeMem2 */

 SQSH_Over:
  d6=d6>>3;                          /*	Lsr.l	#3,d6 */
  a3[16]=d6;                         /*	Move.l	d6,16(a3)	OutLen */
  d7=0;                              /*	Moveq	#0,d7 */

 SQSH_FreeMem2:
  /*	Move.l	4.w,a6
	Move.l	#136*1024+128,d0
	Move.l	52(a3),a1
	Jsr	-210(a6)	Exec - FreeMem */
  free(a3[52]);
  
  a6=a3[56];                         /*	Move.l	56(a3),a6 */
  d0=d7;                             /*	Move.l	d7,d0 */
  goto SQSH_Rts;                     /*	Rts */
  
 SQSH_NoMem:
  a6=a3[56];                         /*	Move.l	56(a3),a6 */
  d0=-7;                             /*	Moveq	#-7,d0 */
  goto SQSH_Rts;                     /*	Rts		Back to mapping code */

  /* a0 should be Source */
  /* a4 should be Dest */
  /* a6 should be ptr to Tree Mem (136*1024+128 bytes) + 8192+128 */
 SQSH_PackThisShit:
  /*	Movem.l	a0/a4/d7,-(sp) not needed because regs used for clearing... */
  a2=a6;                             /*	Move.l	a6,a2 */
  a2+=128*1024;                      /*	Add.l	#128*1024,a2 */

  /*	Moveq	#0,d0	AlgMode
	Moveq	#0,d1
	Moveq	#0,d2
	Moveq	#0,d3
	Moveq	#0,d4
	Moveq	#0,d5
	Moveq	#0,d6	Bit
	Move.l	d0,a0
	Move.l	d0,a1	N
	Move.l	d0,a3	AlgDelta
	Move.l	d0,a4
	Move.l	d0,a5 */
  d0=d1=d2=d3=d4=d5=d7=a1=a3=a5=0;
  for(d6=0; d6<4*48*726; d6++) {
    a2--;
    a2[0]=0;
  }

  /* Move.w	#725,d7
     ClrLoop:
     REPT	4
     Movem.l	d0-d6/a0-a1/a3-a5,-(a2)	12 regs = 48 bytes
     ENDR
     Dbra	d7,ClrLoop */

  a2=1;                                      /*	Move.w	#1,a2 */
  /*	Movem.l	(sp)+,a0/a4/d7 see above */

  a6[Here]=d7;                               /*	Move.w	d7,Here(a6) */
  d7=d7<<3;                                  /*	Lsl.l	#3,d7 */
  a6[OverFlowLimit]=d7;                      /*	Move.l	d7,OverFlowLimit(a6) */

  a4[2]=a0[0];                               /*	Move.b	(a0),2(a4) */
  d6=24;                                     /*	Moveq	#24,d6 */

  /*	;a0 = source%
	;a4 = dest% */

  /* Global regs in loop:
   * ====================
   *  d0 = AlgDelta, VorigeKeer
   *  d6 = BitOffset
   *  a0 = Source%
   *  a1 = N
   *  a3 = -
   ** a4 = Dest
   ** a5 = AlgMode
   ** a6 = ptr to additional regs storage
   *
   *	Register usage:
   *-------------------------------------------------------------
   *	d0	 9	a0	 5
   *	d1	15	a1	10
   *	d2	24	a2	10
   *	d3	35	a3	17
   *	d4	28	a4	 5
   *	d5	19	a5	10
   *	d6	 7	a6	15
   *	d7	32
   */

 SQSH_BigLoop: /*	;a2 = Length */
  d4=a2;                             /*	Move.w	a2,d4 */
  d4--;                              /*	Subq.w	#1,d4	d4 = # of loops */
  a2+=a0+a1+1;                       /*	Lea	1(a0,a1.w),a2 */

  d5=a6[Hash];                       /*	Move.w	Hash(a6),d5 */

  /*	Movem.l	a4-a6,-(sp) IMPLEMENT! */
  /*	;Move.l	Tree(a6),a6 */
  a5=a6-(8192+128);                  /*	Lea	-(8192+128)(a6),a5 */

SQSH_AddLoop:
  a1++;                              /*	Addq.w	#1,a1 */
  d3=0;                              /*	Moveq	#0,d3 */
  d3=a2[0];                          /*	Move.b	(a2),d3	*/

  d7=a2[1];                          /*	Move.b	1(a2),d7 */
  /* Prevents adding runs of the same bytes to the tree to speed up packing. */
  d3=d3<<4;                          /*	Lsl.w	#4,d3 */
  d3=d3^d7;                          /*	Eor.b	d7,d3 */

  /*	Tst.w	d4
   *	Beq.s	.Always	LastLoop... always!! */
  if(!d4)
    goto Always;
  /*	Cmp.w	d3,d5	 
   *	Beq.s	SkipToIt */
  if(d3==d5)
    goto SkipToIt;

  /* d1,d7 = Scratch
   * d0 = AlgDelta, VorigeKeer
   * d2 = Off%
   * d3 = NewHash
   * d4 = LoopCounter
   * d5 = OldHash
   * d6 = BitOffset
   * a0 = Source%
   * a1 = N
   * a2 = CurrentPos in file
   * a3 = Ptr to Hash table
   * a4 = -
   * a5 = -
   * a6 = Tree */

 Always:
  /* d3 = hash (0-4095) */
  d7=MatchLength-2;  /*	Moveq	#MatchLength-2,d7 */
  d2=a5[d3*2];       /*	Move.w	(a5,d3.w*2),d2	off%=DPEEK(tree%+hash&*2) */
  if(d2)             /*	Bne.s	ElseB */
    goto SQSH_ElseB;
  a5[d3*2]=a1;       /*	Move.w	a1,(a5,d3.w*2)	DPOKE tree%+hash&*2,free */
  goto SQSH_EndB;    /*	Bra.s	EndB */

SQSH_ElseB:
  d1=a1;             /*	Move.l	a1,d1 */
  d1-=Deel012;       /*	Sub.w	#Deel012,d1 */
  /*	a0 = source%	;a3 = cmpa% (lookback buffer)
	a6 = tree%	;a4 = cmpb% (current pos)
	a1 = n (0 - 32000)	;d1 = foundadr%
	d2 = off%	;d3 = hash&
	d5 = loop counter	;d7 = foundbyt|  (0 = nothing found) */

 Loop2:
  a3=a0+d2;          /*	Lea	(a0,d2.w),a3	a3 = cmpa% (lookbackadr) */
  a4=a2;             /*	Move.l	a2,a4 */

  d5=MatchLength-2;  /*	Moveq	#MatchLength-2,d5 */
 Loop:
	Cmpm.b	(a3)+,(a4)+
	Dbne	d5,.Loop
	IFNE	P68000
	Lea	(a6,d2.l),a3
	Add.l	d2,a3
	Add.l	d2,a3
	Add.l	d2,a3
	ELSE
	Lea	(a6,d2.l*4),a3
	ENDC
	Ble.s	.Else4b

	Addq.l	#2,a3
.Else4b	Cmp.b	d5,d7
	Blt.s	.SkipIf2
	Cmp.w	d1,d2	;a5 = N - Deel012
	Ble.s	.SkipIf2
	Move.w	d2,d1
	Move.w	d5,d7

.SkipIf2	Move.w	(a3),d2	Just as fast as a Tst
	Bne.s	.Loop2
	Move.w	a1,(a3)
	*

EndB	Move.w	d3,d5
SkipToIt	Addq.l	#1,a2
	Dbra	d4,AddLoop

	Movem.l	(sp)+,a4-a6
	Move.w	d3,Hash(a6)

	Neg.w	d7
	Add.w	#MatchLength-2,d7
	Move.w	d7,FoundByt(a6)

	Cmp.b	#1,d7
	Ble.s	SkipIf
	;LZSS TEST
	;d3 = l = n% - foundadr% - 1
	Move.w	a1,d3
	Sub.w	d1,d3
	Subq.w	#1,d3

IF	Moveq	#0,d4	@
	IFNE	P68000
	Move.l	a0,-(sp)
	Lea	Quick2Tab(pc),a0
	Add.w	d7,a0
	Move.w	(a0,d7.w),d4
	Move.l	(sp)+,a0
	ELSE
	Move.w	(Quick2Tab,pc,d7.w*2),d4	@
	ENDC
	Swap	d4	@

	Cmp.w	#Deel0,d3
	Bhs.s	.ElseIf2
	Moveq	#Deem0b,d2		0-7-9
	Lsr.l	#6,d4	@
	Bra.s	IF2
.ElseIf2	Cmp.w	#Deel01,d3
	Bhs.s	.ElseIf4
	Moveq	#Deem1b,d2		0-11-12
	Lsr.l	#3,d4	@
	Bset	#12,d4	@
	Sub.w	#Deel0,d3
	Bra.s	IF2
.ElseIf4	Moveq	#Deem2b,d2		0-13-15
	Bset	#14,d4	@
	Sub.w	#Deel01,d3

IF2	;d2 = bits
	Or.w	d3,d4	@
	;d4 = LZSS code	@

	IFNE	P68000
	Move.l	a0,-(sp)
	Lea	QuickTab(pc),a0
	Add.b	(a0,d7.w),d2
	Move.l	(sp)+,a0
	ELSE
	Add.b	(QuickTab,pc,d7.w),d2	Bits + 1
	ENDC
	Cmp.w	#7,a5	@
	Bls.s	.Skip	@
	Subq.w	#1,d2	@
	Bclr	d2,d4	@
	Addq.w	#2,d2	@
.Skip	Move.l	d4,LzssCode(a6)	@
	Move.l	d2,d4
	Lsl.w	#2,d4
	Divu	d7,d4	d7: 2-50   d4: 4-26


SkipIf	;DELTA TEST
	Cmp.w	#7,a5	Algmode
	Bhi.s	Else

	;Delta-packing is off:
	Moveq	#1,d1	Lengte
	Moveq	#8,d3	DeltaCode
	Lea	SpecialCase(pc),a3
	Moveq	#9*4,d7
	Bra.s	SkipDiv

Else	;Start of delta-calc-loop
	Lea	-1(a0,a1.w),a2
	Move.b	(a2)+,d5
	Moveq	#0,d1	min
	Moveq	#Blocklength-1,d7
.Loop	Move.b	(a2)+,d3
	Sub.b	d3,d5	d3 = d
	Tst.b	d5	laten staan
	Bge.s	.Pos
	Neg.b	d5
	Subq.b	#1,d5
.Pos	Or.b	d5,d1
	Move.b	d3,d5
	Dbra	d7,.Loop

	;d1 = min
IF3	Cmp.b	#1,d1
	Ble.s	.Else

	IFNE	P68000
	Moveq	#7,d7
.LoopA	Btst	d7,d1
	Bne.s	.ExitA
	Dbra	d7,.LoopA
.ExitA	;7-0 = 24-31
	Neg.w	d7
	;-7-0 = 24-31
	Add.w	#31,d7
	ELSE
	Bfffo	d1{24:32},d7	\
	ENDC
	Moveq	#33,d3	 > d3 = INT(LOG(d1)/LOG(2))+2
	Sub.w	d7,d3	DeltaCode

	Cmp.b	#8,d3
	Bne.s	.Endif
	Moveq	#2,d1	Length
	Bra.s	.Endif2

.Else	Moveq	#2,d3	Delta
.Endif	Moveq	#BlockLength,d1	Length
.Endif2	Move.w	d1,d7
	Mulu	d3,d7


IF4	Cmp.b	#7,d3
	Blt.s	IF5
	Cmp.l	#20*65536,d0
	Bge.s	IF5
	;Delta-packing is on:
	Moveq	#8,d7	Bits3
	Moveq	#1,d1	Length
	Moveq	#8,d3	DeltaCode

IF5	Move.w	d0,d5
	Lsl.w	#3,d5	2*8 teveel
	Add.w	d3,d5	2*8 + 2 teveel
	IFNE	P68000
	Lea	DeltaTable-36(pc),a3
	Add.w	d5,a3
	Add.w	d5,a3
	ELSE
	Lea	DeltaTable-36(pc,d5.w*2),a3
	ENDC
	Add.b	(a3),d7


	;DELTA TESTING DONE...
	Lsl.w	#2,d7
	Divu	d1,d7	d1: 1,2,5     d7: 9-41

SkipDiv	;d7 = deltapercentage
	;d4 = lzsspercentage
	Cmp.w	d4,d7
	Ble.s	NoIF
	;LZSS is it!
	Move.w	FoundByt(a6),d7
	Move.w	d7,a2

	Subq.w	#1,d7	Handy exchange for a compare
	Ble.s	NoIF
	Subq.w	#2,d7
	Blt.s	.Endif
	Beq.s	.SkipOne
	Subq.w	#1,a5	Algmode
.SkipOne	Subq.w	#1,a5	Algmode
	IFNE	P68000
	Cmp.w	#0,a5
	ELSE
	Tst.w	a5
	ENDC
	Bge.s	.Endif
	Sub.w	a5,a5
.Endif	;Usable: d3-d4
	Move.l	LzssCode(a6),d3	@


	IFNE	P68000

	Movem.l	a4/d1-d2/d6,-(sp)
	Move.l	d6,d1
	Lsr.l	#3,d6
	Add.l	d6,a4
	And.w	#7,d1
	Moveq	#7,d6
	Sub.w	d1,d6
	Subq.w	#1,d2
.Loop2	Move.b	(a4),d1
.Loop	Btst	d2,d3
	Bne.s	.Set
	Bclr	d6,d1
	Bra.s	.Skip
.Set	Bset	d6,d1
.Skip	Subq.w	#1,d6
	Bge.s	.Ok
	Move.b	d1,(a4)+
	Moveq	#7,d6
	Dbra	d2,.Loop2
	Bra.s	.Exit
.Ok	Dbra	d2,.Loop
	Move.b	d1,(a4)+
.Exit	Movem.l	(sp)+,a4/d1-d2/d6

	ELSE
	Bfins	d3,(a4){d6:d2}	@
	ENDC


	Add.l	d2,d6	Bits + bits2 + 1 + HuffCor
	Swap	d0
	IFNE	P68000
	Bra	EndifFinal
	ELSE
	Bra.s	EndifFinal
	ENDC
NoIF	;DELTA is it!
	;d0.w = VorigeKeer	d1.w = Length
	;d2   = -		d3   = DeltaMode (2-8)
	;d4   = -		d5   = -
	;d6   = BitOffset	d7   = -
	;a0   = Source%	a1.l = N
	;a2.l = -		a3   = PackInfo
	;a4   = Dest%	a5   = Algmode

	Move.w	d3,d0	Delta,Vorigekeer
	Swap	d0

	Moveq	#0,d2
	Move.b	(a3)+,d2	Length
	Move.b	(a3)+,d4	Code


	IFNE	P68000

	Movem.l	a4/d1-d2/d6,-(sp)
	Move.l	d6,d1
	Lsr.l	#3,d6
	Add.l	d6,a4
	And.w	#7,d1
	Moveq	#7,d6
	Sub.w	d1,d6
	Subq.w	#1,d2
.ALoop2	Move.b	(a4),d1
.ALoop	Btst	d2,d4
	Bne.s	.Set
	Bclr	d6,d1
	Bra.s	.Skip
.Set	Bset	d6,d1
.Skip	Subq.w	#1,d6
	Bge.s	.Ok
	Move.b	d1,(a4)+
	Moveq	#7,d6
	Dbra	d2,.ALoop2
	Bra.s	.Exit
.Ok	Dbra	d2,.ALoop
	Move.b	d1,(a4)+
.Exit	Movem.l	(sp)+,a4/d1-d2/d6

	ELSE
	Bfins	d4,(a4){d6:d2}
	ENDC

	Add.l	d2,d6

	;Usable: d2/d4/d7/a3
	Lea	-1(a0,a1.w),a3
	Move.b	(a3)+,d2
	Move.w	d1,a2
	Subq.w	#1,d1
	Beq.s	.Loop
	Addq.w	#8,d0	Algdelta

.Loop	Move.b	(a3)+,d4
	Sub.b	d4,d2

	IFNE	P68000
	Movem.l	a4/d1/d3/d6,-(sp)
	Move.l	d6,d1
	Lsr.l	#3,d6
	Add.l	d6,a4
	And.w	#7,d1
	Moveq	#7,d6
	Sub.w	d1,d6
	Subq.w	#1,d3
.BLoop2	Move.b	(a4),d1
.BLoop	Btst	d3,d2
	Bne.s	.Set2
	Bclr	d6,d1
	Bra.s	.Skip2
.Set2	Bset	d6,d1
.Skip2	Subq.w	#1,d6
	Bge.s	.Ok2
	Move.b	d1,(a4)+
	Moveq	#7,d6
	Dbra	d3,.BLoop2
	Bra.s	.Exit2
.Ok2	Dbra	d3,.BLoop
	Move.b	d1,(a4)+
.Exit2	Movem.l	(sp)+,a4/d1/d3/d6
	ELSE
	Bfins	d2,(a4){d6:d3}
	ENDC


	Add.l	d3,d6
	Move.b	d4,d2
	Dbra	d1,.Loop

	Cmp.w	#31,a5
	Beq.s	EndifFinal
	Addq.w	#1,a5	Algmode
EndifFinal	;a0 = source%
	;d7 = lengte

	Move.w	d0,d4
	Lsr.w	#3,d4
	Sub.w	d4,d0
	Swap	d0

	Cmp.l	OverFlowlimit(a6),d6
	Bge.s	.OverFlow

	Move.w	a1,d7
	Add.w	a2,d7
	Cmp.w	Here(a6),d7
	Blo	BigLoop
	Rts
.OverFlow	Moveq	#-7,d6
	Rts

DeltaTable	Dc.b	1,01,3,02,4,06,6,28,6,29,6,30,6,31,0,0	2,2-8   DeltaTable
	Dc.b	3,02,1,01,4,06,6,28,6,29,6,30,6,31,0,0	3,2-8   :
	Dc.b	6,28,3,02,1,01,4,06,6,29,6,30,6,31,0,0	4,2-8   :
	Dc.b	6,28,6,29,3,02,1,01,4,06,6,30,6,31,0,0	5,2-8   :
	Dc.b	6,28,6,29,6,30,3,02,1,01,4,06,6,31,0,0	6,2-8   :
	Dc.b	6,28,6,29,6,30,6,31,3,02,1,01,4,06,0,0	7,2-8   :
	Dc.b	6,28,6,29,6,30,6,31,4,06,3,02,1,01,0,0	8,2-8   :

Quick2Tab	Dc.w	0,0,%100,%101,%1100,%1101,%11100,%11101
	Dc.w	%11110000,$f1,$f2,$f3,$f4,$f5,$f6,%11110111
	Dc.w	%1111100000,$3e1,$3e2,$3e3,$3e4,$3e5,$3e6,$3e7
	Dc.w	%1111101000,$3e9,$3ea,$3eb,$3ec,$3ed,$3ee,$3ef
	Dc.w	%1111110000,$3f1,$3f2,$3f3,$3f4,$3f5,$3f6,$3f7
	Dc.w	%1111111000,$3f9,$3fa,$3fb,$3fc,$3fd,$3fe,$3ff
SpecialCase	Dc.b	1,0
;RegsStore	Dc.l	0	LzssCode
;	Dc.w	0	FoundByt
;	Dc.l	0	Tree
;	Dc.w	0	Hash
;	Dc.l	0

QuickTab	;Dc.b	3,3,3,3,4,4,7,7	 8x
	;Dc.b	7,7,7,7,7,7	 6x
	;Dcb.b	32,9	32x

	Dc.b	3,3,3,3,4,4,5,5	 8x     QuickTab
	Dc.b	8,8,8,8,8,8,8,8	 8x     : 
	Dcb.b	32,10	32x     :

 SQSH_Rts:
}
#endif
