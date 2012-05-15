/*
 * encode.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: encode.c,v 1.7 2004/01/05 07:23:29 nrt Exp $
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
#include <stdlib.h>

#include <import.h>
#include <iso8859.h>
#include <iso2cn.h>
#include <iso2jp.h>
#include <iso2kr.h>
#include <eucjapan.h>
#include <shiftjis.h>
#include <big5.h>
#include <utf.h>
#include <hz.h>
#include <raw.h>
#include <begin.h>
#include <encode.h>

typedef void (*encode_table_t)( i_str_t *, int, int, byte, boolean_t );

private encode_table_t encodeTable[ C_TABLE_SIZE ] = {
  EncodeISO2022jp,		/* AUTOSELECT */
#ifndef MSDOS /* IF NOT DEFINED */
  EncodeUTF7,			/* UTF_7 */
#else /* MSDOS */
  EncodeRaw,
#endif /* MSDOS */
  EncodeHz,			/* HZ_GB */
  EncodeEUCjp,			/* EUC_KOREA */
  EncodeEUCjp,			/* EUC_JAPAN */
  EncodeEUCjp,			/* EUC_TAIWAN */
  EncodeEUCjp,			/* EUC_CHINA */
  EncodeBig5,			/* BIG_FIVE */
  EncodeShiftJis,		/* SHIFT_JIS */
#ifndef MSDOS /* IF NOT DEFINED */
  EncodeUTF8,			/* UTF_8 */
#else /* MSDOS */
  EncodeRaw,
#endif /* MSDOS */
  EncodeISO8859,		/* ISO_8859_1 */
  EncodeISO8859,		/* ISO_8859_2 */
  EncodeISO8859,		/* ISO_8859_3 */
  EncodeISO8859,		/* ISO_8859_4 */
  EncodeISO8859,		/* ISO_8859_5 */
  EncodeISO8859,		/* ISO_8859_6 */
  EncodeISO8859,		/* ISO_8859_7 */
  EncodeISO8859,		/* ISO_8859_8 */
  EncodeISO8859,		/* ISO_8859_9 */
  EncodeISO8859,		/* ISO_8859_10 */
  EncodeISO8859,		/* ISO_8859_11 */
  EncodeISO8859,		/* ISO_8859_13 */
  EncodeISO8859,		/* ISO_8859_14 */
  EncodeISO8859,		/* ISO_8859_15 */
  EncodeISO8859,		/* ISO_8859_16 */
  EncodeISO2022cn,		/* ISO_2022_CN */
  EncodeISO2022jp,		/* ISO_2022_JP */
  EncodeISO2022kr,		/* ISO_2022_KR */
  EncodeRaw			/* RAW */
};

#define HexChar( c )	( (c) < 10 ? '0' + (c) : (c) + '7' )

public boolean_t EncodeAddPseudo( int attr, ic_t ic, byte cset,
				 boolean_t binary )
{
  int i;
  byte c;

  if( LINE_FEED == cset ){
    EncodeAddCharRet( attr, LF );
  }else if( SPACE == cset ){
    EncodeAddCharRet( attr, SP );
  } else if( HTAB == cset ){
    if( TRUE == binary ){
      EncodeAddCharRet( attr, HT );
    } else {
      for( i = 0 ; i < MakeByte1( ic ) ; i++ )
	EncodeAddCharRet( attr, ' ' );
    }
  } else if( CNTRL == cset ){
    if( TRUE == binary ){
      EncodeAddCharRet( attr, ic );
    } else {
      c = MakeByte2( ic );
      if( c < SP ){
	EncodeAddCharRet( attr, '^' );
	EncodeAddCharRet( attr, '@' + c );
      } else if( c < DEL ){
	EncodeAddCharRet( attr, c );
      } else {
	EncodeAddCharRet( attr, '<' );
	EncodeAddCharRet( attr, HexChar( ( 0xf0 & c ) >> 4 ) );
	EncodeAddCharRet( attr, HexChar( ( 0x0f & c ) ) );
	EncodeAddCharRet( attr, '>' );
      }
    }
  }

  return TRUE;
}

public boolean_t EncodeAddInvalid( int attr, ic_t ic, byte cset )
{
  byte ch;

  if( TRUE == iTable[ (int)cset ].multi ){
    ch = (byte)( 0x7f & MakeByte1( ic ) );
    if( ch >= 0x20 && ch <= 0x7e )
      EncodeAddCharRet( attr, ch );
    if( 2 == IcharWidth( cset, ic ) ){
      ch = (byte)( 0x7f & MakeByte2( ic ) );
      if( ch >= 0x20 && ch <= 0x7e )
	EncodeAddCharRet( attr, ch );
    }
  } else {
    ch = (byte)( 0x7f & ic );
    EncodeAddCharRet( attr, ch );
  }

  return TRUE;
}

public void EncodeAddEscapeDollar( int attr )
{
  EncodeAddCharAbsolutely( attr, ESC );
  EncodeAddCharAbsolutely( attr, '$' );
}

public void EncodeAdd7bitSS2( int attr )
{
  EncodeAddCharAbsolutely( attr, ESC );
  EncodeAddCharAbsolutely( attr, 'N' );
}

public void EncodeAdd7bitSS3( int attr )
{
  EncodeAddCharAbsolutely( attr, ESC );
  EncodeAddCharAbsolutely( attr, 'O' );
}

public void Encode( i_str_t *istr, int head, int tail,
		   byte codingSystem, boolean_t binary,
		   str_t *code, int *length )
{
  CIDX = 0;

  CSTR = code;
  CHIGH = *length - CODE_EXTRA_LEN;

  (*encodeTable[ (int)codingSystem ])( istr, head, tail,
				      codingSystem, binary );

  CSTR[ CIDX ] = 0x00;

  *length = CIDX;
}

private byte encode_str_stripped[ CODE_SIZE ];

public byte *EncodeStripAttribute( str_t *str, int length )
{
  int i;
  byte *ptr;

  if( length > CODE_SIZE )
    return NULL;

  ptr = encode_str_stripped;

  for( i = 0 ; i < length ; i++ )
    *ptr++ = 0xff & str[ i ];

  *ptr = 0x00;

  return encode_str_stripped;
}
