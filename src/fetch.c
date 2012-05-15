/*
 * fetch.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: fetch.c,v 1.7 2004/01/05 07:30:15 nrt Exp $
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

/*
 * 現在のファイルポインタから 1行をロードし,
 * デコードして (istr に変換して) 物理行に『畳み込み』,
 * サーチが設定されていればサーチを実行する.
 * すべての結果はキャッシュに書き込む.
 *
 * 読み込むべきファイル内容がなければ FALSE を返す.
 *
 * 物理行は fetch によって『生成』される.
 */

private boolean_t LineDecode( file_t *f, int page, line_t *line )
{
  int idx, w, width, hptr, line_size;
  boolean_t simple;
  byte *str;

  str = FileLoadLine( f, &idx, &simple );
  if( 0 == idx )
    return FALSE;

  line_size = idx / 16 + 1;	/* physical lines per logical line */

  if( line->istr )
    IstrFree( line->istr );

  if( line->head )
    free( line->head );
  line->head = (head_t *)Malloc( sizeof( head_t ) * line_size );

  if( TRUE == simple ){
    line->istr = DecodeSimple( IstrAlloc( page, idx + 1 ),
			      str, &idx );
    FileFreeLine( f );

    hptr = 0;
    w = f->width;
    width = idx;
    while( w < idx ){
      if( hptr >= line_size - 1 ){
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
    line->istr = Decode( IstrAlloc( page, idx + 1 ),
			f->inputCodingSystem, str, &idx );
    FileFreeLine( f );

    hptr = 0;
    width = 0;
    for( idx = 0 ; NOSET != line->istr[ idx ].charset ; idx++ ){
      if( HTAB == line->istr[ idx ].charset ){
	w = HTAB_WIDTH - width % HTAB_WIDTH;
	line->istr[ idx ].c &= 0x00ff;
	line->istr[ idx ].c |= ( w << 8 );
	if( width + w == f->width ){
	  if( hptr >= line_size - 1 ){
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
	if( hptr >= line_size - 1 ){
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
  if( NULL == line->head ){
    fprintf( stderr, "lv: realloc() failed\n" );
    exit( -1 );
  }

  if( NULL != f->find.pattern )
    (*find_func)( line );

  return TRUE;
}

/*
 * 指定されたファイルポインタから 1ページをロードする.
 *
 * 読み出し中に EOF を見つけたら, そこでページのロードを中断し,
 * 行数をキャッシュに格納する.
 */

private void PageLoad( file_t *f, int block, long ptr )
{
  int i;

  if( fseek( f->fp, ptr, SEEK_SET ) )
    perror( "PageLoad()" ), exit( -1 );

  f->eof = FALSE;
  for( i = 0 ; i < LV_PAGE_SIZE ; i++ ){
    if( TRUE == f->eof )
      break;
    if( FALSE == LineDecode( f, block, &f->page[ block ].line[ i ] ) )
      break;
  }

  f->page[ block ].lines = i;
}

/*
 * 指定したページがページキャッシュ内にあることを保証する.
 * ページキャッシュにない場合, ファイルからキャッシュへロードしなおす.
 */

public boolean_t FetchLine( file_t *f, unsigned int segment, int offset )
{
  int line, block;

  if( segment >= f->lastSegment ){
    if( FALSE == FileStretch( f, segment ) )
      return FALSE;
  }

  block = Block( segment );

  if( FALSE == f->used[ block ] || segment != f->page[ block ].segment ){
    if( TRUE == f->used[ block ] ){
      for( line = 0 ; line < LV_PAGE_SIZE ; line++ ){
	if( NULL != f->page[ block ].line[ line ].head ){
	  free( f->page[ block ].line[ line ].head );
	  f->page[ block ].line[ line ].head = NULL;
	}
	if( NULL != f->page[ block ].line[ line ].istr ){
	  IstrFree( f->page[ block ].line[ line ].istr );
	  f->page[ block ].line[ line ].istr = NULL;
	}
      }
      IstrFreeZone( block );
    }
    PageLoad( f, block, f->slot[ Frame( segment ) ][ Slot( segment ) ] );
    f->page[ block ].segment = segment;
    f->used[ block ] = TRUE;
  }

  if( f->page[ block ].lines <= offset )
    return FALSE;

  return TRUE;
}
