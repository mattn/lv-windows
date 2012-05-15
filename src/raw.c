/*
 * raw.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: raw.c,v 1.6 2004/01/05 07:23:29 nrt Exp $
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
#include <encode.h>
#include <begin.h>
#include <raw.h>

public void DecodeRaw( state_t *state, byte codingSystem )
{
  byte ch, c[ ICHAR_WIDTH ];

  for( ; ; ){
    GetChar( ch );
    if( SP == ch ){
      DecodeAddSpace( 0 );
    } else if( HT == ch ){
      DecodeAddTab( 0 );
    } else if( ch < SP || ch >= DEL ){
      DecodeAddControl( ch );
    } else {
      c[ 0 ] = ch;
      DecodeAddChar( ASCII, c, 0 );
    }
  }
}

public void EncodeRaw( i_str_t *istr, int head, int tail,
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
