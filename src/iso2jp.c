/*
 * iso2jp.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdio.h>

#include <import.h>
#include <encode.h>
#include <big5.h>
#include <unimap.h>
#include <begin.h>
#include <iso2jp.h>

/*
 * iso-2022-jp, iso-8859-*
 *
 * all 94charsets use G0, and all 96charsets use G2 with single shift,
 * and one pre-designated 8bit charset uses G1.
 */
public void EncodeISO2022jp( i_str_t *istr, int head, int tail,
			    char codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  char cset, lastCset, lastCsetG0, lastCsetG2, csetG1 = ASCII;
  boolean_t lastSet94, set94, bit8;

  attr = NULL;
  lastCset = lastCsetG0 = lastCsetG2 = ASCII;
  lastSet94 = TRUE;

  if( TRUE == cTable[ (int)codingSystem ].bit8 ){
    bit8 = TRUE;
    csetG1 = cTable[ (int)codingSystem ].state.cset[ G1 ];
  } else {
    bit8 = FALSE;
  }

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    ic = istr[ idx ].c;
    attr = (int)istr[ idx ].attr << 8;
    if( BIG5 == cset )
      ic = BIG5toCNS( ic, &cset );
#ifndef MSDOS /* IF NOT DEFINED */
    else if( UNICODE == cset )
      ic = UNItoJIS( ic, &cset );
#endif /* MSDOS */
    set94 = iTable[ (int)cset ].set94;
    if( TRUE == bit8 && ASCII != cset && csetG1 == cset ){
      if( TRUE == iTable[ (int)cset ].multi ){
	EncodeAddChar( attr, 0x80 | MakeByte1( ic ) );
	EncodeAddChar( attr, 0x80 | MakeByte2( ic ) );
      } else {
	EncodeAddChar( attr, 0x80 | ic );
      }
      continue;
    }
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
