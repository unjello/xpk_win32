/* byteswap.c -- the endianess functions
 * Copyright (C) 1999-2000 Vesa Halttunen
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

#include <sys/types.h>

unsigned short _byteswapint16(unsigned short x)
{
  return ((x << 8) | (x >> 8));
}

unsigned int _byteswapint32(unsigned int x)
{
  unsigned char b1, b2, b3, b4;
  
  b1 = x>>24;
  b2 = (x>>16) & 0xff;
  b3 = (x>>8) & 0xff;
  b4 = x & 0xff;
  
  return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}
