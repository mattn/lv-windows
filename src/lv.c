/*
 * lv.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: lv.c,v 1.10 2004/01/05 07:30:15 nrt Exp $
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
#include <sys/types.h>
#include <sys/stat.h>

#ifdef UNIX
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#endif /* UNIX */

#ifdef MSDOS
#include <dos.h>
#endif /* MSDOS */

#include <import.h>
#include <itable.h>
#include <ctable.h>
#include <istr.h>
#include <command.h>
#include <console.h>
#include <conf.h>
#include <file.h>
#include <conv.h>
#include <decode.h>
#include <encode.h>
#include <find.h>
#include <uty.h>
#include <stream.h>
#include <version.h>
#include <begin.h>

/*#define DEBUG*/

#ifdef MSDOS
#define HEAP_SIZE	16384U
#endif /* MSDOS */

#define GREP_COMMAND		"lgrep"
#define GREP_COMMAND_DOS	"LGREP"

private void LvInit( byte **argv )
{
  byte *ptr;

  ItableInit();
  CtableInit();
  IstrInit();
  FileInit();
  DecodeInit();
  CommandInit();
  ConsoleInit();
  ConfInit( (unsigned char **)argv );

  if( NULL != (ptr = strrchr( argv[ 0 ], '/' )) )
    *ptr++ = 0x00;
  else if( NULL != (ptr = strrchr( argv[ 0 ], '\\' )) )
    *ptr++ = 0x00;
  else
    ptr = argv[ 0 ];

  if( !strncmp( ptr, GREP_COMMAND, strlen( GREP_COMMAND ) )
     || !strncmp( ptr, GREP_COMMAND_DOS, strlen( GREP_COMMAND_DOS ) ) )
    grep_mode = TRUE;
  else
    grep_mode = FALSE;
}

private boolean_t LvOpen( conf_t *conf )
{
  if( NULL != conf->file ){
    if( NULL == (conf->st = StreamOpen( *conf->file )) ){
      while( NULL != *(++conf->file) ){
	if( NULL != (conf->st = StreamOpen( *conf->file )) )
	  return TRUE;
      }
      return FALSE;
    }
  } else {
    conf->st = StreamReconnectStdin();
  }

  return TRUE;
}

private boolean_t LvGrep( conf_t *conf, boolean_t showFileName )
{
  file_t *f;
  i_str_t *pattern;
  int length;
  byte *res;
  boolean_t flagMatched;

  if( FALSE == LvOpen( conf ) )
    return FALSE;

  length = strlen( conf->pattern );
  pattern = Decode( IstrAlloc( ZONE_FREE, length + 1 ),
		   conf->keyboardCodingSystem,
		   conf->pattern,
		   &length );

  binary_decode = TRUE;

  FindSetup();

  if( NULL != conf->st->sp ){
    fclose( conf->st->fp );
    conf->st->fp = conf->st->sp;
    conf->st->sp = NULL;
  }

  f = FileAttach( ConfFilename( conf ), conf->st,
		  0, 0,
		  conf->inputCodingSystem,
		  conf->outputCodingSystem,
		  conf->keyboardCodingSystem,
		  conf->pathnameCodingSystem,
		  conf->defaultCodingSystem );

  if( NULL != (res = FindSetPattern( f, pattern )) )
    fprintf( stderr, "lv: %s\n", res ), exit( -1 );

  flagMatched = Conv( f, showFileName );

  FileFree( f );

#ifndef MSDOS /* if NOT defined */
  if( 0 != conf->st->pid ){
    int status;
    wait( &status );
  }
#endif /* MSDOS */

  StreamClose( conf->st );
  conf->st = NULL;

  return flagMatched;
}

private boolean_t LvView( conf_t *conf )
{
  file_t *f;

  if( FALSE == LvOpen( conf ) )
    return FALSE;

  binary_decode = FALSE;

  ConsoleTermInit();
  FindSetup();

  if( conf->width > 0 ) WIDTH = conf->width;
  if( conf->height > 0 ) HEIGHT = conf->height;

  if( HEIGHT > LV_PAGE_SIZE * ( BLOCK_SIZE - 1 ) )
    HEIGHT = LV_PAGE_SIZE * ( BLOCK_SIZE - 1 );

  if( TRUE == conf->keyCodingSystemVirgin )
    conf->keyboardCodingSystem = conf->outputCodingSystem;

  f = FileAttach( ConfFilename( conf ), conf->st,
		  WIDTH, HEIGHT - 1,
		  conf->inputCodingSystem,
		  conf->outputCodingSystem,
		  conf->keyboardCodingSystem,
		  conf->pathnameCodingSystem,
		  conf->defaultCodingSystem );

  /*ConsoleSetUp();*/ /* moved to command.c */
  Command( f, conf->file );
  ConsoleSetDown();

  return TRUE;
}

private boolean_t LvConv( conf_t *conf )
{
  file_t *f;

  if( FALSE == LvOpen( conf ) )
    return FALSE;

  binary_decode = TRUE;

  if( NULL != conf->st->sp ){
    fclose( conf->st->fp );
    conf->st->fp = conf->st->sp;
    conf->st->sp = NULL;
  }

  f = FileAttach( ConfFilename( conf ), conf->st,
		  0, 0,
		  conf->inputCodingSystem,
		  conf->outputCodingSystem,
		  conf->keyboardCodingSystem,
		  conf->pathnameCodingSystem,
		  conf->defaultCodingSystem );

  Conv( f, FALSE );

  return TRUE;
}

public int main( int argc, char **argv )
{
  conf_t *conf;

#ifdef MSDOS
  free( Malloc( HEAP_SIZE ) );
#endif /* MSDOS */

  LvInit( (unsigned char **)argv );

  conf = Malloc( sizeof( conf_t ) );
  Conf( conf, (unsigned char **)argv );

  if( ( NULL == conf->file && IsAtty( 0 ) )
     || ( TRUE == grep_mode && NULL == conf->pattern ) ){
    Banner();
    exit( -1 );
  }

  if( TRUE == grep_mode ){
    boolean_t flagMatched = FALSE;

    if( NULL == conf->file || NULL == conf->file[ 1 ] ){
      /*
       * 対象ファイルが一個
       */
      if( TRUE == LvGrep( conf, FALSE ) )
	flagMatched = TRUE;
    } else {
      /*
       * 対象ファイルが複数
       */
      for( ; ; ){
	if( TRUE == LvGrep( conf, TRUE ) )
	  flagMatched = TRUE;
	if( NULL == *(++conf->file) )
	  break;
      }
    }
    if( TRUE == flagMatched )
      exit( 0 );
    else
      exit( 1 );
    /*
     * never reach here.
     */
  }

  if( IsAtty( 1 ) ){
    LvView( conf );
  } else {
    LvConv( conf );
  }

  exit( 0 );
}
