/*
 * uty.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef UNIX
#include <unistd.h>
#endif /* UNIX */

#ifdef MSDOS
#include <dos.h>
#endif /* MSDOS */

#include <import.h>
#include <itable.h>
#include <begin.h>
#include <uty.h>

public ic_t BinarySearch( codes_t *array, int high, ic_t code )
{
  int low, mid;

  low = 0;
  mid = high >> 1;

  for( ; low <= high ; mid = ( low + high ) >> 1 ){
    if( array[ mid ].code == code ){
      return array[ mid ].peer;
    } else if( array[ mid ].code > code ){
      high = mid - 1;
    } else {
      low = mid + 1;
    }
  }

  return 0;
}

public ic_t BinarySearchCset( codes_cset_t *array, int high, ic_t code,
			     char *cset )
{
  int low, mid;

  low = 0;
  mid = high >> 1;

  for( ; low <= high ; mid = ( low + high ) >> 1 ){
    if( array[ mid ].code == code ){
      *cset = array[ mid ].cset;
      return array[ mid ].peer;
    } else if( array[ mid ].code > code ){
      high = mid - 1;
    } else {
      low = mid + 1;
    }
  }

  return 0;
}

public void NotEnoughMemory()
{
  fprintf( stderr, "lv: not enough memory\n" ), exit( -1 );
}

public void FatalErrorOccurred()
{
  fprintf( stderr, "lv: fatal error occurred\n" ), exit( -1 );
}

public void *Malloc( unsigned int size )
{
  void *ptr;

  if( NULL == (ptr = malloc( size )) )
    NotEnoughMemory();

  return ptr;
}

#define TOKEN_LENGTH	32

public char *TokenAlloc( char *s )
{
  int i, j;
  char *token;

  for( i = 0 ; i < TOKEN_LENGTH ; i++ )
    if( NULL == s[ i ] || ' ' == s[ i ] || 0x09 == s[ i ] )
      break;
  if( i == TOKEN_LENGTH )
    return "";

  token = (char *)Malloc( i + 1 );
  for( j = 0 ; j < i ; j++ )
    token[ j ] = s[ j ];
  token[ j ] = NULL;

  return token;
}

#ifdef MSDOS
public void far *FarMalloc( unsigned int size )
{
  union REGS regs;

  regs.h.ah = 0x48;
  if( size & 0x000f )
    regs.x.bx = 1 + ( size >> 4 );
  else
    regs.x.bx = size >> 4;

  intdos( &regs, &regs );

  if( 0x0001 & regs.x.flags ){
    if( 0x08 == regs.x.ax )
      NotEnoughMemory();
    else
      FatalErrorOccurred();
  }

  return MK_FP( regs.x.ax, 0 );
}

public void FarFree( void far *ptr )
{
  union REGS regs;

  regs.h.ah = 0x49;
  regs.x.es = FP_SEG( ptr );

  intdosy( &regs, &regs );

  if( 0x0001 & regs.x.flags )
    FatalErrorOccurred();
}
#endif /* MSDOS */

public boolean_t IsAtty( int fd )
{
#ifdef MSDOS
  union REGS regs;

  regs.x.ax = 0x4400;
  regs.x.bx = fd;
  intdos( &regs, &regs );

  if( 0x0001 & regs.x.flags ){
    /*
     * carry flag was set
     */
    if( 0x0006 == regs.x.ax )
      fprintf( stderr, "lv: file descripter %d is not opend\n" );
    else
      fprintf( stderr, "lv: device io control for fd:%d failed\n", fd );
    exit( -1 );
  } else if( 0x8000 == ( 0x8000 & regs.x.dx ) ){
    /*
     * character device
     */
    if( 0x0003 == ( 0x0003 & regs.x.dx ) )
      /*
       * stdin and stdout
       */
      return TRUE;
    else
      return FALSE;
  } else {
    /*
     * block device
     */
    return FALSE;
  }
#else
  if( isatty( fd ) )
    return TRUE;
  else
    return FALSE;
#endif /* MSDOS */
}

public char *Exts( char *s )
{
  int i;

  for( i = strlen( s ) - 1 ; i >= 0 ; i-- ){
    if( '.' == s[ i ] )
      return &s[ i + 1 ];
    else if( '/' == s[ i ] || '\\' == s[ i ] )
      return NULL;
  }

  return NULL;
}
