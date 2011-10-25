/*
 * fetch.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef UNIX
#include <unistd.h>
#endif /* UNIX */

#include <import.h>
#include <decode.h>
#include <find.h>
#include <uty.h>
#include <begin.h>
#include <fetch.h>

private boolean_t LineDecode( file_t *f, line_t *line )
{
  int idx, w, width, hptr;
  boolean_t simple;

  simple = FileLoadLine( f, &idx );
  if( 0 == idx )
    return FALSE;

  if( line->istr )
    IstrFree( line->istr );

  if( line->head )
    free( line->head );
  line->head = (head_t *)Malloc( sizeof( head_t ) * LINE_SIZE );

  if( TRUE == simple ){
    line->istr = DecodeSimple( load_str, &idx );

    hptr = 0;
    w = f->width;
    width = idx;
    while( w < idx ){
      if( hptr >= LINE_SIZE - 1 ){
	width = f->width;
	idx = w;
	line_truncated = TRUE;
	break;
      }
      line->head[ hptr ].width = f->width;
      line->head[ hptr++ ].ptr = w;
      width = idx - w;
      w += f->width;
    }
  } else {
    line->istr = Decode( f->inputCodingSystem, load_str, &idx );

    hptr = 0;
    width = 0;
    for( idx = 0 ; NOSET != line->istr[ idx ].charset ; idx++ ){
      if( HTAB == line->istr[ idx ].charset ){
	w = HTAB_WIDTH - width % HTAB_WIDTH;
	line->istr[ idx ].c &= 0x00ff;
	line->istr[ idx ].c |= ( w << 8 );
	if( width + w == f->width ){
	  if( hptr >= LINE_SIZE - 1 ){
	    idx++;
	    line_truncated = TRUE;
	    break;
	  }
	  line->head[ hptr ].width = width;
	  line->head[ hptr++ ].ptr = idx + 1;
	  width = 0;
	  continue;
	}
      } else {
	w = IcharWidth( line->istr[ idx ].charset, line->istr[ idx ].c );
      }
      if( width + w > f->width ){
	if( hptr >= LINE_SIZE - 1 ){
	  line_truncated = TRUE;
	  break;
	}
	line->head[ hptr ].width = width;
	line->head[ hptr++ ].ptr = idx;
	width = w;
      } else {
	width += w;
      }
    }
  }

  line->head[ hptr ].width = width;
  line->head[ hptr++ ].ptr = idx;
  line->heads = hptr;

  line->head = (head_t *)realloc( line->head, sizeof( head_t ) * hptr );

  if( NULL != f->find.pattern )
    (*find_func)( line );

  return TRUE;
}

private void PageLoad( file_t *f, int block, long ptr )
{
  int i;

  if( fseek( f->fp, ptr, SEEK_SET ) )
    perror( "PageLoad()" ), exit( -1 );

  f->eof = FALSE;
  for( i = 0 ; i < PAGE_SIZE ; i++ ){
    if( TRUE == f->eof )
      break;
    if( FALSE == LineDecode( f, &f->page[ block ].line[ i ] ) )
      break;
  }

  f->page[ block ].lines = i;
}

public boolean_t FetchLine( file_t *f, unsigned int segment, int offset )
{
  int block;

  if( segment >= f->lastSegment )
    if( FALSE == FileStretch( f, segment ) )
      return FALSE;

  block = Block( segment );

  if( FALSE == f->used[ block ] || segment != f->page[ block ].segment ){
    PageLoad( f, block, f->slot[ segment ].ptr );
    f->page[ block ].segment = segment;
    f->used[ block ] = TRUE;
  }

  if( f->page[ block ].lines <= offset )
    return FALSE;

  return TRUE;
}
