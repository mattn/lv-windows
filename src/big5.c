/*
 * big5.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: big5.c,v 1.6 2004/01/05 07:23:29 nrt Exp $
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
#include <decode.h>
#include <escape.h>
#include <encode.h>
#include <uty.h>
#include <unimap.h>
#include <begin.h>
#include <big5.h>

#include <big5cns.map>

private ic_t BinarySearchRange( codes_t *array, int high, ic_t code )
{
  int low, mid, distance, tmp;

  low = 0;
  mid = high >> 1;

  for( ; low <= high ; mid = ( low + high ) >> 1 ){
    if( ( array[ mid ].code <= code ) && ( array[ mid + 1 ].code > code ) ){
      if( 0 == array[ mid ].peer )
	return 0;
      if( code >= 0xa140U ){
	/* big5 to cns */
	tmp = ( ( code & 0xff00 ) - ( array[ mid ].code & 0xff00 ) ) >> 8;
	high = code & 0x00ff;
	low = array[ mid ].code & 0x00ff;
	/*
	 * NOTE: big5 high_byte: 0xa1-0xfe, low_byte: 0x40-0x7e, 0xa1-0xfe
	 *                                 (radicals: 0x00-0x3e, 0x3f-0x9c)
	 *  big5 radix is 0x9d.                     [region_low, region_high]
	 *  We should remember big5 has two different regions (above).
	 *  There is a bias for the distance between these regions.
	 *    0xa1 - 0x7e + bias = 1 (Distance between 0xa1 and 0x7e is 1.)
	 *                  bias = - 0x22.
	 */
	distance = tmp * 0x9d + high - low +
	  ( high >= 0xa1 ? ( low >= 0xa1 ? 0 : - 0x22 )
	   : ( low >= 0xa1 ? + 0x22 : 0 ) );
	/*
	 * NOTE: we have to convert the distance into a code point.
	 *       The code point's low_byte is 0x21 plus mod_0x5e.
	 *       In the first, we extract the mod_0x5e of the starting
	 *       code point, subtracting 0x21, and add distance to it.
	 *       Then we calculate again mod_0x5e of them, and restore
	 *       the final codepoint, adding 0x21.
	 */
	tmp = ( array[ mid ].peer & 0x00ff ) + distance - 0x21;
	tmp = ( array[ mid ].peer & 0xff00 ) + ( ( tmp / 0x5e ) << 8 )
	  + 0x21 + tmp % 0x5e;
	return tmp;
      } else {
	/* cns to big5 */
	tmp = ( ( code & 0xff00 ) - ( array[ mid ].code & 0xff00 ) ) >> 8;
	/*
	 * NOTE: ISO charsets ranges between 0x21-0xfe (94charset).
	 *       Its radix is 0x5e. But there is no distance bias like big5.
	 */
	distance = tmp * 0x5e
	  + ( (int)( code & 0x00ff ) - (int)( array[ mid ].code & 0x00ff ) );
	/*
	 * NOTE: Similar to big5 to cns conversion, we extract mod_0x9d and
	 *       restore mod_0x9d into a code point.
	 */
	low = array[ mid ].peer & 0x00ff;
	tmp = low + distance - ( low >= 0xa1 ? 0x62 : 0x40 );
	low = tmp % 0x9d;
	tmp = ( array[ mid ].peer & 0xff00 ) + ( ( tmp / 0x9d ) << 8 )
	  + ( low > 0x3e ? 0x62 : 0x40 ) + low;
	return tmp;
      }
    } else if( array[ mid ].code > code ){
      high = mid - 1;
    } else {
      low = mid + 1;
    }
  }

  return 0;
}

public ic_t BIG5toCNS( ic_t big5, byte *cset )
{
  ic_t cns = 0;

  if( big5 < 0xc940U ){
    /* level 1 */
    if( 0 < (cns = BinarySearchRange( big5Level1ToCnsPlane1, 23, big5 )) )
      *cset = CNS_1;
  } else if( big5 == 0xc94aU ){
    /* level 2 */
    *cset = CNS_1;
    cns = 0x4442;
  } else {
    /* level 2 */
    if( 0 < (cns = BinarySearchRange( big5Level2ToCnsPlane2, 46, big5 )) )
      *cset = CNS_2;
  }

  if( 0 == cns ){
    *cset = ASCII;
    return (ic_t)'?';
  }

  return cns;
}

public ic_t CNStoBIG5( ic_t cns, byte *cset )
{
  unsigned int big5 = 0;

  switch( *cset ){
  case CNS_1:
    big5 = BinarySearchRange( cnsPlane1ToBig5Level1, 24, cns );
    break;
  case CNS_2:
    big5 = BinarySearchRange( cnsPlane2ToBig5Level2, 47, cns );
    break;
  }

  if( 0 == big5 ){
    *cset = ASCII;
    return (ic_t)'?';
  }

  *cset = BIG5;

  return big5;
}

public void ConvertCNStoBIG5( i_str_t *istr )
{
  int i;
  byte cset;

  for( i = 0 ; NOSET != istr[ i ].charset ; i++ ){
    if( CNS_1 == istr[ i ].charset || CNS_2 == istr[ i ].charset ){
      cset = istr[ i ].charset;
      istr[ i ].c = CNStoBIG5( istr[ i ].c, &cset );
      istr[ i ].charset = cset;
    }
  }
}

public void ConvertBIG5toCNS( i_str_t *istr )
{
  int i;
  byte cset;

  for( i = 0 ; NOSET != istr[ i ].charset ; i++ ){
    if( BIG5 == istr[ i ].charset ){
      cset = istr[ i ].charset;
      istr[ i ].c = BIG5toCNS( istr[ i ].c, &cset );
      istr[ i ].charset = cset;
    }
  }
}

public void DecodeBig5( state_t *state, byte codingSystem )
{
  byte charset, ch;
  byte c[ ICHAR_WIDTH ];

  for( ; ; ){
    GetChar( ch );
    if( ch < SP ){
      if( ESC == ch ){
	if( FALSE == DecodeEscape( state ) )
	  break;
      } else if( HT == ch )
	DecodeAddTab( state->attr );
      else if( SO == ch )	/* LS1 for 8bit */
	state->gset[ GL ] = G1;
      else if( SI == ch )	/* LS0 for 8bit */
	state->gset[ GL ] = G0;
      else if( BS == ch )
	DecodeAddBs();
      else
	DecodeAddControl( ch );
    } else {
      if( 0 != state->sset ){
	if( FALSE == DecodeAddShifted( state, ch ) )
	  break;
	else
	  continue;
      } else if( IsBig5Byte1( ch ) ){
	/*
	 * Big5
	 */
	charset = BIG5;
	c[ 0 ] = ch;
	GetChar( ch );
	if( !IsBig5Byte2( ch ) ){
	  DecodeAddControl( c[ 0 ] );
	  DecodeAddControl( ch );
	  continue;
	}
	c[ 1 ] = ch;
      } else {
	/*
	 * iso-2022
	 */
	charset = CSET( G0 );
	if( !IsGraphicChar( charset, ch ) ){
	  if( SP == ch ){
	    DecodeAddSpace( state->attr );
	  } else {
	    DecodeAddControl( ch );
	  }
	  continue;
	}
	c[ 0 ] = ch;
	if( TRUE == iTable[ (int)charset ].multi ){
	  GetChar( ch );
	  ch &= 0x7f;
	  if( !IsGraphicChar( charset, ch ) )
	    continue;
	  c[ 1 ] = ch;
	}
      }
      DecodeAddChar( charset, c, state->attr );
    }
  }
}

public void EncodeBig5( i_str_t *istr, int head, int tail,
		       byte codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  byte cset;

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    ic = istr[ idx ].c;
    attr = (int)istr[ idx ].attr << 8;
    if( CNS_1 == cset || CNS_2 == cset )
      ic = CNStoBIG5( ic, &cset );
#ifndef MSDOS /* IF NOT DEFINED */
    else if( UNICODE == cset )
      ic = UNItoBIG5( ic, &cset );
#endif /* MSDOS */
    if( cset < PSEUDO ){
      if( ASCII == cset ){
	EncodeAddChar( attr, ic );
      } else if( BIG5 == cset ){
	EncodeAddChar( attr, MakeByte1( ic ) );
	EncodeAddChar( attr, MakeByte2( ic ) );
      } else {
	if( FALSE == EncodeAddInvalid( attr, ic, cset ) )
	  break;
      }
    } else if( FALSE == EncodeAddPseudo( attr, ic, cset, binary ) ){
      break;
    }
  }
}
