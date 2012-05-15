/*
 * eucjapan.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: eucjapan.c,v 1.3 2003/11/13 03:08:19 nrt Exp $
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
#include <encode.h>
#include <unimap.h>
#include <big5.h>
#include <begin.h>
#include <eucjapan.h>

public void EncodeEUCjp( i_str_t *istr, int head, int tail,
			byte codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  byte cset, g0, g1, g2, g3;

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
