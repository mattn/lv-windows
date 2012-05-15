/*
 * conv.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: conv.c,v 1.7 2003/11/13 03:08:19 nrt Exp $
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

#include <stdlib.h>

#include <import.h>
#include <file.h>
#include <encode.h>
#include <decode.h>
#include <uty.h>
#include <find.h>
#include <begin.h>
#include <conv.h>

public boolean_t Conv( file_t *f, boolean_t showFileName )
{
  int len;
  boolean_t simple;
  long lineNumber = 0;
  boolean_t flagMatchedResult = FALSE;
  boolean_t flagMatched = FALSE;
  str_t *encoding_space = NULL;
  byte *str;
  i_str_t *istr = NULL;
  int i, gc_count = 0;

  for( f->eof = FALSE ; FALSE == f->eof ; ){
    str = FileLoadLine( f, &len, &simple );
    if( 0 == len )
      return flagMatchedResult;

    if( NULL != istr )
      IstrFree( istr );

    if( ++gc_count > 64 ){
      gc_count = 0;
      IstrFreeZone( ZONE_PAGE0 );
    }

    if( TRUE == simple )
      istr = DecodeSimple( IstrAlloc( ZONE_PAGE0, len + 1 ),
			  str, &len );
    else
      istr = Decode( IstrAlloc( ZONE_PAGE0, len + 1 ),
		    f->inputCodingSystem, str, &len );

    if( TRUE == grep_mode ){
      lineNumber++;

      flagMatched = (*find_only_func)( istr );

      if( TRUE == flagMatched ){
	if( TRUE == grep_inverted )
	  flagMatched = FALSE;
	else {
	  flagMatchedResult = TRUE;
	  if( TRUE == showFileName )
	    printf( "%s:", f->fileName );
	  if( TRUE == line_number )
	    printf( "%ld:", lineNumber );
	}
      } else {
	if( TRUE == grep_inverted ){
	  flagMatched = TRUE;
	  flagMatchedResult = TRUE;
	  if( TRUE == showFileName )
	    printf( "%s:", f->fileName );
	  if( TRUE == line_number )
	    printf( "%ld:", lineNumber );
	}
      }
    }

    if( TRUE == flagMatched || FALSE == grep_mode ){
      if( TRUE == simple ){
	for( i = 0 ; i < len ; i++ )
	  putchar( str[ i ] );
      } else {
	if( len > (STR_SIZE >> 2) ){
	  encode_length = (len << 2) + CODE_EXTRA_LEN;
	  encoding_space = (str_t *)Malloc( encode_length * sizeof( str_t ) );
	  Encode( istr, 0, len,
		 f->outputCodingSystem, TRUE,
		 encoding_space, &encode_length );
	  for( len = 0 ; len < encode_length ; len++ )
	    putchar( 0xff & encoding_space[ len ] );
	  free( encoding_space );
	} else {
	  encode_length = CODE_SIZE;
	  Encode( istr, 0, len,
		 f->outputCodingSystem, TRUE,
		 encode_str, &encode_length );
	  for( len = 0 ; len < encode_length ; len++ )
	    putchar( 0xff & encode_str[ len ] );
	}
      }
    }
  }

  return flagMatchedResult;
}
