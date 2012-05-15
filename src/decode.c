/*
 * decode.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: decode.c,v 1.9 2004/01/05 07:23:29 nrt Exp $
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
#include <iso2022.h>
#include <shiftjis.h>
#include <big5.h>
#include <utf.h>
#include <raw.h>
#include <uty.h>
#include <kana.h>
#include <begin.h>
#include <decode.h>

private boolean_t overStrike = FALSE;
private boolean_t kanjiBackSpace = FALSE;

typedef void (*decode_table_t)( state_t *, byte );

private decode_table_t decodeTable[ C_TABLE_SIZE ] = {
  DecodeISO2022,		/* AUTOSELECT */
#ifndef MSDOS /* IF NOT DEFINED */
  DecodeUTF,			/* UTF_7 */
#else /* MSDOS */
  DecodeRaw,
#endif /* MSDOS */
  DecodeISO2022,		/* HZ_GB */
  DecodeISO2022,		/* EUC_KOREA */
  DecodeISO2022,		/* EUC_JAPAN */
  DecodeISO2022,		/* EUC_TAIWAN */
  DecodeISO2022,		/* EUC_CHINA */
  DecodeBig5,			/* BIG_FIVE */
  DecodeShiftJis,		/* SHIFT_JIS */
#ifndef MSDOS /* IF NOT DEFINED */
  DecodeUTF,			/* UTF_8 */
#else /* MSDOS */
  DecodeRaw,
#endif /* MSDOS */
  DecodeISO2022,		/* ISO_8859_1 */
  DecodeISO2022,		/* ISO_8859_2 */
  DecodeISO2022,		/* ISO_8859_3 */
  DecodeISO2022,		/* ISO_8859_4 */
  DecodeISO2022,		/* ISO_8859_5 */
  DecodeISO2022,		/* ISO_8859_6 */
  DecodeISO2022,		/* ISO_8859_7 */
  DecodeISO2022,		/* ISO_8859_8 */
  DecodeISO2022,		/* ISO_8859_9 */
  DecodeISO2022,		/* ISO_8859_10 */
  DecodeISO2022,		/* ISO_8859_11 */
  DecodeISO2022,		/* ISO_8859_13 */
  DecodeISO2022,		/* ISO_8859_14 */
  DecodeISO2022,		/* ISO_8859_15 */
  DecodeISO2022,		/* ISO_8859_16 */
  DecodeISO2022,		/* ISO_2022_CN */
  DecodeISO2022,		/* ISO_2022_JP */
  DecodeISO2022,		/* ISO_2022_KR */
  DecodeRaw			/* RAW */
};

public void DecodeAddSpace( byte attr )
{
  overStrike = FALSE;

  ISTR[ ISIDX ].charset = SPACE;
  ISTR[ ISIDX ].attr    = attr;
  ISTR[ ISIDX ].c       = (ic_t)SP;
  ISIDX++;
}

public void DecodeAddTab( byte attr )
{
  overStrike = FALSE;

  ISTR[ ISIDX ].charset = HTAB;
  ISTR[ ISIDX ].attr    = attr;
  ISTR[ ISIDX ].c       = MakeIchar( HTAB_INTERNAL_WIDTH, HT );
  ISIDX++;
}

private void DeleteChar()
{
  if( ISIDX > 0 ){
    if( FALSE == overStrike || FALSE == kanjiBackSpace ){
      overStrike = TRUE;
      ISIDX--;
      if( TRUE == iTable[ (int)ISTR[ ISIDX ].charset ].multi )
	kanjiBackSpace = TRUE;
      else
	kanjiBackSpace = FALSE;
    } else {
      kanjiBackSpace = FALSE;
    }
  }
}

public void DecodeAddBs()
{
  if( TRUE == binary_decode )
    DecodeAddControl( BS );
  else
    DeleteChar();
}

public void DecodeAddControl( byte ch )
{
  if( DEL == ch && FALSE == binary_decode ){
    DeleteChar();
    return;
  }

  overStrike = FALSE;

  ISTR[ ISIDX ].charset = CNTRL;
  ISTR[ ISIDX ].attr    = ATTR_STANDOUT;
  if( ch < SP )
    ISTR[ ISIDX ].c     = MakeIchar( CNTRLWIDTH_MIDDLEFORM, ch );
  else if( ch < DEL )
    ISTR[ ISIDX ].c     = MakeIchar( CNTRLWIDTH_SHORTFORM, ch );
  else
    ISTR[ ISIDX ].c     = MakeIchar( CNTRLWIDTH_LONGFORM, ch );
  ISIDX++;
}

public void DecodeAddIchar( byte charset, ic_t ic, byte attr )
{
  if( TRUE == overStrike ){
    overStrike = FALSE;

    if( ISTR[ ISIDX ].charset == charset && ISTR[ ISIDX ].c == ic )
      ISTR[ ISIDX ].attr |= ATTR_HILIGHT | attr;
    else if( ASCII == ISTR[ ISIDX ].charset && '_' == ISTR[ ISIDX ].c )
      ISTR[ ISIDX ].attr |= ATTR_UNDERLINE | attr;
    else if( ASCII == ISTR[ ISIDX ].charset && 'o' == ISTR[ ISIDX ].c
	    && ASCII == charset && '+' == ic ){
      ic = 'o';
      ISTR[ ISIDX ].attr |= ATTR_HILIGHT | attr;
    }
  } else
    ISTR[ ISIDX ].attr = attr;

  ISTR[ ISIDX ].charset = charset;
  ISTR[ ISIDX ].c = ic;

  ISIDX++;

  if( X0201KANA == charset && TRUE == kana_conv )
    KanaX0201toX0208();
}

public boolean_t DecodeAddShifted( state_t *state, byte ch )
{
  byte charset;
  byte c[ ICHAR_WIDTH ];

  /*
   * single shifted character
   */
  charset = SSET;
  state->sset = 0;
  ch &= 0x7f;	/* for EUC */
  if( !IsGraphicChar( charset, ch ) ){
    if( SP == ch ){
      DecodeAddSpace( state->attr );
    } else {
      DecodeAddControl( ch );
    }
    return TRUE;
  }
  c[ 0 ] = ch;
  if( TRUE == iTable[ (int)charset ].multi ){
    GetCharRet( ch );
    ch &= 0x7f;	/* for EUC */
    if( !IsGraphicChar( charset, ch ) )
      return TRUE;
    c[ 1 ] = ch;
  }

  DecodeAddChar( charset, c, state->attr );

  return TRUE;
}

private byte simpleCharset[ 128 ];

public i_str_t *DecodeSimple( i_str_t *istr, byte *str, int *shigh )
{
  int i, j;

  for( i = j = 0 ; i < *shigh ; i++ ){
    istr[ j ].charset = simpleCharset[ str[ i ] ];
    istr[ j ].attr = 0;
    istr[ j ].c = (ic_t)str[ i ];
    j++;
  }

  istr[ j ].charset = NOSET;

  *shigh = j;

  return istr;
}

public i_str_t *Decode( i_str_t *istr,
		       byte codingSystem, byte *str, int *shigh )
{
  state_t state;
  boolean_t linefeeded = FALSE;

  ISIDX = 0;
  ISTR  = istr;
  SIDX  = 0;
  STR   = str;
  SHIGH = *shigh;

  if( LF == str[ SHIGH - 1 ] ){
    linefeeded = TRUE;
    SHIGH--;
  }

  state = cTable[ (int)codingSystem ].state;

  overStrike = FALSE;

  (*decodeTable[ (int)codingSystem ])( &state, codingSystem );

  if( TRUE == linefeeded ){
    ISTR[ ISIDX ].charset = LINE_FEED;
    ISTR[ ISIDX ].attr    = 0;
    ISTR[ ISIDX ].c       = (ic_t)LF;
    ISIDX++;
  }

  ISTR[ ISIDX ].charset = NOSET;

  *shigh = ISIDX;

  return ISTR;
}

public void DecodeInit()
{
  int i;

  for( i = 0 ; i < 128 ; i++ )
    simpleCharset[ i ] = ASCII;

  simpleCharset[ CR ] = LINE_FEED;
  simpleCharset[ LF ] = LINE_FEED;
  simpleCharset[ SP ] = SPACE;
}
