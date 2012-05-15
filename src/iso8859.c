/*
 * iso8859.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: iso8859.c,v 1.3 2003/11/13 03:08:19 nrt Exp $
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
#include <encode.h>
#include <unimap.h>
#include <begin.h>
#include <iso8859.h>

/*
 * iso-8859-*
 *
 * one pre-designated 8bit charset uses G1.
 */
public void EncodeISO8859( i_str_t *istr, int head, int tail,
			  byte codingSystem, boolean_t binary )
{
  int idx, attr;
  ic_t ic;
  byte cset, csetG1 = ASCII;

  csetG1 = cTable[ (int)codingSystem ].state.cset[ G1 ];

  for( idx = head ; idx < tail ; idx++ ){
    cset = istr[ idx ].charset;
    ic = istr[ idx ].c;
    attr = (int)istr[ idx ].attr << 8;
#ifndef MSDOS /* IF NOT DEFINED */
    if( UNICODE == cset )
      ic = UNItoISO8859( ic, &cset, codingSystem );
#endif /* MSDOS */
    if( cset < PSEUDO ){
      if( ASCII == cset ){
	EncodeAddChar( attr, ic );
      } else if( csetG1 == cset ){
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
