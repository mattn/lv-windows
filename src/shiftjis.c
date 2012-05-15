/*
 * shiftjis.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: shiftjis.c,v 1.7 2004/01/05 07:23:29 nrt Exp $
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
#include <unimap.h>
#include <big5.h>
#include <begin.h>
#include <shiftjis.h>

private void msk2jis( byte *c )
{
  int c1, c2;

  c1 = (int)c[ 0 ];
  c2 = (int)c[ 1 ];

  if( 0x00e0 <= c1 )
    c1 = ( c1 << 1 ) - 0x0160;			/* 63-94 ku */
  else
    c1 = ( c1 << 1 ) - 0x00e0;			/* 01-62 ku */

  if( 0x009f <= c2 )
    c2 -= 0x007e;				/* even ku */
  else {
    c1--;					/* odd ku */
    if( c2 >= (int)DEL )
      c2 -= 0x0020;
    else
      c2 -= 0x001f;
  }

  c[ 0 ] = (byte)c1;
  c[ 1 ] = (byte)c2;
}

public void DecodeShiftJis( state_t *state, byte codingSystem )
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
      } else if( 0x80 & ch ){
	if( IsShiftJisByte1( ch ) ){
	  charset = X0208;
	  c[ 0 ] = ch;
	  GetChar( ch );
	  c[ 1 ] = ch;
	  msk2jis( c );
	  if( !IsGraphicChar94( c[ 0 ] ) || !IsGraphicChar94( c[ 1 ] ) ){
	    DecodeAddControl( ch );
	    continue;
	  }
	} else if( IsKatakana( 0x7f & ch ) ){
	  charset = X0201KANA;
	  c[ 0 ] = 0x7f & ch;
	} else {
	  DecodeAddControl( ch );
	  continue;
	}
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
	} else if( X0201KANA == charset && !IsKatakana( ch ) ){
	  DecodeAddControl( ch );
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

private void jis2msk( byte *c )
{
  int c1, c2;

  c1 = (int)c[ 0 ];
  c2 = (int)c[ 1 ];

  if( 0 == ( c1 & 0x0001 ) )
    c2 += 0x007e;				/* even ku */
  else {
    if( c2 >= 0x0060 )				/* odd ku */
      c2 += 0x0020;
    else
      c2 += 0x001f;
  }

  if( 0x005f <= c1 )
    c1 = ( ( c1 - 0x005f ) >> 1 ) + 0x00e0;	/* 63-94 ku */
  else
    c1 = ( ( c1 - 0x0021 ) >> 1 ) + 0x0081;	/* 01-62 ku */

  c[ 0 ] = (byte)c1;
  c[ 1 ] = (byte)c2;
}

public void EncodeShiftJis( i_str_t *istr, int head, int tail,
			   byte codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  byte cset, sj[ 2 ];

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    ic = istr[ idx ].c;
    attr = (int)istr[ idx ].attr << 8;
#ifndef MSDOS /* IF NOT DEFINED */
    if( UNICODE == cset )
      ic = UNItoJIS( ic, &cset );
#endif /* MSDOS */
    if( cset < PSEUDO ){
      if( ASCII == cset ){
	EncodeAddChar( attr, ic );
      } else if( X0208 == cset || C6226 == cset ){
	sj[ 0 ] = MakeByte1( ic );
	sj[ 1 ] = MakeByte2( ic );
	jis2msk( sj );
	EncodeAddChar( attr, sj[ 0 ] );
	EncodeAddChar( attr, sj[ 1 ] );
      } else if( X0201KANA == cset ){
	EncodeAddChar( attr, 0x80 | ic );
      } else {
	if( FALSE == EncodeAddInvalid( attr, ic, cset ) )
	  break;
      }
    } else if( FALSE == EncodeAddPseudo( attr, ic, cset, binary ) ){
      break;
    }
  }
}
