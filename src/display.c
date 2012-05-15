/*
 * display.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: display.c,v 1.4 2003/11/13 03:08:19 nrt Exp $
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
#include <screen.h>
#include <console.h>
#include <encode.h>
#include <fetch.h>
#include <position.h>
#include <begin.h>
#include <display.h>

/*
 * ページ内容をエンコードして画面に出力する.
 *
 * screen.c でロードしたページ内容を表示する.
 * (ただし, display.c からも動的にページをメモリにロードしている).
 */

#define ScreenGetTop( f, seg, blk, off, phy )				\
  PositionGet( (f)->screen.top, (seg), (blk), (off), (phy) )

#define ScreenGetBot( f, seg, blk, off, phy )				\
  PositionGet( (f)->screen.bot, (seg), (blk), (off), (phy) )

private void LineEncode( file_t *f, int blk, int off, int phy )
{
  line_t *line;
  int head, tail;

  line = &f->page[ blk ].line[ off ];
  head = LineHead( line, phy );
  tail = head + LineLength( line, phy );

  encode_length = CODE_SIZE;
  Encode( line->istr, head, tail,
	 f->outputCodingSystem, FALSE,
	 encode_str, &encode_length );
}

public void DisplayFull( file_t *f )
{
  int i;
  int seg, blk, off, phy;

  ConsoleOffCur();

  ScreenGetTop( f, seg, blk, off, phy );

  ConsoleClearScreen();

  for( i = 0 ; i < f->screen.lines ; i++ ){
    LineEncode( f, blk, off, phy );
    if( 0 == i || TRUE == carefully_divide || 0 == phy ){
      ConsoleSetCur( 0, i );
    }
    ConsolePrintsStr( encode_str, encode_length );
    PositionInc( f, seg, blk, off, phy );
  }
  for( i = f->screen.lines ; i < f->height ; i++ ){
    ConsoleSetCur( 0, i );
    ConsolePrint( '~' );
  }

  ConsoleOnCur();

  f->dirty = FALSE;
}

public void DisplayTop( file_t *f, int arg )
{
  int seg, blk, off, phy;
  int i;

  if( TRUE == no_scroll || arg > f->height || f->screen.lines < f->height ){
    DisplayFull( f );
    return;
  }

  if( arg == f->height ){
    if( FALSE == smooth_paging ){
      DisplayFull( f );
      return;
    }
    ScreenGetBot( f, seg, blk, off, phy );
  } else {
    ScreenGetTop( f, seg, blk, off, phy );
    for( i = 1 ; i < arg ; i++ )
      PositionInc( f, seg, blk, off, phy );
  }

  ConsoleOffCur();

  for( i = 0 ; i < arg ; i++ ){
    LineEncode( f, blk, off, phy );
    ConsoleSetCur( 0, HEIGHT - 1 );
    ConsoleClearRight();
    ConsoleSetCur( 0, 0 );
    ConsoleScrollDown();
    ConsolePrintsStr( encode_str, encode_length );
    PositionDec( f, seg, blk, off, phy );
  }

  ConsoleOnCur();
}

public void DisplayBot( file_t *f, int arg )
{
  int seg, blk, off, phy;
  int i;

  if( TRUE == no_scroll || arg > f->height || f->screen.lines < f->height ){
    DisplayFull( f );
    return;
  }

  if( arg == f->height ){
    if( FALSE == smooth_paging ){
      DisplayFull( f );
      return;
    }
    ScreenGetTop( f, seg, blk, off, phy );
  } else {
    ScreenGetBot( f, seg, blk, off, phy );
    for( i = 1 ; i < arg ; i++ )
      PositionDec( f, seg, blk, off, phy );
  }

  ConsoleOffCur();

  for( i = 0 ; i < arg ; i++ ){
    LineEncode( f, blk, off, phy );
    ConsoleSetCur( 0, 0 );
    ConsoleScrollUp();
    ConsoleSetCur( 0, f->height - 1 );
    ConsoleClearRight();
    ConsolePrintsStr( encode_str, encode_length );
    PositionInc( f, seg, blk, off, phy );
  }

  ConsoleOnCur();
}
