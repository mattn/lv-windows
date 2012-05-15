/*
 * iso2kr.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: iso2kr.c,v 1.4 2004/01/05 07:23:29 nrt Exp $
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
#include <iso2kr.h>

/*
 * iso-2022-kr
 *
 * All charsets except ASCII use only G1 with locking shift.
 */
public void EncodeISO2022kr( i_str_t *istr, int head, int tail,
			    byte codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  byte cset, gl, lastCsetG1;
  boolean_t set94;

  attr = 0;
  gl = G0;
  lastCsetG1 = ASCII;

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    attr = istr[ idx ].attr << 8;
    ic = istr[ idx ].c;
    if( BIG5 == cset )
      ic = BIG5toCNS( ic, &cset );
#ifndef MSDOS /* IF NOT DEFINED */
    else if( UNICODE == cset )
      ic = UNItoKorean( ic, &cset );
#endif /* MSDOS */
    set94 = iTable[ (int)cset ].set94;
    if( ASCII == cset || cset > PSEUDO ){
      if( gl != G0 ){
	gl = G0;
	EncodeAddCharAbsolutely( attr, SI );
      }
    } else {
      if( gl != G1 ){
	gl = G1;
	EncodeAddCharAbsolutely( attr, SO );
      }
      if( lastCsetG1 != cset ){
	EncodeAddCharAbsolutely( attr, ESC );
	if( TRUE == iTable[ (int)cset ].multi ){
	  EncodeAddCharAbsolutely( attr, '$' );
	}
	if( TRUE == set94 ){
	  EncodeAddCharAbsolutely( attr, ')' ); /* designate set94 to G1 */
	} else {
	  EncodeAddCharAbsolutely( attr, '-' ); /* designate set96 to G1 */
	}
	EncodeAddCharAbsolutely( attr, iTable[ (int)cset ].fin );
      }
      lastCsetG1 = cset;
    }
    if( cset < PSEUDO ){
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
  if( gl != G0 ){
    EncodeAddCharAbsolutely( attr, SI );
  }
}
