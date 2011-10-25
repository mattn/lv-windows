/*
 * raw.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <import.h>
#include <decode.h>
#include <encode.h>
#include <begin.h>
#include <raw.h>

public void DecodeRaw( state_t *state, char codingSystem )
{
  char ch, c[ ICHAR_WIDTH ];

  for( ; ; ){
    GetChar( ch );
    if( CR == ch )
      continue;
    else if( SP == ch ){
      DecodeAddSpace( NULL );
    } else if( HT == ch ){
      DecodeAddTab( NULL );
    } else if( ch < SP || ch >= DEL ){
      DecodeAddControl( ch );
    } else {
      c[ 0 ] = ch;
      DecodeAddChar( ASCII, c, NULL );
    }
  }
}

public void EncodeRaw( i_str_t *istr, int head, int tail,
		      char codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  char cset;

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    ic = istr[ idx ].c;
    attr = (int)istr[ idx ].attr << 8;
    if( cset < PSEUDO ){
      if( TRUE == iTable[ (int)cset ].multi ){
	EncodeAddChar( attr, MakeByte1( ic ) );
	if( 2 == IcharWidth( cset, ic ) )
	  EncodeAddChar( attr, MakeByte2( ic ) );
      } else {
	EncodeAddChar( attr, ic );
      }
    } else if( FALSE == EncodeAddPseudo( attr, ic, cset, binary ) ){
      break;
    }
  }
}
