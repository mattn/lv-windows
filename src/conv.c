/*
 * conv.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdlib.h>

#include <import.h>
#include <file.h>
#include <encode.h>
#include <decode.h>
#include <uty.h>
#include <begin.h>
#include <conv.h>

public void Conv( file_t *f )
{
  int idx, segment, offset;
  boolean_t simple;
  i_str_t *istr;

  segment = 0;
  istr = NULL;

  for( f->eof = FALSE ; FALSE == f->eof ; segment++ ){
    if( FALSE == FileSeek( f, segment ) )
      return;

    for( offset = 0 ; offset < PAGE_SIZE && FALSE == f->eof ; offset++ ){
      simple = FileLoadLine( f, &idx );
      if( 0 == idx )
	return;

      if( NULL != istr )
	IstrFree( istr );

      if( TRUE == simple )
	istr = DecodeSimple( load_str, &idx );
      else
	istr = Decode( f->inputCodingSystem, load_str, &idx );

      Encode( istr, 0, idx,
	     f->outputCodingSystem, TRUE,
	     encode_str, &encode_length );

      for( idx = 0 ; idx < encode_length ; idx++ )
	putchar( 0xff & encode_str[ idx ] );

      putchar( LF );
    }
  }
}
