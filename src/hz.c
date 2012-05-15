/*
 * hz.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: hz.c,v 1.3 2003/11/13 03:08:19 nrt Exp $
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
#include <encode.h>
#include <unimap.h>
#include <big5.h>
#include <begin.h>
#include <hz.h>

public void EncodeHz( i_str_t *istr, int head, int tail,
		     byte codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  byte cset;
  boolean_t asciiMode = TRUE;

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    ic = istr[ idx ].c;
    attr = (int)istr[ idx ].attr << 8;
    if( BIG5 == cset )
      ic = BIG5toCNS( ic, &cset );
#ifndef MSDOS /* IF NOT DEFINED */
    else if( UNICODE == cset )
      ic = UNItoGB( ic, &cset );
#endif /* MSDOS */
    if( cset < PSEUDO ){
      if( ASCII == cset ){
	if( FALSE == asciiMode ){
	  asciiMode = TRUE;
	  EncodeAddChar( attr, (ic_t)'~' );
	  EncodeAddChar( attr, (ic_t)'}' );
	}
	EncodeAddChar( attr, ic );
	if( (ic_t)'~' == ic )
	  EncodeAddChar( attr, (ic_t)'~' );
      } else if( GB2312 == cset ){
	if( TRUE == asciiMode ){
	  asciiMode = FALSE;
	  EncodeAddChar( attr, '~' );
	  EncodeAddChar( attr, '{' );
	}
	EncodeAddChar( attr, MakeByte1( ic ) );
	EncodeAddChar( attr, MakeByte2( ic ) );
      } else {
	if( FALSE == EncodeAddInvalid( attr, ic, cset ) )
	  break;
      }
    } else if( FALSE == EncodeAddPseudo( attr, ic, cset, binary ) ){
      break;
    }
  }
}
