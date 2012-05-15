/*
 * file.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: file.c,v 1.11 2004/01/05 07:30:15 nrt Exp $
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
#include <string.h>

#ifdef MSDOS
#include <dos.h>
#endif /* MSDOS */

#ifdef UNIX
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#endif /* UNIX */

#include <import.h>
#include <decode.h>
#include <encode.h>
#include <console.h>
#include <uty.h>
#include <guess.h>
#include <begin.h>
#include <file.h>

extern byte *FindResetPattern( file_t *f, i_str_t *istr );

#define LOAD_SIZE	STR_SIZE

private byte short_str[ LOAD_SIZE ];

#ifndef MSDOS /* if NOT defned */
#define LOAD_COUNT	8		/* upper logical length 8Kbytes */
#else
#define LOAD_COUNT	1
#endif /* MSDOS */

private byte *long_str = NULL;
private byte *load_array[ LOAD_COUNT ];

public void FileFreeLine( file_t *f )
{
  if( long_str ){
    free( long_str );
    long_str = NULL;
  }
}

/*
 * 現在のファイルポインタから 1行を読み込んでバッファに格納する.
 * コード系の自動判別の対象となる場合, 自動判別を行なう.
 * EOF を見つけた時は EOF フラグを立てる.
 * 読み込んだ文字列の長さをポインタの中身に返す.
 * 重要: 読み込んだ長さが 0 は読むべきデータが無いことをを意味する.
 *       一方, EOF はデータを読み込んだが, 次に読むべきデータが
 *       無いことを意味する.
 */
public byte *FileLoadLine( file_t *f, int *length, boolean_t *simple )
{
  boolean_t flagSimple = TRUE;
  boolean_t flagEightBit = FALSE, flagHz = FALSE;
  int idx, len, count, ch;
  char *str;

  if( long_str ){
    free( long_str );
    long_str = NULL;
  }

  len = 0;
  count = 0;
  idx = 0;

  while( EOF != (ch = getc( f->fp )) ){
    len++;
    load_array[ count ][ idx++ ] = (byte)ch;
    if( LF == ch ){
      /* UNIX style */
      break;
    } else if( CR == ch ){
      if( LF == (ch = getc( f->fp )) ){
	/* MSDOS style */
      } else if( EOF == ch ){
	/* need to avoid EOF due to pre-load of that */
	ch = LF;
      } else {
	/* Mac style */
	ungetc( ch, f->fp );
      }
      load_array[ count ][ idx - 1 ] = LF;
      break;
    }
    if( LOAD_SIZE == idx ){
      count++;
      if( LOAD_COUNT == count )
	break;
      load_array[ count ] = (byte *)Malloc( LOAD_SIZE );
      idx = 0;
    }
    if( FALSE == flagEightBit ){
      if( ch > DEL ){
	flagEightBit = TRUE;
	flagSimple = FALSE;
      } else if( FALSE == flagHz ){
	if( '~' == ch && TRUE == hz_detection ){
	  flagHz = TRUE;
	  flagSimple = FALSE;
	}
      }
    }
    if( TRUE == flagSimple ){
      if( ch < SP || DEL == ch
	 || ( '+' == ch && UTF_7 == f->inputCodingSystem ) )
	flagSimple = FALSE;
    }
  }

  if( EOF == ch )
    f->eof = TRUE;
  else
    f->eof = FALSE;

  if( LOAD_SIZE >= len ){
    str = short_str;
  } else {
    long_str = (byte *)Malloc( len );
    for( ch = count = idx = 0 ; ch < len ; ch++, idx++ ){
      if( LOAD_SIZE == idx ){
	if( count > 0 )
	  free( load_array[ count ] );
	idx = 0;
	count++;
      }
      long_str[ ch ] = load_array[ count ][ idx ];
    }
    if( count > 0 )
      free( load_array[ count ] );
    str = long_str;
  }

  *length = len;
  *simple = flagSimple;

  if( AUTOSELECT == f->inputCodingSystem ){
    if( TRUE == flagEightBit ){
      f->inputCodingSystem = GuessCodingSystem( str, len,
					       f->defaultCodingSystem );
      if( NULL != f->find.pattern )
	FindResetPattern( f, f->find.pattern );
    } else if( TRUE == flagHz ){
      if( GuessHz( str, len ) ){
	f->inputCodingSystem = HZ_GB;
	if( NULL != f->find.pattern )
	  FindResetPattern( f, f->find.pattern );
      }
    }
  }

  return str;
}

/*
 * 現在のファイルポインタからファイルを読み,
 * 指定されたページまでファイルポインタ・フレームを伸ばす.
 * 指定されたページまで辿りつけなかった場合 FALSE を返す.
 *      1) キーボードインタラプトがかかった場合や
 *      2) ソケットに読むべきデータがない場合を含む.
 *
 * EOF に到達した場合, 読み出し終了フラグを設定する.
 *
 * ※ n ページ目まで stretch した時, ファイルポインタは n+1 ページの先頭.
 */

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
    while( EOF != (ch = getc( f->sp )) ){
      putc( ch, f->fp );
      count++;
      if( LF == ch || CR == ch || count == (LOAD_SIZE * LOAD_COUNT) ){
	if( CR == ch ){
	  if( LF != (ch = getc( f->sp )) )
	    ungetc( ch, f->sp );
	  else
	    putc( LF, f->fp );
	}
	count = 0;
	if( 0 > (ptr = ftell( f->fp )) )
	  perror( "FileStretch()" ), exit( -1 );
	if( ++line == LV_PAGE_SIZE ){
	  f->totalLines += line;
	  line = 0;
	  if( 0 == Slot( ++segment ) ){
	    if( FRAME_SIZE == ++f->lastFrame
	       ||
	       NULL == (f->slot[ f->lastFrame ]
			= (long *)malloc( sizeof( long ) * SLOT_SIZE ))
	       ){
	      f->done = TRUE;
	      f->truncated = TRUE;
	      return FALSE;
	    }
	  }
	  f->slot[ f->lastFrame ][ Slot( segment ) ] = ptr;
	  f->lastSegment = segment;
	  f->lastPtr = ptr;
	  if( segment > target )
	    return TRUE;
	}
	if( TRUE == kb_interrupted )
	  return FALSE;
      }
    }
    if( -1 != f->pid && feof( f->sp ) ){
      int status;
      wait( &status );
    }
  } else {
#endif /* MSDOS */
    while( EOF != (ch = getc( f->fp )) ){
      count++;
      if( LF == ch || CR == ch || count == (LOAD_SIZE * LOAD_COUNT) ){
	if( CR == ch ){
	  if( LF != (ch = getc( f->fp )) )
	    ungetc( ch, f->fp );
	}
	count = 0;
	if( 0 > (ptr = ftell( f->fp )) )
	  perror( "FileStretch()" ), exit( -1 );
	if( ++line == LV_PAGE_SIZE ){
	  f->totalLines += line;
	  line = 0;
	  if( 0 == Slot( ++segment ) ){
	    if( FRAME_SIZE == ++f->lastFrame
	       || NULL == (f->slot[ f->lastFrame ]
			   = (long *)malloc( sizeof( long ) * SLOT_SIZE ))
	       ){
	      f->done = TRUE;
	      f->truncated = TRUE;
	      return FALSE;
	    }
	  }
	  f->slot[ f->lastFrame ][ Slot( segment ) ] = ptr;
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

/*
 * 指定されたページまでファイルポインタ・フレームを伸ばし,
 * ファイルポインタをページの先頭に設定する.
 * 指定されたページまで辿りつけなかった場合 FALSE を返す.
 */

public boolean_t FileSeek( file_t *f, unsigned int segment )
{
  if( segment >= f->lastSegment )
    if( FALSE == FileStretch( f, segment ) )
      return FALSE;

  if( fseek( f->fp, f->slot[ Frame( segment ) ][ Slot( segment ) ], SEEK_SET ) )
    perror( "FileSeek()" ), exit( -1 );

  return TRUE;
}

public void FileCacheInit( file_t *f )
{
  int i, j;

  for( i = 0 ; i < BLOCK_SIZE ; i++ ){
    f->used[ i ] = FALSE;
    f->page[ i ].lines = 0;
    for( j = 0 ; j < LV_PAGE_SIZE ; j++ ){
      f->page[ i ].line[ j ].heads = 0;
      f->page[ i ].line[ j ].istr = NULL;
      f->page[ i ].line[ j ].head = NULL;
    }
  }
}

public void FileRefresh( file_t *f )
{
  int i, j;

  f->fileNameI18N = NULL;

  for( i = 0 ; i < BLOCK_SIZE ; i++ ){
    for( j = 0 ; j < LV_PAGE_SIZE ; j++ ){
      if( NULL != f->page[ i ].line[ j ].head ){
	free( f->page[ i ].line[ j ].head );
	f->page[ i ].line[ j ].head = NULL;
      }
      if( NULL != f->page[ i ].line[ j ].istr ){
	IstrFree( f->page[ i ].line[ j ].istr );
	f->page[ i ].line[ j ].istr = NULL;
      }
    }
  }

  FileCacheInit( f );
}

public file_t *FileAttach( byte *fileName, stream_t *st,
			  int width, int height,
			  byte inputCodingSystem,
			  byte outputCodingSystem,
			  byte keyboardCodingSystem,
			  byte pathnameCodingSystem,
			  byte defaultCodingSystem )
{
  file_t *f;
  int i;

#ifdef MSDOS
#undef file_t
  if( NULL == (f = (file_t far *)FarMalloc( sizeof( file_t ) )) )
    NotEnoughMemory();
#define file_t file_t far
#else
  f = (file_t *)Malloc( sizeof( file_t ) );
#endif /* MSDOS */

  f->fileName		= fileName;
  f->fileNameI18N	= NULL;
  f->fileNameLength	= 0;

  f->fp			= st->fp;
  f->sp			= st->sp;
  f->pid		= st->pid;
  f->lastSegment	= 0;
  f->totalLines		= 0L;
  f->lastPtr		= 0L;

  f->lastFrame		= 0;

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
  f->pathnameCodingSystem	= pathnameCodingSystem;
  f->defaultCodingSystem	= defaultCodingSystem;

  for (i=0; i<BLOCK_SIZE; i++) f->used[i]=FALSE;

  return f;
}

public void FilePreload( file_t *f )
{
  int i;

  for( i = 0 ; i < FRAME_SIZE ; i++ )
    f->slot[ i ] = NULL;

  f->slot[ 0 ]		= (long *)Malloc( sizeof( long ) * SLOT_SIZE );
  f->slot[ 0 ][ 0 ]	= 0L;

  FileCacheInit( f );
  FileStretch( f, 0 );
}

/*
 * IstrFreeAll() することに注意.
 */
public boolean_t FileFree( file_t *f )
{
  if( NULL == f )
    return FALSE;

#ifdef MSDOS
  FarFree( f );
#else
  free( f );
#endif /* MSDOS */

  IstrFreeAll();

  return TRUE;
}

public boolean_t FileDetach( file_t *f )
{
  int i;

  if( NULL == f )
    return FALSE;

  for( i = 0 ; i < FRAME_SIZE ; i++ ){
    if( NULL != f->slot[ i ] )
      free( f->slot[ i ] );
  }

  FileRefresh( f );

  FileFree( f );

  return TRUE;
}

public byte *FileName( file_t *f )
{
  int length;

  if( NULL == f->fileNameI18N ){
    length = strlen( f->fileName );
    f->fileNameI18N = Decode( IstrAlloc( ZONE_FREE, length + 1 ),
			     f->pathnameCodingSystem,
			     f->fileName,
			     &length );
    f->fileNameLength = length;
  }

  encode_length = CODE_SIZE;
  Encode( f->fileNameI18N, 0, f->fileNameLength,
	 f->outputCodingSystem, FALSE,
	 encode_str, &encode_length );

  return EncodeStripAttribute( encode_str, encode_length );
}

private byte fileStatus[ 256 ];

public byte *FileStatus( file_t *f )
{
  sprintf( fileStatus, "%s %lu/%lu [%s|%s|%s|%s]",
	  FileName( f ),
	  (unsigned long)( 1 + f->screen.top.seg * LV_PAGE_SIZE + f->screen.top.off ),
	  f->totalLines,
	  cTable[ (int)f->inputCodingSystem ].codingSystemName,
	  cTable[ (int)f->keyboardCodingSystem ].codingSystemName,
	  cTable[ (int)f->outputCodingSystem ].codingSystemName,
	  cTable[ (int)f->pathnameCodingSystem ].codingSystemName
	  );

  return fileStatus;
}

public void FileInit()
{
  load_array[ 0 ] = short_str;
}

