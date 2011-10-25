/*
 * iso2kr.c
 */

#include <import.h>
#include <encode.h>
#include <big5.h>
#include <unimap.h>
#include <begin.h>
#include <iso2kr.h>

/*
 * iso-2022-kr
 *
 * All charsets except ASCII use only G1 with locking shift.
 */
public void EncodeISO2022kr( i_str_t *istr, int head, int tail,
			    char codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  char cset, gl, lastCsetG1;
  boolean_t set94;

  attr = NULL;
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
      ic = UNItoKSC( ic, &cset );
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
