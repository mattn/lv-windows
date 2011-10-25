/*
 * file.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef MSDOS
#include <dos.h>
#endif /* MSDOS */

#ifdef UNIX
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#endif /* UNIX */

#include <import.h>
#include <decode.h>
#include <console.h>
#include <uty.h>
#include <begin.h>
#include <file.h>

#define IsEucByte( ch )		( ( ch >= 0xa1 && ch <= 0xfe ) )

public boolean_t FileLoadLine( file_t *f, int *length )
{
  int i, ch;
  boolean_t simple = TRUE, eightBit = FALSE;

  i = 0;
  while( EOF != (ch = fgetc( f->fp )) ){
    load_str[ i++ ] = (char)ch;
    if( LF == ch )
      break;
    if( i == ( STR_SIZE - 1 ) )
      break;
    if( FALSE == eightBit && ch > DEL )
      eightBit = TRUE;
    if( TRUE == simple ){
      if( ( ( ch < SP || ch >= DEL ) && CR != ch )
	 || ( UTF_7 == f->inputCodingSystem && '+' == ch ) )
	simple = FALSE;
    }
  }

  *length = i;

  if( EOF == ch )
    f->eof = TRUE;
  else
    f->eof = FALSE;

  if( AUTOSELECT == f->inputCodingSystem && TRUE == eightBit ){
    for( i = 0 ; i < *length ; i++ ){
      ch = load_str[ i ];
      if( 0x80 & ch ){
	if( !IsEucByte( ch ) && ch != SS2 && ch != SS3 ){
	  f->inputCodingSystem = SHIFT_JIS;
	  break;
	} else {
	  f->inputCodingSystem = EUC_JAPAN;
	}
      }
    }
  }

  return simple;
}

public boolean_t FileStretch( file_t *f, unsigned int target )
{
  int ch, count;
  unsigned int segment, line;
  long ptr;

  if( TRUE == f->done )
    return FALSE;

  line = 0;
  count = 0;
  ptr = f->lastPtr;
  segment = f->lastSegment;

  if( fseek( f->fp, ptr, SEEK_SET ) )
    perror( "FileStretch()" ), exit( -1 );

#ifndef MSDOS /* IF NOT DEFINED */
  if( NULL != f->sp ){
    while( EOF != (ch = fgetc( f->sp )) ){
      fputc( ch, f->fp );
      if( TRUE == immediate_print ){
	putchar( ch );
	f->dirty = TRUE;
      }
      count++;
      if( LF == ch || count == ( STR_SIZE - 1 ) ){
	count = 0;
	if( 0 > (ptr = ftell( f->fp )) )
	  perror( "FileStretch()" ), exit( -1 );
	if( ++line == PAGE_SIZE ){
	  f->totalLines += line;
	  line = 0;
	  if( SLOT_SIZE == ++segment ){
	    f->done = TRUE;
	    f->truncated = TRUE;
	    return FALSE;
	  }
	  f->slot[ segment ].ptr = ptr;
	  f->lastSegment = segment;
	  f->lastPtr = ptr;
	  if( segment > target )
	    return TRUE;
	}
	if( TRUE == kb_interrupted )
	  return FALSE;
      }
    }
  } else {
#endif /* MSDOS */
    while( EOF != (ch = fgetc( f->fp )) ){
      count++;
      if( LF == ch || count == ( STR_SIZE - 1 ) ){
	count = 0;
	if( 0 > (ptr = ftell( f->fp )) )
	  perror( "FileStretch()" ), exit( -1 );
	if( ++line == PAGE_SIZE ){
	  f->totalLines += line;
	  line = 0;
	  if( SLOT_SIZE == ++segment ){
	    f->done = TRUE;
	    f->truncated = TRUE;
	    return FALSE;
	  }
	  f->slot[ segment ].ptr = ptr;
	  f->lastSegment = segment;
	  f->lastPtr = ptr;
	  if( segment > target )
	    return TRUE;
	}
#ifdef MSDOS
	if( TRUE == allow_interrupt )
	  bdos( 0x0b, 0, 0 );
#endif /* MSDOS */
	if( TRUE == kb_interrupted )
	  return FALSE;
      }
    }
#ifndef MSDOS /* IF NOT DEFINED */
  }
#endif /* MSDOS */

  if( FALSE == kb_interrupted ){
    if( 0 < line || 0 < count ){
      segment++;
      f->totalLines += line;
      f->lastSegment = segment;
      if( 0 > (f->lastPtr = ftell( f->fp )) )
	perror( "FileStretch()" ), exit( -1 );
    }
    f->done = TRUE;
  }

  if( segment > target )
    return TRUE;
  else
    return FALSE;
}

public boolean_t FileSeek( file_t *f, unsigned int segment )
{
  if( segment >= f->lastSegment )
    if( FALSE == FileStretch( f, segment ) )
      return FALSE;

  if( fseek( f->fp, f->slot[ segment ].ptr, SEEK_SET ) )
    perror( "FileSeek()" ), exit( -1 );

  return TRUE;
}

public void FileFreeHead( file_t *f )
{
  int i, j;

  for( i = 0 ; i < BLOCK_SIZE ; i++ ){
    for( j = 0 ; j < PAGE_SIZE ; j++ ){
      if( NULL != f->page[ i ].line[ j ].head )
	free( f->page[ i ].line[ j ].head );
      f->page[ i ].line[ j ].head = NULL;
    }
  }
}

public void FileRefresh( file_t *f )
{
  int i, j;

  for( i = 0 ; i < BLOCK_SIZE ; i++ ){
    f->used[ i ] = FALSE;
    f->page[ i ].lines = 0;
    for( j = 0 ; j < PAGE_SIZE ; j++ ){
      f->page[ i ].line[ j ].heads = 0;
      f->page[ i ].line[ j ].istr = NULL;
      f->page[ i ].line[ j ].head = NULL;
    }
  }
}

public file_t *FileOpen( char *fileName, FILE *fp, FILE *sp,
			int width, int height,
			char inputCodingSystem,
			char outputCodingSystem,
			char keyboardCodingSystem )
{
  file_t *f;

#ifdef MSDOS
#undef file_t
  if( NULL == (f = (file_t far *)FarMalloc( sizeof( file_t ) )) )
    NotEnoughMemory();
#define file_t file_t far
#else
  f = (file_t *)Malloc( sizeof( file_t ) );
#endif /* MSDOS */

  f->fileName		= fileName;
  f->fp			= fp;
  f->sp			= sp;
  f->lastSegment	= 0;
  f->totalLines		= 0;
  f->lastPtr		= 0L;
  f->slot[ 0 ].ptr	= 0L;

  f->done		= FALSE;
  f->eof		= FALSE;
  f->top		= TRUE;
  f->bottom		= FALSE;
  f->truncated		= FALSE;
  f->dirty		= FALSE;

  f->find.pattern	= NULL;
  f->find.displayed	= FALSE;

  f->width		= width;
  f->height		= height;

  f->inputCodingSystem		= inputCodingSystem;
  f->outputCodingSystem		= outputCodingSystem;
  f->keyboardCodingSystem	= keyboardCodingSystem;

  FileRefresh( f );
  FileStretch( f, 0 );

  return f;
}

public boolean_t FileClose( file_t *f )
{
  if( NULL == f )
    return FALSE;

  FileFreeHead( f );

#ifdef MSDOS
  FarFree( f );
#else
  free( f );
#endif /* MSDOS */

  return TRUE;
}

private char fileStatus[ 128 ];

public char *FileStatus( file_t *f )
{
  sprintf( fileStatus, "%s %u/%u [%s|%s|%s]",
	  f->fileName,
	  1 + f->screen.top.seg * PAGE_SIZE + f->screen.top.off,
	  f->totalLines,
	  cTable[ f->inputCodingSystem ].codingSystemName,
	  cTable[ f->keyboardCodingSystem ].codingSystemName,
	  cTable[ f->outputCodingSystem ].codingSystemName
	  );

  return fileStatus;
}
