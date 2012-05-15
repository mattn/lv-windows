/*
 * stream.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: stream.c,v 1.5 2003/11/13 03:08:19 nrt Exp $
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
#include <uty.h>
#include <begin.h>
#include "stream.h"

private byte *gz_filter = "zcat";
private byte *bz2_filter = "bzcat";

private stream_t *StreamAlloc()
{
  stream_t *st;

  st = (stream_t *)Malloc( sizeof( stream_t ) );

  st->fp  = NULL;
  st->sp  = NULL;
  st->pid = -1;

  return st;
}

public stream_t *StreamOpen( byte *file )
{
  stream_t *st;
  byte *exts, *filter = NULL;

  if( access( file, 0 ) ){
    perror( file );
    return NULL;
  }

  st = StreamAlloc();

  if( NULL != (exts = Exts( file )) ){
    if( !strcmp( "gz", exts ) || !strcmp( "GZ", exts )
	|| !strcmp( "z", exts ) || !strcmp( "Z", exts ) )
      filter = gz_filter;
    else if( !strcmp( "bz2", exts ) || !strcmp( "BZ2", exts ) )
      filter = bz2_filter;
  }
  if( NULL != filter ){
    /*
     * zcat or bzcat
     */
    if( NULL == (st->fp = (FILE *)tmpfile()) )
      perror( "temporary file" ), exit( -1 );

#ifdef MSDOS
    { int sout;

      sout = dup( 1 );
      close( 1 );
      dup( fileno( st->fp ) );
      if( 0 > spawnlp( 0, filter, filter, file, NULL ) )
	FatalErrorOccurred();
      close( 1 );
      dup( sout );
      rewind( st->fp );

      return st;
    }
#endif /* MSDOS */

#ifdef UNIX
    { int fds[ 2 ], pid;

      if( 0 > pipe( fds ) )
	perror( "pipe" ), exit( -1 );

      switch( pid = fork() ){
      case -1:
	perror( "fork" ), exit( -1 );
      case 0:
	/*
	 * child process
	 */
	close( fds[ 0 ] );
	close( 1 );
	dup( fds[ 1 ] );
	if( 0 > execlp( filter, filter, file, NULL ) )
	  perror( filter ), exit( -1 );
	/*
	 * never reach here
	 */
      default:
	/*
	 * parent process
	 */
	st->pid = pid;
	close( fds[ 1 ] );
	if( NULL == (st->sp = fdopen( fds[ 0 ], "r" )) )
	  perror( "fdopen" ), exit( -1 );

	return st;
      }
    }
#endif /* UNIX */
  }

  if( NULL == (st->fp = fopen( file, "rb" )) ){
    perror( file );
    return NULL;
  }

  return st;
}

private void StdinDuplicationFailed()
{
  fprintf( stderr, "lv: stdin duplication failed\n" );
  exit( -1 );
}

public stream_t *StreamReconnectStdin()
{
  stream_t *st;
#ifdef UNIX
  struct stat sbuf;
#endif

  st = StreamAlloc();

#ifdef MSDOS
  if( NULL == (st->fp = fdopen( dup( 0 ), "rb" )) )
    StdinDuplicationFailed();
  close( 0 );
  dup( 1 );
#endif /* MSDOS */
#ifdef UNIX
  fstat( 0, &sbuf );
  if( S_IFREG == ( sbuf.st_mode & S_IFMT ) ){
    /* regular */
    if( NULL == (st->fp = fdopen( dup( 0 ), "r" )) )
      StdinDuplicationFailed();
  } else {
    /* socket */
    if( NULL == (st->fp = (FILE *)tmpfile()) )
      perror( "temporary file" ), exit( -1 );
    if( NULL == (st->sp = fdopen( dup( 0 ), "r" )) )
      StdinDuplicationFailed();
  }
  close( 0 );
  if( IsAtty( 1 ) && 0 != open( "/dev/tty", O_RDONLY ) )
    perror( "/dev/tty" ), exit( -1 );
#endif /* UNIX */

  return st;
}

public boolean_t StreamClose( stream_t *st )
{
  fclose( st->fp );
  if( st->sp )
    fclose( st->fp );

  free( st );

  return TRUE;
}
