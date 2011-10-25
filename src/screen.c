/*
 * screen.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdlib.h>

#include <import.h>
#include <position.h>
#include <fetch.h>
#include <console.h>
#include <begin.h>
#include <screen.h>

#define ScreenSetBoth( f, seg, blk, off, phy )				\
{									\
  PositionSet( (f)->screen.top, seg, blk, off, phy );			\
  PositionSet( (f)->screen.bot, seg, blk, off, phy );			\
}

#define ScreenDecTop( f )						\
  PositionDec( (f), (f)->screen.top.seg, (f)->screen.top.blk, (f)->screen.top.off, (f)->screen.top.phy )

#define ScreenDecBot( f )						\
  PositionDec( (f), (f)->screen.bot.seg, (f)->screen.bot.blk, (f)->screen.bot.off, (f)->screen.bot.phy )

#define ScreenIncTop( f )						\
  PositionInc( (f), (f)->screen.top.seg, (f)->screen.top.blk, (f)->screen.top.off, (f)->screen.top.phy )

#define ScreenIncBot( f )						\
  PositionInc( (f), (f)->screen.bot.seg, (f)->screen.bot.blk, (f)->screen.bot.off, (f)->screen.bot.phy )

/*
 * screen position functions
 */

public unsigned int ScreenPrev( file_t *f, int physical )
{
  int i;

  for( i = 0 ; i < physical ; i++ ){
    ScreenDecTop( f );
    f->screen.lines++;
    if( f->screen.lines > f->height ){
      ScreenDecBot( f );
      f->screen.lines--;
    }
  }
  if( i < physical )
    f->top = TRUE;
  else
    f->top = FALSE;
  if( f->screen.lines < f->height )
    f->bottom = TRUE;
  else
    f->bottom = FALSE;
  f->find.pos = f->screen.top;

  return i;
}

public unsigned int ScreenNext( file_t *f, int physical )
{
  int i;

  for( i = 0 ; i < physical ; i++ ){
    ScreenIncBot( f );
    f->screen.lines++;
    if( f->screen.lines > f->height ){
      ScreenIncTop( f );
      f->screen.lines--;
    }
  }
  if( i < physical ){
    f->top = FALSE;
    f->bottom = TRUE;
  } else {
    f->top = FALSE;
    f->bottom = FALSE;
  }
  f->find.pos = f->screen.top;

  return i;
}

public boolean_t ScreenBot( file_t *f )
{
  int i;
  unsigned int seg;
  int blk, off, phy;

  if( FALSE == f->done ){
    ConsoleEnableInterrupt();
    FileStretch( f, PAGE_SIZE * SLOT_SIZE );
    ConsoleDisableInterrupt();
  }

  if( FALSE == FetchLine( f, f->lastSegment - 1, 0 ) ){
    /*
     * null file
     */
    f->top = FALSE;
    f->bottom = TRUE;
    f->screen.lines = 0;
    ScreenSetBoth( f, 0, 0, 0, 0 );
    f->find.pos = f->screen.top;
    return FALSE;
  }

  seg = f->lastSegment - 1;
  blk = Block( seg );
  off = f->page[ blk ].lines - 1;
  phy = f->page[ blk ].line[ off ].heads - 1;

  ScreenSetBoth( f, seg, blk, off, phy );

  for( i = 0 ; i < ( f->height - 1 ) ; i++ )
    ScreenDecTop( f );

  f->top = FALSE;
  f->bottom = TRUE;
  f->screen.lines = i + 1;
  f->find.pos = f->screen.top;

  if( TRUE == kb_interrupted )
    return FALSE;

  return TRUE;
}

public boolean_t ScreenTop( file_t *f, unsigned int logical )
{
  int i;
  unsigned int seg;
  int blk, off, phy;
  boolean_t res;

  if( logical > 0 )
    logical--;

  ConsoleEnableInterrupt();
  res = FetchLine( f, Segment( logical ), Offset( logical ) );
  ConsoleDisableInterrupt();
  if( FALSE == res ){
    if( FALSE == kb_interrupted )
      return ScreenBot( f );
    logical = f->lastSegment * PAGE_SIZE;
  }

  seg = Segment( logical );
  blk = Block( seg );
  off = Offset( logical );
  phy = 0;

  ScreenSetBoth( f, seg, blk, off, phy );

  for( i = 0 ; i < ( f->height - 1 ) ; i++ )
    ScreenIncBot( f );

  if( 0 == logical )
    f->top = TRUE;
  else
    f->top = FALSE;
  if( i < f->height - 1 )
    f->bottom = TRUE;
  else
    f->bottom = FALSE;

  f->screen.lines = i + 1;
  f->find.pos = f->screen.top;

  if( TRUE == kb_interrupted )
    return FALSE;

  return TRUE;
}

public boolean_t ScreenTopPhysical( file_t *f, position_t *pos )
{
  int i;

  if( FALSE == FetchLine( f, pos->seg, pos->off ) )
    return ScreenBot( f );

  f->screen.top.seg = f->screen.bot.seg = pos->seg;
  f->screen.top.blk = f->screen.bot.blk = pos->blk;
  f->screen.top.off = f->screen.bot.off = pos->off;
  f->screen.top.phy = f->screen.bot.phy = pos->phy;

  for( i = 0 ; i < ( f->height - 1 ) ; i++ )
    ScreenIncBot( f );

  if( f->screen.top.seg == 0 && f->screen.top.off == 0 && f->screen.top.phy == 0 )
    f->top = TRUE;
  else
    f->top = FALSE;
  if( i < f->height - 1 )
    f->bottom = TRUE;
  else
    f->bottom = FALSE;

  f->screen.lines = i + 1;
  f->find.pos = f->screen.top;

  return TRUE;
}

public void ScreenRefresh( file_t *f )
{
  unsigned int logical;

  ConsoleGetWindowSize();

  if( HEIGHT > PAGE_SIZE * ( BLOCK_SIZE - 1 ) )
    HEIGHT = PAGE_SIZE * ( BLOCK_SIZE - 1 );

  f->width = WIDTH;
  f->height = HEIGHT - 1;

  logical = 1 + f->screen.top.seg * PAGE_SIZE + f->screen.top.off;
  ScreenTop( f, logical );
}
