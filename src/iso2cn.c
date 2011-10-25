/*
 * iso2cn.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <import.h>
#include <encode.h>
#include <big5.h>
#include <unimap.h>
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
			    char codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  char cset, gl, lastCsetG1, lastCsetG2, lastCsetG3, shiftState;
  boolean_t set94;

  attr = NULL;
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
    else if( UNICODE == cset ){
      ic_t res;
      char cs;

      cs = cset;
      res = UNItoGB( ic, &cs );
      if( ASCII == cs ){
	ic = UNItoCNS( ic, &cset );
      } else {
	ic = res;
	cset = cs;
      }
    }
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
    } else if( ( cset >= CNS_3 && cset <= CNS_7 ) ){
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
