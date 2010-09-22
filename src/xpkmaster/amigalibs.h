/* xpk/amigalibs.h -- the Amiga compatibility header file
 * Copyright (C) 1999-2000 Vesa Halttunen
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

#ifndef _AMIGALIBS_H_
#define _AMIGALIBS_H_

#include <sys/types.h>

typedef unsigned int Tag;

struct TagItem
{
  /* identifies the type of data */
  Tag ti_Tag;
  /* type-specific data	*/
  void *ti_Data;
};

/* constants for Tag.ti_Tag, control tag values */
/* terminates array of TagItems. ti_Data unused */
#define TAG_DONE   (0L)
/* synonym for TAG_DONE			  */
#define TAG_END	   (0L)
/* ignore this item, not end of array		  */
#define	TAG_IGNORE (1L)
/* ti_Data is pointer to another array of TagItems
 * note that this tag terminates the current array */
#define	TAG_MORE   (2L)
/* skip this and the next ti_Data items	 */
#define	TAG_SKIP   (3L)

/* differentiates user tags from control tags */
#define TAG_USER   ((unsigned int)(1L<<31))

/* If the TAG_USER bit is set in a tag number, it tells utility.library that
 * the tag is not a control tag (like TAG_DONE, TAG_IGNORE, TAG_MORE) and is
 * instead an application tag. "USER" means a client of utility.library in
 * general, including system code like Intuition or ASL, it has nothing to do
 * with user code. */

/* Tag filter logic specifiers for use with FilterTagItems() */
/* exclude everything but filter hits	*/
#define TAGFILTER_AND 0
/* exclude only filter hits		*/
#define TAGFILTER_NOT 1

/* Mapping types for use with MapTags() */
/* remove tags that aren't in mapList */
#define MAP_REMOVE_NOT_FOUND 0
/* keep tags that aren't in mapList   */
#define MAP_KEEP_NOT_FOUND   1

/* minimal node -- no type checking possible */
struct MinNode {
  struct MinNode *mln_Succ;
  struct MinNode *mln_Pred;
};

struct Hook
{
  struct MinNode h_MinNode;
  unsigned int (*h_Entry)();        /* assembler entry point */
  unsigned int (*h_SubEntry)();     /* often HLL entry point */
  unsigned char  *h_Data;              /* owner specific        */
};

#endif /* TAGITEM_H */
