/*
 * iso2022.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdio.h>

#include <import.h>
#include <decode.h>
#include <escape.h>
#include <begin.h>
#include <iso2022.h>

public void DecodeISO2022( state_t *state, char codingSystem )
{
  int region;
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
      else if( EM == ch )
	state->sset = G2;
      else
	DecodeAddControl( ch );
    } else {
      if( NULL != state->sset ){
	/*
	 * single shifted character
	 */
	if( EUC_TAIWAN == codingSystem && G2 == state->sset ){
	  charset = CNS_1 + ( ch - 0xa1 );
	  if( charset < CNS_1 || charset > CNS_7 ){
	    state->sset = 0;
	    continue;
	  }
	  GetChar( ch );
	} else {
	  charset = SSET;
	}
	ch &= 0x7f;	/* for EUC */
	state->sset = 0;
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
	  ch &= 0x7f;	/* for EUC */
	  if( !IsGraphicChar( charset, ch ) )
	    continue;
	  c[ 1 ] = ch;
	}
      } else {
	if( 0x80 & ch ){
	  if( SS2 == ch ){
	    state->sset = G2;
	    continue;
	  } else if( SS3 == ch ){
	    state->sset = G3;
	    continue;
	  }
	  region = GR;
	  ch &= 0x7f;
	} else {
	  region = GL;
	}
	charset = CSET( region );
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
