/*
 * decode.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
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
#include <begin.h>
#include <decode.h>

private boolean_t overStrike = FALSE;
private boolean_t kanjiBackSpace = FALSE;

typedef void (*decode_table_t)( state_t *, char );

private decode_table_t decodeTable[ C_TABLE_SIZE ] = {
  DecodeISO2022,		/* AUTOSELECT */
  DecodeISO2022,		/* ISO_2022_CN */
  DecodeISO2022,		/* ISO_2022_JP */
  DecodeISO2022,		/* ISO_2022_KR */
  DecodeISO2022,		/* EUC_CHINA */
  DecodeISO2022,		/* EUC_JAPAN */
  DecodeISO2022,		/* EUC_KOREA */
  DecodeISO2022,		/* EUC_TAIWAN */
  DecodeShiftJis,		/* SHIFT_JIS */
  DecodeBig5,			/* BIG_FIVE */
  DecodeISO2022,		/* ISO_8859_1 */
  DecodeISO2022,		/* ISO_8859_2 */
  DecodeISO2022,		/* ISO_8859_3 */
  DecodeISO2022,		/* ISO_8859_4 */
  DecodeISO2022,		/* ISO_8859_5 */
  DecodeISO2022,		/* ISO_8859_6 */
  DecodeISO2022,		/* ISO_8859_7 */
  DecodeISO2022,		/* ISO_8859_8 */
  DecodeISO2022,		/* ISO_8859_9 */
#ifndef MSDOS /* IF NOT DEFINED */
  DecodeUTF,			/* UTF_7 */
  DecodeUTF,			/* UTF_8 */
#else /* MSDOS */
  DecodeRaw,
  DecodeRaw,
#endif /* MSDOS */
  DecodeRaw			/* RAW */
};

public void DecodeAddSpace( char attr )
{
  overStrike = FALSE;

  ISTR[ ISIDX ].charset = SPACE;
  ISTR[ ISIDX ].attr    = attr;
  ISTR[ ISIDX ].c       = (ic_t)SP;
  ISIDX++;
}

public void DecodeAddTab( char attr )
{
  overStrike = FALSE;

  ISTR[ ISIDX ].charset = HTAB;
  ISTR[ ISIDX ].attr    = attr;
  ISTR[ ISIDX ].c       = MakeIchar( HTAB_INTERNAL_WIDTH, HT );
  ISIDX++;
}

public void DecodeAddControl( char ch )
{
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

public void DecodeAddBs()
{
  if( TRUE == binary_decode )
    DecodeAddControl( BS );
  else if( ISIDX > 0 ){
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

public void DecodeAddIchar( char charset, ic_t ic, char attr )
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
}

public boolean_t DecodeAddShifted( state_t *state, char ch )
{
  char charset;
  char c[ ICHAR_WIDTH ];

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

public i_str_t *DecodeSimple( char *str, int *shigh )
{
  i_str_t *istr;
  int i, j;

  istr = IstrAlloc( *shigh + 1 );

  for( i = j = 0 ; i < *shigh ; i++ ){
    if( LF == str[ i ] )
      break;
    if( CR == str[ i ] )
      continue;
    if( SP == str[ i ] )
      istr[ j ].charset = SPACE;
    else
      istr[ j ].charset = ASCII;
    istr[ j ].attr = NULL;
    istr[ j ].c = (ic_t)str[ i ];
    j++;
  }

  istr[ j ].charset = NOSET;

  *shigh = j;

  return istr;
}

public i_str_t *Decode( char codingSystem, char *str, int *shigh )
{
  state_t state;

  ISIDX = 0;
  ISTR  = IstrAlloc( *shigh + 1 );
  SIDX  = 0;
  STR   = str;
  SHIGH = *shigh;	/* up to STR_SIZE */

  state = cTable[ (int)codingSystem ].state;

  overStrike = FALSE;

  (*decodeTable[ (int)codingSystem ])( &state, codingSystem );

  ISTR[ ISIDX ].charset = NOSET;

  *shigh = ISIDX;

  return ISTR;
}
