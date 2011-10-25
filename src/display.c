/*
 * display.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <import.h>
#include <screen.h>
#include <console.h>
#include <encode.h>
#include <fetch.h>
#include <position.h>
#include <begin.h>
#include <display.h>

#define ScreenGetTop( f, seg, blk, off, phy )				\
  PositionGet( (f)->screen.top, seg, blk, off, phy )

#define ScreenGetBot( f, seg, blk, off, phy )				\
  PositionGet( (f)->screen.bot, seg, blk, off, phy )

private void LineEncode( file_t *f, int blk, int off, int phy )
{
  line_t *line;
  int head, tail;

  line = &f->page[ blk ].line[ off ];
  head = LineHead( line, phy );
  tail = head + LineLength( line, phy );

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
  for( i = 0 ; i < f->screen.lines ; i++ ){
    LineEncode( f, blk, off, phy );
    ConsoleSetCur( 0, i );
    ConsoleClearRight();
    ConsolePrintsStr( encode_str, encode_length );
    PositionInc( f, seg, blk, off, phy );
  }
  for( i = f->screen.lines ; i < f->height ; i++ ){
    ConsoleSetCur( 0, i );
    ConsoleClearRight();
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
