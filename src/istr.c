/*
 * istr.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdlib.h>

#ifdef MSDOS
#include <dos.h>
#endif /* MSDOS */

#include <import.h>
#include <itable.h>
#include <file.h>
#include <uty.h>
#include <begin.h>
#include <istr.h>

#define STUB_16			0
#define STUB_32			1
#define STUB_64			2
#define STUB_128		3
#define STUB_256		4
#define STUB_512		5
#define STUB_1024		6

#define STUB_MAX		STUB_1024

typedef struct {
  int blockSize;
  int blockMax;
  int segmentIndex;
  int segmentMax;
  i_str_t *ptr;
  i_str_t **segments;
} stub_t;

private stub_t stubTable[ STUB_MAX + 1 ];

private char stubIndexTable[ STR_SIZE ];

public void IstrInit()
{
  int i, blockSize, segmentMax;
  char stubIndex;

  blockSize = STR_SIZE / 2;
  stubIndex = STUB_MAX;

  for( i = STR_SIZE - 1 ; i >= 0 ; i-- ){
    stubIndexTable[ i ] = stubIndex;
    if( i == blockSize && stubIndex > 0 ){
      blockSize /= 2;
      stubIndex--;
    }
  }

  blockSize = STR_SIZE;
  segmentMax = PAGE_SIZE * BLOCK_SIZE;

  for( i = STUB_MAX ; i >= 0 ; i-- ){
    if( 0 == blockSize || 0 == segmentMax )
      fprintf( stderr, "lv: invalid stub table\n" ), exit( -1 );

    stubTable[ i ].blockSize = blockSize;
    stubTable[ i ].blockMax = STR_SIZE / blockSize;
    stubTable[ i ].ptr = NULL;
    stubTable[ i ].segmentIndex = 0;
    stubTable[ i ].segmentMax = segmentMax;
    stubTable[ i ].segments = (i_str_t **)Malloc( segmentMax
						 * sizeof( i_str_t * ) );
    blockSize /= 2;
    segmentMax /= 2;
  }
}

private i_str_t *SegmentAlloc( int stubIndex )
{
  stub_t *stub;
  int i, len, istrLength;
  char far *istr;
  char far *ptr;
  char far *prev;

  stub = &stubTable[ stubIndex ];

  if( stub->segmentIndex == stub->segmentMax )
    return NULL;

#ifdef MSDOS
#undef i_str_t
#endif /* MSDOS */
  istrLength = stub->blockSize * sizeof( i_str_t );
  len = stub->blockMax * sizeof( int ) + STR_SIZE * sizeof( i_str_t );
#ifdef MSDOS
#define i_str_t i_str_t far
#endif /* MSDOS */
#ifdef MSDOS
  if( NULL == (istr = FarMalloc( len )) )
    return NULL;
#else
  if( NULL == (istr = malloc( len )) )
    return NULL;
#endif /* MSDOS */
  stub->segments[ stub->segmentIndex ] = (i_str_t *)istr;
  ptr = istr + len;

  ptr -= istrLength;
  *(i_str_t * far *)ptr = (i_str_t *)NULL;
  prev = ptr;
  ptr -= sizeof( int );
  *(int far *)ptr = stubIndex;

  for( i = 1 ; i < stubTable[ stubIndex ].blockMax ; i++ ){
    ptr -= istrLength;
    *(i_str_t * far *)ptr = (i_str_t *)prev;
    prev = ptr;
    ptr -= sizeof( int );
    *(int far *)ptr = stubIndex;
  }

  return (i_str_t *)istr;
}

public i_str_t *IstrAlloc( int stubIndex )
{
  stub_t *stub;
  i_str_t *ptr;

  stubIndex--;
  if( stubIndex < 0 || stubIndex >= STR_SIZE )
    fprintf( stderr, "lv: istr size must be in range 0 .. STR_SIZE-1\n" ), exit( -1 );

  stubIndex = (int)stubIndexTable[ stubIndex ];
  stub = &stubTable[ stubIndex ];

  if( NULL == (ptr = stub->ptr) ){
    if( NULL == (ptr = SegmentAlloc( stubIndex )) ){
      if( stubIndex < STUB_MAX ){
	if( NULL != (ptr = IstrAlloc( stubTable[ stubIndex + 1 ].blockSize )) )
	  return ptr;
      }
      fprintf( stderr, "lv: exhausted stub table\n" );
      NotEnoughMemory();
    }
    stub->segmentIndex++;
    ptr = (i_str_t *)( (char far *)ptr + sizeof( int ) );
  }

  stub->ptr = *(i_str_t * far *)ptr;

  return ptr;
}

public void IstrFree( i_str_t *istr )
{
  stub_t *stub;
  i_str_t *ptr;

  stub = &stubTable[ *(int far *)( (char far *)istr - sizeof( int ) ) ];

  ptr = stub->ptr;
  stub->ptr = istr;

  *(i_str_t * far *)istr = ptr;
}

public void IstrFreeAll()
{
  int i, j;

  for( i = STUB_MAX ; i >= 0 ; i-- ){
    stubTable[ i ].ptr = NULL;
    if( 0 < stubTable[ i ].segmentIndex ){
      for( j = 0 ; j < stubTable[ i ].segmentIndex ; j++ )
#ifdef MSDOS
        FarFree( stubTable[ i ].segments[ j ] );
#else
	free( stubTable[ i ].segments[ j ] );
#endif /* MSDOS */
      stubTable[ i ].segmentIndex = 0;
    }
  }
}
