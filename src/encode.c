/*
 * encode.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdio.h>
#include <stdlib.h>

#include <import.h>
#include <iso2cn.h>
#include <iso2jp.h>
#include <iso2kr.h>
#include <eucjapan.h>
#include <shiftjis.h>
#include <big5.h>
#include <utf.h>
#include <raw.h>
#include <begin.h>
#include <encode.h>

typedef void (*encode_table_t)( i_str_t *, int, int, char, boolean_t );

private encode_table_t encodeTable[ C_TABLE_SIZE ] = {
  EncodeISO2022jp,		/* AUTOSELECT */
  EncodeISO2022cn,		/* ISO_2022_CN */
  EncodeISO2022jp,		/* ISO_2022_JP */
  EncodeISO2022kr,		/* ISO_2022_KR */
  EncodeEUCjp,			/* EUC_CHINA */
  EncodeEUCjp,			/* EUC_JAPAN */
  EncodeEUCjp,			/* EUC_KOREA */
  EncodeEUCjp,			/* EUC_TAIWAN */
  EncodeShiftJis,		/* SHIFT_JIS */
  EncodeBig5,			/* BIG_FIVE */
  EncodeISO2022jp,		/* ISO_8859_1 */
  EncodeISO2022jp,		/* ISO_8859_2 */
  EncodeISO2022jp,		/* ISO_8859_3 */
  EncodeISO2022jp,		/* ISO_8859_4 */
  EncodeISO2022jp,		/* ISO_8859_5 */
  EncodeISO2022jp,		/* ISO_8859_6 */
  EncodeISO2022jp,		/* ISO_8859_7 */
  EncodeISO2022jp,		/* ISO_8859_8 */
  EncodeISO2022jp,		/* ISO_8859_9 */
#ifndef MSDOS /* IF NOT DEFINED */
  EncodeUTF7,			/* UTF_7 */
  EncodeUTF8,			/* UTF_8 */
#else /* MSDOS */
  EncodeRaw,
  EncodeRaw,
#endif /* MSDOS */
  EncodeRaw			/* RAW */
};

#define HexChar( c )	( (c) < 10 ? '0' + (c) : (c) + '7' )

public boolean_t EncodeAddPseudo( int attr, ic_t ic, char cset,
				 boolean_t binary )
{
  int i;
  char c;

  if( SPACE == cset ){
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

public boolean_t EncodeAddInvalid( int attr, ic_t ic, char cset )
{
  char ch;

  if( TRUE == iTable[ (int)cset ].multi ){
    ch = (char)( 0x7f & MakeByte1( ic ) );
    if( ch >= 0x20 && ch <= 0x7e )
      EncodeAddCharRet( attr, ch );
    if( 2 == IcharWidth( cset, ic ) ){
      ch = (char)( 0x7f & MakeByte2( ic ) );
      if( ch >= 0x20 && ch <= 0x7e )
	EncodeAddCharRet( attr, ch );
    }
  } else {
    ch = (char)( 0x7f & ic );
    EncodeAddCharRet( attr, ch );
  }

  return TRUE;
}

public void EncodeAddEscapeDollar( char attr )
{
  EncodeAddCharAbsolutely( attr, ESC );
  EncodeAddCharAbsolutely( attr, '$' );
}

public void EncodeAdd7bitSS2( char attr )
{
  EncodeAddCharAbsolutely( attr, ESC );
  EncodeAddCharAbsolutely( attr, 'N' );
}

public void EncodeAdd7bitSS3( char attr )
{
  EncodeAddCharAbsolutely( attr, ESC );
  EncodeAddCharAbsolutely( attr, 'O' );
}

public void Encode( i_str_t *istr, int head, int tail,
		   char codingSystem, boolean_t binary,
		   str_t *code, int *length )
{
  CIDX = 0;
  CSTR = code;

  (*encodeTable[ (int)codingSystem ])( istr, head, tail,
				      codingSystem, binary );

  CSTR[ CIDX ] = NULL;

  *length = CIDX;
}
