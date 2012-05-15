/*
 * iso2cn.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: iso2cn.c,v 1.4 2004/01/05 07:23:29 nrt Exp $
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
#include <iso2cn.h>

/*
 * iso-2022-cn, iso-2022-cn-ext
 *
 * G0: ASCII
 * G1: GB 2312-80, CNS 11643-1992 Plane 1, and ISO-IR-165
 * G2: CNS 11643-1992 Pane 2
 * G3: CNS 11643-1992 Pane 3, 4, 5, 6, 7
 */

public void EncodeISO2022cn( i_str_t *istr, int head, int tail,
			    byte codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  byte cset, gl, lastCsetG1, lastCsetG2, lastCsetG3, shiftState;
  boolean_t set94;

  attr = 0;
  gl = G0;
  lastCsetG1 = lastCsetG2 = lastCsetG3 = ASCII;
  shiftState = NUL;

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    ic = istr[ idx ].c;
    attr = (int)istr[ idx ].attr << 8;
    if( BIG5 == cset )
      ic = BIG5toCNS( ic, &cset );
#ifndef MSDOS /* IF NOT DEFINED */
    else if( UNICODE == cset )
      ic = UNItoChinese( ic, &cset );
#endif /* MSDOS */
    set94 = iTable[ (int)cset ].set94;
    if( ASCII == cset || cset > PSEUDO ){
      shiftState = NUL;
      if( gl != G0 ){
	gl = G0;
	EncodeAddCharAbsolutely( attr, SI );
      }
    } else if( CNS_1 == cset || GB2312 == cset || ISO_IR_165 == cset ){
      shiftState = NUL;
      if( gl != G1 ){
        gl = G1;
        EncodeAddCharAbsolutely( attr, SO );
      }
      if( lastCsetG1 != cset ){
        EncodeAddEscapeDollar( attr );
        EncodeAddCharAbsolutely( attr, ')' );
	EncodeAddCharAbsolutely( attr, iTable[ (int)cset ].fin );
      }
      lastCsetG1 = cset;
    } else if( cset >= CNS_3 && cset <= CNS_7 ){
      shiftState = SS3;
      if( lastCsetG3 != cset ){
	EncodeAddEscapeDollar( attr );
	EncodeAddCharAbsolutely( attr, '+' );
	EncodeAddCharAbsolutely( attr, iTable[ (int)cset ].fin );
      }
      lastCsetG3 = cset;
    } else {
      /* CNS_2 */
      shiftState = SS2;
      if( lastCsetG2 != cset ){
	EncodeAddCharAbsolutely( attr, ESC );
	if( TRUE == iTable[ (int)cset ].multi ){
	  EncodeAddCharAbsolutely( attr, '$' );
	}
	if( TRUE == set94 ){
	  EncodeAddCharAbsolutely( attr, '*' ); /* designate set94 to G2 */
	} else {
	  EncodeAddCharAbsolutely( attr, '.' ); /* designate set96 to G2 */
	}
	EncodeAddCharAbsolutely( attr, iTable[ (int)cset ].fin );
      }
      lastCsetG2 = cset;
    }
    if( cset < PSEUDO ){
      switch( shiftState ){
      case SS2:
	EncodeAdd7bitSS2( attr );
	break;
      case SS3:
	EncodeAdd7bitSS3( attr );
	break;
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
  if( gl != G0 ){
    EncodeAddCharAbsolutely( attr, SI );
  }
}
