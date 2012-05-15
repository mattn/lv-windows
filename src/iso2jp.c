/*
 * iso2jp.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: iso2jp.c,v 1.5 2004/01/05 07:23:29 nrt Exp $
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

#include <stdio.h>

#include <import.h>
#include <encode.h>
#include <unimap.h>
#include <big5.h>
#include <begin.h>
#include <iso2jp.h>

/*
 * iso-2022-jp
 *
 * all 94charsets use G0, and all 96charsets use G2 with single shift.
 */
public void EncodeISO2022jp( i_str_t *istr, int head, int tail,
			    byte codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  byte cset, lastCset, lastCsetG0, lastCsetG2;
  boolean_t lastSet94, set94, bit8 = FALSE;

  attr = 0;
  lastCset = lastCsetG0 = lastCsetG2 = ASCII;
  lastSet94 = TRUE;

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    ic = istr[ idx ].c;
    attr = (int)istr[ idx ].attr << 8;
    if( BIG5 == cset )
      ic = BIG5toCNS( ic, &cset );
#ifndef MSDOS /* IF NOT DEFINED */
    else if( UNICODE == cset )
      ic = UNItoJapanese( ic, &cset );
#endif /* MSDOS */
    set94 = iTable[ (int)cset ].set94;
    if( lastSet94 != set94 ){
      if( TRUE == set94 ){
	lastCsetG2 = lastCset;
	lastCset = lastCsetG0;
      } else {
	lastCsetG0 = lastCset;
	lastCset = lastCsetG2;
      }
    }
    lastSet94 = set94;
    if( lastCset != cset ){
      if( !( ( lastCset == ASCII && cset > PSEUDO )
	    || ( lastCset > PSEUDO && ASCII == cset )
	    || ( lastCset > PSEUDO && cset > PSEUDO ) ) ){
	EncodeAddCharAbsolutely( attr, ESC );
	if( TRUE == iTable[ (int)cset ].multi ){
	  EncodeAddCharAbsolutely( attr, '$' );
	  if( !( cset == X0208 || cset == C6226 || cset == GB2312 ) ){
	    if( TRUE == set94 ){
	      EncodeAddCharAbsolutely( attr, '(' );
	    } else {
	      EncodeAddCharAbsolutely( attr, '.' );
	    }
	  }
	} else {
	  if( TRUE == set94 ){
	    EncodeAddCharAbsolutely( attr, '(' ); /* designate set94 to G0 */
	  } else {
	    EncodeAddCharAbsolutely( attr, '.' ); /* designate set96 to G2 */
	  }
	}
	EncodeAddCharAbsolutely( attr, iTable[ (int)cset ].fin );
      }
    }
    lastCset = cset;
    if( cset < PSEUDO ){
      if( FALSE == set94 ){
	if( TRUE == bit8 ){
	  EncodeAddChar( attr, SS2 );
	} else {
	  EncodeAdd7bitSS2( attr );
	}
      }
      if( TRUE == iTable[ (int)cset ].multi ){
	EncodeAddChar( attr, MakeByte1( ic ) );
	EncodeAddChar( attr, MakeByte2( ic ) );
      } else {
	EncodeAddChar( attr, ic );
      }
    } else if( FALSE == EncodeAddPseudo( attr, ic, cset, binary ) ){
      break;
    }
  }
  if( TRUE != lastSet94 )
    lastCset = lastCsetG0;
  if( ASCII != lastCset && lastCset < PSEUDO ){
    EncodeAddCharAbsolutely( attr, ESC );
    EncodeAddCharAbsolutely( attr, '(' );
    EncodeAddCharAbsolutely( attr, iTable[ ASCII ].fin );
  }
}
