/*
 * kana.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: kana.c,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */
/*
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <import.h>
#include <decode.h>
#include <begin.h>
#include <kana.h>

private ic_t X0201toX0208[ 61 ] = {
  0x2123,	/* 0x21 */
  0x2156,
  0x2157,
  0x2122,
  0x2126,
  0x2572,
  0x2521,
  0x2523,
  0x2525,
  0x2527,
  0x2529,
  0x2563,
  0x2565,
  0x2567,
  0x2543,
  0x213c,	/* 0x30 */
  0x2522,
  0x2524,
  0x2526,
  0x2528,
  0x252a,
  0x252b,
  0x252d,
  0x252f,
  0x2531,
  0x2533,
  0x2535,
  0x2537,
  0x2539,
  0x253b,
  0x253d,
  0x253f,	/* 0x40 */
  0x2541,
  0x2544,
  0x2546,
  0x2548,
  0x254a,
  0x254b,
  0x254c,
  0x254d,
  0x254e,
  0x254f,
  0x2552,
  0x2555,
  0x2558,
  0x255b,
  0x255e,
  0x255f,	/* 0x50 */
  0x2560,
  0x2561,
  0x2562,
  0x2564,
  0x2566,
  0x2568,
  0x2569,
  0x256a,
  0x256b,
  0x256c,
  0x256d,
  0x256f,
  0x2573	/* 0x5d */
};

#define IsU( c )		( 0x2526U == (c) )

private boolean_t IsKaToHo( ic_t c )
{
  if( 0x255bU < c ){		/* higher than `HO' */
    return FALSE;
  } else if( c <= 0x2541U ){	/* lower than `CHI' */
    if( 0x252bU <= c ){		/* between `KA' and `CHI' */
      c -= 0x252bU;
      if( 0 == c % 2 )
	return TRUE;
    }
  } else {			/* between `DI' and `HO' */
    if( 0x254fU <= c ){		/* between `HA' and `HO' */
      c -= 0x254fU;
      if( 0 == c % 3 )
	return TRUE;
    } else if( 0x2544U <= c && c <= 0x2548U ){	/* between `TSU' and `TO' */
      c -= 0x2544U;
      if( 0 == c % 2 )
	return TRUE;
    }
  }

  return FALSE;
}

private boolean_t IsHaToHo( ic_t c )
{
  if( 0x255bU < c ){		/* higher than `HO' */
    return FALSE;
  } else if( 0x254fU <= c ){		/* between `HA' and `HO' */
    c -= 0x254fU;
    if( 0 == c % 3 )
      return TRUE;
  }

  return FALSE;
}

public void KanaX0201toX0208()
{
  int index;
  ic_t x0201;

  /*
   * Remember this function is called only when:
   *   ISIDX > 0 and X0201KANA == ISTR[ ISIDX - 1 ].charset
   */

  index = ISIDX - 1;
  x0201 = ISTR[ index ].c;

  if( x0201 >= 0x21 && x0201 <= 0x5d ){
    ISTR[ index ].charset = X0208;
    ISTR[ index ].c       = X0201toX0208[ x0201 - 0x21 ];
  } else if( x0201 == 0x5e ){
    /* katakana voiced sound mark */
    if( ISIDX > 1 && X0208 == ISTR[ ISIDX - 2 ].charset ){
      index--;
      if( IsU( ISTR[ index ].c ) ){
	ISTR[ index ].c = 0x2574U;
	ISIDX--;
	return;
      } else if( IsKaToHo( ISTR[ index ].c ) ){
	ISTR[ index ].c++;
	ISIDX--;
	return;
      }
      index++;
    }
    ISTR[ index ].charset = X0208;
    ISTR[ index ].c       = 0x212bU;
  } else if( x0201 == 0x5f ){
    /* katakana semi-voiced sound mark */
    if( ISIDX > 1 && X0208 == ISTR[ ISIDX - 2 ].charset ){
      index--;
      if( IsHaToHo( ISTR[ index ].c ) ){
	ISTR[ index ].c += 2;
	ISIDX--;
	return;
      }
      index++;
    }
    ISTR[ index ].charset = X0208;
    ISTR[ index ].c       = 0x212cU;
  } else {
    /* undefined region */
    ISTR[ index ].charset = ASCII;
    ISTR[ index ].c       = (ic_t)'?';
  }
}
