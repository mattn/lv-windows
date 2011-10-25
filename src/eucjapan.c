/*
 * eucjapan.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <import.h>
#include <encode.h>
#include <big5.h>
#include <unimap.h>
#include <begin.h>
#include <eucjapan.h>

public void EncodeEUCjp( i_str_t *istr, int head, int tail,
			char codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  char cset, g0, g1, g2, g3;

  g0 = cTable[ (int)codingSystem ].state.cset[ G0 ];
  g1 = cTable[ (int)codingSystem ].state.cset[ G1 ];
  g2 = cTable[ (int)codingSystem ].state.cset[ G2 ];
  g3 = cTable[ (int)codingSystem ].state.cset[ G3 ];

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    ic = istr[ idx ].c;
    attr = (int)istr[ idx ].attr << 8;
    if( BIG5 == cset )
      ic = BIG5toCNS( ic, &cset );
#ifndef MSDOS /* IF NOT DEFINED */
    else if( UNICODE == cset ){
      switch( codingSystem ){
      case EUC_CHINA: ic = UNItoGB( ic, &cset ); break;
      case EUC_JAPAN: ic = UNItoJIS( ic, &cset ); break;
      case EUC_KOREA: ic = UNItoKSC( ic, &cset ); break;
      case EUC_TAIWAN:ic = UNItoCNS( ic, &cset ); break;
      default: ic = UNItoJIS( ic, &cset );
      }
    }
#endif /* MSDOS */
    if( cset < PSEUDO ){
      if( g0 == cset ){
	if( TRUE == iTable[ (int)cset ].multi ){
	  EncodeAddChar( attr, MakeByte1( ic ) );
	  EncodeAddChar( attr, MakeByte2( ic ) );
	} else {
	  EncodeAddChar( attr, ic );
	}
	continue;
      } else if( C6226 == cset && EUC_JAPAN == codingSystem ){
	/* Japanese alternative g1 set */
      } else if( g1 == cset ){
      } else if( EUC_TAIWAN == codingSystem
		&& ( cset >= CNS_2 && cset <= CNS_7 ) ){
	EncodeAddChar( attr, SS2 );
	EncodeAddChar( attr, ( cset - CNS_1 ) + 0xa1 );
      } else if( g2 == cset ){
	EncodeAddChar( attr, SS2 );
      } else if( g3 == cset ){
	EncodeAddChar( attr, SS3 );
      } else {
        if( FALSE == EncodeAddInvalid( attr, ic, cset ) )
          break;
        else
          continue;
      }
      if( TRUE == iTable[ (int)cset ].multi ){
	EncodeAddChar( attr, 0x80 | MakeByte1( ic ) );
	EncodeAddChar( attr, 0x80 | MakeByte2( ic ) );
      } else {
	EncodeAddChar( attr, 0x80 | ic );
      }
    } else if( FALSE == EncodeAddPseudo( attr, ic, cset, binary ) ){
      break;
    }
  }
}
