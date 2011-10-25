/*
 * big5.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <import.h>
#include <decode.h>
#include <escape.h>
#include <encode.h>
#include <uty.h>
#include <unimap.h>
#include <begin.h>
#include <big5.h>

private codes_t big5Level1ToCnsPlane1[ 25 ] = {	/* range */
  { 0xA140, 0x2121 },
  { 0xA1F6, 0x2258 },
  { 0xA1F7, 0x2257 },
  { 0xA1F8, 0x2259 },
  { 0xA2AF, 0x2421 },
  { 0xA3C0, 0x4221 },
  { 0xa3e1, 0x0000 },
  { 0xA440, 0x4421 },
  { 0xACFE, 0x5753 },
  { 0xacff, 0x0000 },
  { 0xAD40, 0x5323 },
  { 0xAFD0, 0x5754 },
  { 0xBBC8, 0x6B51 },
  { 0xBE52, 0x6B50 },
  { 0xBE53, 0x6F5C },
  { 0xC1AB, 0x7536 },
  { 0xC2CB, 0x7535 },
  { 0xC2CC, 0x7737 },
  { 0xC361, 0x782E },
  { 0xC3B9, 0x7865 },
  { 0xC3BA, 0x7864 },
  { 0xC3BB, 0x7866 },
  { 0xC456, 0x782D },
  { 0xC457, 0x7962 },
  { 0xc67f, 0x0000 }
};

private codes_t big5Level2ToCnsPlane2[ 48 ] = {	/* range */
  { 0xC940, 0x2121 },
  { 0xc94a, 0x0000 },
  { 0xC94B, 0x212B },
  { 0xC96C, 0x214D },
  { 0xC9BE, 0x214C },
  { 0xC9BF, 0x217D },
  { 0xC9ED, 0x224E },
  { 0xCAF7, 0x224D },
  { 0xCAF8, 0x2439 },
  { 0xD77A, 0x3F6A },
  { 0xD77B, 0x387E },
  { 0xDBA7, 0x3F6B },
  { 0xDDFC, 0x4176 },
  { 0xDDFD, 0x4424 },
  { 0xE8A3, 0x554C },
  { 0xE976, 0x5723 },
  { 0xEB5B, 0x5A29 },
  { 0xEBF1, 0x554B },
  { 0xEBF2, 0x5B3F },
  { 0xECDE, 0x5722 },
  { 0xECDF, 0x5C6A },
  { 0xEDAA, 0x5D75 },
  { 0xEEEB, 0x642F },
  { 0xEEEC, 0x6039 },
  { 0xF056, 0x5D74 },
  { 0xF057, 0x6243 },
  { 0xF0CB, 0x5A28 },
  { 0xF0CC, 0x6337 },
  { 0xF163, 0x6430 },
  { 0xF16B, 0x6761 },
  { 0xF16C, 0x6438 },
  { 0xF268, 0x6934 },
  { 0xF269, 0x6573 },
  { 0xF2C3, 0x664E },
  { 0xF375, 0x6762 },
  { 0xF466, 0x6935 },
  { 0xF4B5, 0x664D },
  { 0xF4B6, 0x6962 },
  { 0xF4FD, 0x6A4C },
  { 0xF663, 0x6A4B },
  { 0xF664, 0x6C52 },
  { 0xF977, 0x7167 },
  { 0xF9C4, 0x7166 },
  { 0xF9C5, 0x7234 },
  { 0xF9C6, 0x7240 },
  { 0xF9C7, 0x7235 },
  { 0xF9D2, 0x7241 },
  { 0xf9d6, 0x0000 }
};

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

public ic_t BIG5toCNS( ic_t big5, char *cset )
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

private codes_t cnsPlane1ToBig5Level1[ 26 ] = {	/* range */
  { 0x2121, 0xA140 },
  { 0x2257, 0xA1F7 },
  { 0x2258, 0xA1F6 },
  { 0x2259, 0xA1F8 },
  { 0x234f, 0x0000 },
  { 0x2421, 0xA2AF },
  { 0x2571, 0x0000 },
  { 0x4221, 0xA3C0 },
  { 0x4242, 0x0000 },
  { 0x4421, 0xA440 },
  { 0x5323, 0xAD40 },
  { 0x5753, 0xACFE },
  { 0x5754, 0xAFD0 },
  { 0x6B50, 0xBE52 },
  { 0x6B51, 0xBBC8 },
  { 0x6F5C, 0xBE53 },
  { 0x7535, 0xC2CB },
  { 0x7536, 0xC1AB },
  { 0x7737, 0xC2CC },
  { 0x782D, 0xC456 },
  { 0x782E, 0xC361 },
  { 0x7864, 0xC3BA },
  { 0x7865, 0xC3B9 },
  { 0x7866, 0xC3BB },
  { 0x7962, 0xC457 },
  { 0x7d4c, 0x0000 }
};

private codes_t cnsPlane2ToBig5Level2[ 49 ] = {	/* range */
  { 0x2121, 0xC940 },
  { 0x212B, 0xC94B },
  { 0x214C, 0xC9BE },
  { 0x214D, 0xC96C },
  { 0x217D, 0xC9BF },
  { 0x224D, 0xCAF7 },
  { 0x224E, 0xC9ED },
  { 0x2439, 0xCAF8 },
  { 0x387E, 0xD77B },
  { 0x3F6A, 0xD77A },
  { 0x3F6B, 0xDBA7 },
  { 0x4424, 0x0000 },
  { 0x4176, 0xDDFC },
  { 0x4177, 0x0000 },
  { 0x4424, 0xDDFD },
  { 0x554B, 0xEBF1 },
  { 0x554C, 0xE8A3 },
  { 0x5722, 0xECDE },
  { 0x5723, 0xE976 },
  { 0x5A28, 0xF0CB },
  { 0x5A29, 0xEB5B },
  { 0x5B3F, 0xEBF2 },
  { 0x5C6A, 0xECDF },
  { 0x5D74, 0xF056 },
  { 0x5D75, 0xEDAA },
  { 0x6039, 0xEEEC },
  { 0x6243, 0xF057 },
  { 0x6337, 0xF0CC },
  { 0x642F, 0xEEEB },
  { 0x6430, 0xF163 },
  { 0x6438, 0xF16C },
  { 0x6573, 0xF269 },
  { 0x664D, 0xF4B5 },
  { 0x664E, 0xF2C3 },
  { 0x6761, 0xF16B },
  { 0x6762, 0xF375 },
  { 0x6934, 0xF268 },
  { 0x6935, 0xF466 },
  { 0x6962, 0xF4B6 },
  { 0x6A4B, 0xF663 },
  { 0x6A4C, 0xF4FD },
  { 0x6C52, 0xF664 },
  { 0x7166, 0xF9C4 },
  { 0x7167, 0xF977 },
  { 0x7234, 0xF9C5 },
  { 0x7235, 0xF9C7 },
  { 0x7240, 0xF9C6 },
  { 0x7241, 0xF9D2 },
  { 0x7245, 0x0000 }
};

public ic_t CNStoBIG5( ic_t cns, char *cset )
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

#define IsBig5Byte1( c )						\
  ( ( (c) >= 0xa1 && (c) <= 0xfe ) )

#define IsBig5Byte2( c )						\
  ( ( (c) >= 0x40 && (c) <= 0x7e ) || ( (c) >= 0xa1 && (c) <= 0xfe ) )

public void DecodeBig5( state_t *state, char codingSystem )
{
  char charset, ch;
  char c[ ICHAR_WIDTH ];

  for( ; ; ){
    GetChar( ch );
    if( ch < SP ){
      if( CR == ch )
	continue;
      else if( ESC == ch ){
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
      if( NULL != state->sset ){
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
		       char codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  char cset;

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
