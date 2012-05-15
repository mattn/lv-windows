/*
 * find.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: find.c,v 1.6 2004/01/05 07:30:15 nrt Exp $
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

#ifdef MSDOS
#include <dos.h>
#endif /* MSDOS */

#include <import.h>
#include <fetch.h>
#include <decode.h>
#include <screen.h>
#include <position.h>
#include <console.h>
#include <dfa.h>
#include <istr.h>
#include <guess.h>
#include <begin.h>
#include <find.h>

private i_str_t *searchPattern = NULL;
private ic_t firstLetter;

private boolean_t FindCheck( line_t *line )
{
  int i, iptr, cptr, tptr, hptr;

  for( hptr = 0 ; hptr < line->heads ; hptr++ )
    line->head[ hptr ].hit = FALSE;
  hptr = 0;

  for( iptr = 0 ; NOSET != line->istr[ iptr ].charset ; iptr++ ){
    if( firstLetter == line->istr[ iptr ].c ){
      for( cptr = iptr, tptr = 0 ; NOSET != searchPattern[ tptr ].charset ; tptr++, cptr++ ){
	if( line->istr[ cptr ].c != searchPattern[ tptr ].c )
	  goto findNext;
	if( searchPattern[ tptr ].charset != line->istr[ cptr ].charset )
	  goto findNext;
      }
      /*
       * string matched
       */
      while( iptr >= line->head[ hptr ].ptr )
	hptr++;
      line->head[ hptr ].hit = TRUE;
      for( i = iptr ; i < cptr ; i++ )
	line->istr[ i ].attr |= ATTR_STANDOUT;
      iptr = cptr - 1;
    }
findNext:
    /* dummy line */;
  }

  return line->head[ hptr ].hit;
}

private boolean_t FindOnly( i_str_t *istr )
{
  int iptr, cptr, tptr;

  for( iptr = 0 ; NOSET != istr[ iptr ].charset ; iptr++ ){
    if( firstLetter == istr[ iptr ].c ){
      for( cptr = iptr, tptr = 0 ; NOSET != searchPattern[ tptr ].charset ; tptr++, cptr++ ){
	if( searchPattern[ tptr ].c != istr[ cptr ].c )
	  goto findOnlyNext;
	if( searchPattern[ tptr ].charset != istr[ cptr ].charset )
	  goto findOnlyNext;
      }
      return TRUE;
    }
findOnlyNext:
    /* dummy line */;
  }

  return FALSE;
}

private boolean_t FindCheckRegexp( line_t *line )
{
  int i, iptr, cptr, hptr;

  for( hptr = 0 ; hptr < line->heads ; hptr++ )
    line->head[ hptr ].hit = FALSE;
  hptr = 0;

  for( iptr = 0 ; /* NOSET != line->istr[ iptr ].charset */ ; iptr++ ){
    if( TRUE != regexp_short_cut
       || regexp_first_letter == ToLower( line->istr[ iptr ].charset,
					 line->istr[ iptr ].c ) ){
      cptr = iptr;
      if( TRUE == ReRun( line->istr, &cptr ) ){
	/*
	 * string matched
	 */
	if( cptr == iptr ){
	  /* in case that pattern begins with '^' */
	  line->head[ hptr ].hit = TRUE;
	  line->istr[ iptr ].attr |= ATTR_STANDOUT;
	} else {
	  while( iptr >= line->head[ hptr ].ptr )
	    hptr++;
	  line->head[ hptr ].hit = TRUE;
	  for( i = iptr ; i <= cptr ; i++ )
	    line->istr[ i ].attr |= ATTR_STANDOUT;
	  iptr = cptr;
	}
      } else if( NOSET == line->istr[ cptr ].charset )
	break;
    }
    if( NOSET == line->istr[ iptr ].charset )
      break;
  }

  return line->head[ hptr ].hit;
}

private boolean_t FindOnlyRegexp( i_str_t *istr )
{
  int iptr, cptr;

  for( iptr = 0 ; /* NOSET != istr[ iptr ].charset */ ; iptr++ ){
    if( TRUE != regexp_short_cut
       || regexp_first_letter == ToLower( istr[ iptr ].charset,
					 istr[ iptr ].c ) ){
      cptr = iptr;
      if( TRUE == ReRun( istr, &cptr ) )
	return TRUE;
      else if( NOSET == istr[ cptr ].charset )
	break;
    }
    if( NOSET == istr[ iptr ].charset )
      break;
  }

  return FALSE;
}

public void FindSetup()
{
  if( TRUE == regexp_search ){
    find_func = FindCheckRegexp;
    find_only_func = FindOnlyRegexp;
  } else {
    find_func = FindCheck;
    find_only_func = FindOnly;
  }
}

private void FindReset( file_t *f )
{
  int i, j, k;
  line_t *line;
  i_str_t *istr;

  for( i = 0 ; i < BLOCK_SIZE ; i++ ){
    if( TRUE == f->used[ i ] ){
      for( j = 0 ; j < f->page[ i ].lines ; j++ ){
	line = &f->page[ i ].line[ j ];
	istr = line->istr;
	for( k = 0 ; NOSET != istr[ k ].charset ; k++ ){
	  if( CNTRL != istr[ k ].charset )
	    istr[ k ].attr &= ~ATTR_STANDOUT;
	}
	if( NULL != f->find.pattern )
	  (*find_func)( line );
      }
    }
  }
}

public boolean_t FindClearPattern( file_t *f )
{
  if( NULL != f->find.pattern ){
    IstrFree( f->find.pattern );
    f->find.pattern = NULL;
    searchPattern = NULL;
    FindReset( f );
    ReFreeDFA();
    return TRUE;
  } else {
    return FALSE;
  }
}

public byte *FindResetPattern( file_t *f, i_str_t *istr )
{
  byte *res = NULL;

  AdjustPatternCharset( f->inputCodingSystem, f->keyboardCodingSystem,
		       f->defaultCodingSystem, istr );

  searchPattern = istr;
  firstLetter = searchPattern[ 0 ].c;

  f->find.pattern = istr;

  if( TRUE == regexp_search ){
    if( NULL != (res = ReMakeDFA( istr )) ){
      IstrFree( f->find.pattern );
      f->find.pattern = NULL;
      searchPattern = NULL;
    }
  }

  return res;
}

public byte *FindSetPattern( file_t *f, i_str_t *istr )
{
  byte *res;

  if( NULL != f->find.pattern )
    IstrFree( f->find.pattern );

  f->find.first = TRUE;
  f->find.pos = f->screen.top;

  res = FindResetPattern( f, istr );

  FindReset( f );

  return res;
}

private boolean_t FindContinue( file_t *f, position_t *pos, boolean_t forward )
{
  int idx;
  unsigned int segment;
  int offset, foundOffset = 0;
  i_str_t *istr;
  boolean_t simple, res, found;
  byte *str;

  segment = pos->seg;

  f->eof = FALSE;
  found = FALSE;

  while( FALSE == f->eof ){
    if( FALSE == FileSeek( f, segment ) )
      return FALSE;
#ifdef MSDOS
    if( TRUE == allow_interrupt )
      bdos( 0x0b, 0, 0 );
#endif /* MSDOS */
    if( TRUE == kb_interrupted )
      break;
    for( offset = 0 ; offset < LV_PAGE_SIZE && FALSE == f->eof ; offset++ ){
      str = FileLoadLine( f, &idx, &simple );
      if( 0 == idx )
	return FALSE;
      
      if( TRUE == simple )
	istr = DecodeSimple( IstrAlloc( ZONE_FREE, idx + 1 ),
			    str, &idx );
      else
	istr = Decode( IstrAlloc( ZONE_FREE, idx + 1 ),
		      f->inputCodingSystem, str, &idx );
      FileFreeLine( f );

      res = (*find_only_func)( istr );
      IstrFree( istr );

      if( TRUE == res ){
	if( TRUE == forward ){
	  pos->seg = segment;
	  pos->off = offset;
	  return TRUE;
	} else {
	  found = TRUE;
	  foundOffset = offset;
	}
      }
    }
    if( TRUE == forward ){
      segment++;
    } else {
      if( TRUE == found ){
	pos->seg = segment;
	pos->off = foundOffset;
	return TRUE;
      }
      if( 0 == segment )
	return FALSE;
      else
	segment--;
    }
  }

  return FALSE;
}

public int FindForward( file_t *f )
{
  boolean_t flagFind, flagContinue;
  position_t pos;
  int count = 0;

  PositionAssign( pos, f->find.pos );

  if( FALSE == f->find.first ){
    flagContinue = FALSE;
    do {
      PositionInc( f, pos.seg, pos.blk, pos.off, pos.phy );
      count++;
      flagContinue = TRUE;
    } while( 0 );
    if( FALSE == flagContinue )
      return -1;
  } else {
    if( FALSE == FetchLine( f, pos.seg, pos.off ) )
      return -1;
    f->find.first = FALSE;
  }

  ConsoleEnableInterrupt();

  flagContinue = FALSE;
  flagFind = FALSE;

  /*
   * キャッシュ内のサーチ
   */
  do {
#ifdef MSDOS
    if( TRUE == allow_interrupt )
      bdos( 0x0b, 0, 0 );
#endif /* MSDOS */
    if( TRUE == kb_interrupted )
      break;
    if( TRUE == f->page[ pos.blk ].line[ pos.off ].head[ pos.phy ].hit ){
      flagFind = TRUE;
      break;
    }
    if( ++pos.phy >= f->page[ pos.blk ].line[ pos.off ].heads ){
      if( ++pos.off >= f->page[ pos.blk ].lines ){
	pos.seg++;
	pos.blk = Block( pos.seg );
	if( TRUE == f->used[ pos.blk ]
	   && pos.seg == f->page[ pos.blk ].segment ){
	  pos.off = 0;
	} else {
	  flagContinue = TRUE;
	  break;
	}
      }
      pos.phy = 0;
    }
    count++;
  } while( 1 );

  /*
   * キャッシュ外のサーチ
   */
  if( TRUE == flagContinue ){
    if( TRUE == FindContinue( f, &pos, TRUE ) ){
      ConsoleDisableInterrupt();
      ScreenTop( f, 1 + pos.seg * LV_PAGE_SIZE + pos.off );
      PositionAssign( pos, f->screen.top );
      while( FALSE == f->page[ pos.blk ].line[ pos.off ].head[ pos.phy ].hit ){
	PositionInc( f, pos.seg, pos.blk, pos.off, pos.phy );
      }
      ScreenTopPhysical( f, &pos );
      return -2;
    }
  }

  ConsoleDisableInterrupt();

  if( TRUE == flagFind ){
    ScreenTopPhysical( f, &pos );
    return count;
  } else {
    PositionAssign( pos, f->find.pos );		/* for MSDOS */
    ScreenTopPhysical( f, &pos );
    return -1;
  }
}

public int FindBackward( file_t *f )
{
  boolean_t flagFind, flagContinue;
  position_t pos;
  int count = 0;

  PositionAssign( pos, f->find.pos );

  if( FALSE == f->find.first ){
    flagContinue = FALSE;
    do {
      PositionDec( f, pos.seg, pos.blk, pos.off, pos.phy );
      count++;
      flagContinue = TRUE;
    } while( 0 );
    if( FALSE == flagContinue )
      return -1;
  } else {
    if( FALSE == FetchLine( f, pos.seg, pos.off ) )
      return -1;
    f->find.first = FALSE;
  }

  ConsoleEnableInterrupt();

  flagContinue = FALSE;
  flagFind = FALSE;

  /*
   * キャッシュ内のサーチ
   */
  do {
#ifdef MSDOS
    if( TRUE == allow_interrupt )
      bdos( 0x0b, 0, 0 );
#endif /* MSDOS */
    if( TRUE == kb_interrupted )
      break;
    if( TRUE == f->page[ pos.blk ].line[ pos.off ].head[ pos.phy ].hit ){
      flagFind = TRUE;
      break;
    }
    if( --pos.phy < 0 ){
      if( --pos.off < 0 ){
	if( pos.seg == 0 ){
	  pos.phy++;
	  pos.off++;
	  break;
	}
	pos.seg--;
	pos.blk = Block( pos.seg );
	if( TRUE == f->used[ pos.blk ]
	   && pos.seg == f->page[ pos.blk ].segment ){
	  pos.off = f->page[ pos.blk ].lines - 1;
	} else {
	  flagContinue = TRUE;
	  break;
	}
      }
      pos.phy = f->page[ pos.blk ].line[ pos.off ].heads - 1;
    }
    count++;
  } while( 1 );

  /*
   * キャッシュ外のサーチ
   */
  if( TRUE == flagContinue ){
    if( TRUE == FindContinue( f, &pos, FALSE ) ){
      ConsoleDisableInterrupt();
      ScreenTop( f, 1 + pos.seg * LV_PAGE_SIZE + pos.off );
      PositionAssign( pos, f->screen.top );
      pos.phy = f->page[ pos.blk ].line[ pos.off ].heads - 1;
      while( FALSE == f->page[ pos.blk ].line[ pos.off ].head[ pos.phy ].hit ){
	PositionDec( f, pos.seg, pos.blk, pos.off, pos.phy );
      }
      ScreenTopPhysical( f, &pos );
      return -2;
    }
  }

  ConsoleDisableInterrupt();

  if( TRUE == flagFind ){
    ScreenTopPhysical( f, &pos );
    return count;
  } else {
    PositionAssign( pos, f->find.pos );		/* for MSDOS */
    ScreenTopPhysical( f, &pos );
    return -1;
  }
}
