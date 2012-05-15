/*
 * escape.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: escape.c,v 1.5 2003/11/13 03:08:19 nrt Exp $
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
#include <decode.h>
#include <console.h>
#include <begin.h>
#include <escape.h>

public boolean_t DecodeEscape( state_t *state )
{
  boolean_t multi;
  boolean_t set94 = TRUE;
  boolean_t omitted;
  boolean_t unknownSequence;
  byte charset, ic, ch, attr, color;
  int g = 0, index, arg;

  /*
   * escape sequence
   */

  multi = FALSE;
  omitted = FALSE;
  unknownSequence = FALSE;

  GetCharRet( ch );

  switch( ch ){
  case '[':
    if( FALSE == allow_ansi_esc || TRUE == binary_decode )
      break;

    /*
     * possibly, ANSI character attribute sequence
     */

    index = SIDX;

    do {
      GetCharRet( ch );
      if( !( ( ch >= '0' && ch <= '9' ) || ';' == ch ) )
	break;
    } while( 'm' != ch );

    SIDX = index;

    if( 'm' != ch ){
      /* invalid ANSI sequence */
      ch = '[';
      break;
    }

    arg = 0;
    attr = color = ATTR_NULL;
    do {
      GetCharRet( ch );

      if( ch >= '0' && ch <= '9' ){
	arg = 10 * arg + ch - '0';
      } else if( ';' == ch || 'm' == ch ){
	switch( arg ){
	case 0: /* ignore it */ break;
	case 1: attr |= ATTR_HILIGHT; break;
	case 4: attr |= ATTR_UNDERLINE; break;
	case 5: attr |= ATTR_BLINK; break;
	case 7: attr |= ATTR_REVERSE; break;
	default:
	  if( arg >= 30 && arg <= 37 )
	    color = (byte)( arg - 30 );
	  else if( arg >= 40 && arg <= 47 ){
	    color = (byte)( arg - 40 );
	    attr |= ATTR_REVERSE;
	  }
	}
	arg = 0;
      }
    } while( 'm' != ch );

    state->attr = attr | color;

    return TRUE;
  case ' ': /* announcer */
    GetCharRet( ch );
    /*
     * current implementation just ignores it.
     */
    return TRUE;
  case '&': /* KOUSIN of registered charset */
    GetCharRet( ch );
    /*
     * current implementation just ignores it.
     */
    return TRUE;
  case '~': /* LS1R */	state->gset[ GR ] = G1; return TRUE;
  case 'n': /* LS2 */	state->gset[ GL ] = G2; return TRUE;
  case '}': /* LS2R */	state->gset[ GR ] = G2; return TRUE;
  case 'o': /* LS3 */	state->gset[ GL ] = G3; return TRUE;
  case '|': /* LS3R */	state->gset[ GR ] = G3; return TRUE;
  case 'N': /* SS2 */	state->sset = G2; return TRUE;
  case 'O': /* SS3 */	state->sset = G3; return TRUE;
  }

  if( ch < SP || IsFchar( ch ) ){
    /*
     * unknown ESC F sequence
     */
    DecodeAddControl( ESC );
    DecodeAddControl( ch );
    return TRUE;
  }

  if( '$' == ch ){
    multi = TRUE;
    GetCharRet( ch );
  }

  ic = ch;
  if( ch >= '(' && ch <= '/' ){
    switch( ch ){
    case '(': set94 = TRUE;  g = 0; break;
    case ')': set94 = TRUE;  g = 1; break;
    case '*': set94 = TRUE;  g = 2; break;
    case '+': set94 = TRUE;  g = 3; break;
    case ',': set94 = FALSE; g = 0; break;	/* MULE specific */
    case '-': set94 = FALSE; g = 1; break;
    case '.': set94 = FALSE; g = 2; break;
    case '/': set94 = FALSE; g = 3; break;
    }
  } else {
    if( '@' == ch || 'A' == ch || 'B' == ch ){
      set94 = TRUE;
      g = 0;
      omitted = TRUE;
    } else {
      unknownSequence = TRUE;
    }
  }

  if( FALSE == unknownSequence && FALSE == omitted ){
    GetCharRet( ch );
  }

  if( TRUE == unknownSequence
     || NOSET == (charset = ItableLookup( ch, multi, set94 )) ){
    /*
     * unknown ESC I ... F sequence
     */
    DecodeAddControl( ESC );
    if( TRUE == multi ){
      DecodeAddControl( '$' );
    }
    DecodeAddControl( ic );
    if( FALSE == unknownSequence && FALSE == omitted ){
      DecodeAddControl( ch );
    } else {
      ch = ic;
    }
    while( ! IsFchar( ch ) ){
      GetCharRet( ch );
      DecodeAddControl( ch );
    }
    return TRUE;
  }

  if( TRUE == allow_unify ){
    if( charset == X0201ROMAN )
      charset = ASCII;
    else if( C6226 == charset )
      charset = X0208;
  }

  state->cset[ g ] = charset;

  return TRUE;
}
