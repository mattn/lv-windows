/*
 * utf.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: utf.c,v 1.7 2004/01/05 07:23:29 nrt Exp $
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
#include <unirev.h>
#include <begin.h>
#include <utf.h>

private int base64char[ 64 ] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'
};

private int base64[ 256 ] = {
/* 0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0 */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 1 */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, /* 2 */
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, /* 3 */
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /* 4 */
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, /* 5 */
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /* 6 */
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, /* 7 */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 8 */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 9 */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* a */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* b */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* c */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* d */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* e */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  /* f */
};

private void DecodeAddUnicode( byte attr, ic_t uni )
{
  if( uni <= (ic_t)DEL ){
    if( uni <= (ic_t)SP ){
      if( (ic_t)SP == uni )
	DecodeAddSpace( attr );
      else
	DecodeAddControl( (byte)uni );
    } else if( uni < (ic_t)DEL )
      DecodeAddIchar( ASCII, uni, attr );
    else {
      DecodeAddControl( (byte)uni );
    }
  } else
    DecodeAddIchar( UNICODE, uni, attr );
}

private char DecodeBase64( byte attr, byte ch )
{
  unsigned long acc;
  int val;
  ic_t uni;

  do {
    if( 0 <= (val = base64[ (int)ch ]) ){
      acc = val << 26;
      GetChar( ch );
      if( 0 <= (val = base64[ (int)ch ]) ){
	acc |= val << 20;
	GetChar( ch );
	if( 0 <= (val = base64[ (int)ch ]) ){
	  acc |= val << 14;
	  uni = acc >> 16;
	  DecodeAddUnicode( attr, uni );
	  acc = ( acc & 0x0000ffffL ) << 16;
	  GetChar( ch );
	  if( 0 <= (val = base64[ (int)ch ]) ){
	    acc |= val << 24;
	    GetChar( ch );
	    if( 0 <= (val = base64[ (int)ch ]) ){
	      acc |= val << 18;
	      GetChar( ch );
	      if( 0 <= (val = base64[ (int)ch ]) ){
		acc |= val << 12;
		uni = acc >> 16;
		DecodeAddUnicode( attr, uni );
		acc = ( acc & 0x0000ffffL ) << 16;
		GetChar( ch );
		if( 0 <= (val = base64[ (int)ch ]) ){
		  acc |= val << 22;
		  GetChar( ch );
		  if( 0 <= (val = base64[ (int)ch ]) ){
		    acc |= val << 16;
		    uni = acc >> 16;
		    DecodeAddUnicode( attr, uni );
		    GetChar( ch );
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  } while( val >= 0 );

  return ch;
}

public void DecodeUTF( state_t *state, byte codingSystem )
{
  byte ch;
  ic_t uni;

  for( ; ; ){
    GetChar( ch );
utfContinue:
    if( ch <= SP ){
      if( SP == ch )
	DecodeAddSpace( state->attr );
      else if( ESC == ch ){
	if( FALSE == DecodeEscape( state ) )
	  break;
      } else if( HT == ch )
	DecodeAddTab( state->attr );
      else if( BS == ch )
	DecodeAddBs();
      else
	DecodeAddControl( ch );
    } else {
      if( ch < (ic_t)DEL ){
	if( UTF_8 == codingSystem ){
	  DecodeAddIchar( ASCII, (ic_t)ch, state->attr );
	} else {
	  /* UTF_7 */
	  if( '+' != ch ){
	    DecodeAddIchar( ASCII, (ic_t)ch, state->attr );
	  } else {
	    /* '+' == ch */
	    GetChar( ch );
	    if( '-' == ch ){
	      DecodeAddIchar( ASCII, (ic_t)'+', state->attr );
	    } else {
	      ch = DecodeBase64( state->attr, ch );
	      if( SHIGH == SIDX || LF == ch || CR == ch )
		return;
	      else if( '-' == ch )
		continue;
	      else {
		decoding_penalty++;
		goto utfContinue;
	      }
	    }
	  }
	}
      } else if( 0xc0 == ( 0xe0 & ch ) ){
	/* 1st of 2 */
	uni = (ic_t)( 0x1f & ch ) << 6;
	GetChar( ch );
	if( 0x80 == ( 0xc0 & ch ) ){
	  /* trailor */
	  uni |= (ic_t)( 0x3f & ch );
	  DecodeAddUnicode( state->attr, uni );
	}
      } else if( 0xe0 == ( 0xf0 & ch ) ){
	/* 1st of 3 */
	uni = (ic_t)( 0x0f & ch ) << 12;
	GetChar( ch );
	if( 0x80 == ( 0xc0 & ch ) ){
	  /* trailor */
	  uni |= (ic_t)( 0x3f & ch ) << 6;
	  GetChar( ch );
	  if( 0x80 == ( 0xc0 & ch ) ){
	    /* trailor */
	    uni |= (ic_t)( 0x3f & ch );
	    DecodeAddUnicode( state->attr, uni );
	  }
	}
      } else {
	decoding_penalty++;
	DecodeAddControl( ch );
      }
    }
  }
}

typedef enum { ASCII_STATE, PENDING_NONE, PENDING_4, PENDING_2 } utf7_state_t;

private void EncodeUTF7PendingBit( utf7_state_t stat, int attr, ic_t uni )
{
  if( PENDING_NONE != stat )
    EncodeAddCharAbsolutely( attr, base64char[ uni ] );
  EncodeAddCharAbsolutely( attr, (ic_t)'-' );
}

public void EncodeUTF7( i_str_t *istr, int head, int tail,
		       byte codingSystem, boolean_t binary )
{
  int idx, attr, lastAttr;
  ic_t ic, uni = 0;
  byte cset;
  utf7_state_t stat;

  attr = lastAttr = 0;
  stat = ASCII_STATE;

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    ic = istr[ idx ].c;
    attr = (int)istr[ idx ].attr << 8;
    if( attr != lastAttr ){
      if( ASCII_STATE != stat ){
	EncodeUTF7PendingBit( stat, lastAttr, uni );
	stat = ASCII_STATE;
      }
      lastAttr = attr;
    }
    if( cset < PSEUDO ){
      if( ASCII == cset ){
	if( ASCII_STATE != stat ){
	  EncodeUTF7PendingBit( stat, lastAttr, uni );
	  stat = ASCII_STATE;
	}
	if( (ic_t)'+' == ic ){
	  EncodeAddCharAbsolutely( attr, ic );
	  EncodeAddCharAbsolutely( attr, (ic_t)'-' );
	} else {
	  EncodeAddChar( attr, ic );
	}
      } else {
	if( UNICODE != cset )
	  ic = RevUNI( ic, &cset );
	if( ic < 0x80U ){
	  if( ASCII_STATE != stat ){
	    EncodeUTF7PendingBit( stat, lastAttr, uni );
	    stat = ASCII_STATE;
	  }
	  if( (ic_t)'+' == ic ){
	    EncodeAddCharAbsolutely( attr, ic );
	    EncodeAddCharAbsolutely( attr, (ic_t)'-' );
	  } else {
	    EncodeAddChar( attr, ic );
	  }
	} else {
	  switch( stat ){
	  case ASCII_STATE:
	    EncodeAddChar( attr, (ic_t)'+' );
	  case PENDING_NONE:
	    EncodeAddChar( attr, base64char[ ic >> 10 ] );
	    EncodeAddChar( attr, base64char[ 0x3f & ( ic >> 4 ) ] );
	    uni = ( ic & 0x0f ) << 2;
	    stat = PENDING_4;
	    break;
	  case PENDING_4:
	    uni |= ic >>14;
	    EncodeAddChar( attr, base64char[ uni ] );
	    EncodeAddChar( attr, base64char[ 0x3f & ( ic >> 8 ) ] );
	    EncodeAddChar( attr, base64char[ 0x3f & ( ic >> 2 ) ] );
	    uni = ( ic & 0x03 ) << 4;
	    stat = PENDING_2;
	    break;
	  case PENDING_2:
	    uni |= ic >> 12;
	    EncodeAddChar( attr, base64char[ uni ] );
	    EncodeAddChar( attr, base64char[ 0x3f & ( ic >> 6 ) ] );
	    EncodeAddChar( attr, base64char[ 0x3f & ic ] );
	    stat = PENDING_NONE;
	  }
	}
      }
    } else {
      if( ASCII_STATE != stat ){
	EncodeUTF7PendingBit( stat, lastAttr, uni );
	stat = ASCII_STATE;
      }
      if( FALSE == EncodeAddPseudo( attr, ic, cset, binary ) ){
	break;
      }
    }
  }
  if( ASCII_STATE != stat ){
    EncodeUTF7PendingBit( stat, lastAttr, uni );
  }
}

public void EncodeUTF8( i_str_t *istr, int head, int tail,
		       byte codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  byte cset;

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    ic = istr[ idx ].c;
    attr = (int)istr[ idx ].attr << 8;
    if( cset < PSEUDO ){
      if( ASCII == cset ){
	EncodeAddChar( attr, ic );
      } else {
	if( UNICODE != cset )
	  ic = RevUNI( ic, &cset );
	if( ic < 0x80U ){
	  EncodeAddChar( attr, ic );
	} else if( ic < 0x0800U ){
	  EncodeAddChar( attr, 0xc0 | ( ic >> 6 ) );
	  EncodeAddChar( attr, 0x80 | ( 0x3f & ic ) );
	} else {
	  EncodeAddChar( attr, 0xe0 | ( ic >> 12 ) );
	  EncodeAddChar( attr, 0x80 | ( 0x3f & ( ic >> 6 ) ) );
	  EncodeAddChar( attr, 0x80 | ( 0x3f & ( ic ) ) );
	}
      }
    } else if( FALSE == EncodeAddPseudo( attr, ic, cset, binary ) ){
      break;
    }
  }
}
