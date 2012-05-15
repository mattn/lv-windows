/*
 * istr.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: istr.c,v 1.8 2004/01/05 07:30:15 nrt Exp $
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
#include <itable.h>
#include <file.h>
#include <uty.h>
#include <begin.h>
#include <istr.h>

#define STUB_128		0
#define STUB_256		1
#define STUB_512		2
#define STUB_1024		3
#define STUB_SPARE		4

#define STUB_SIZE		( STUB_SPARE + 1 )

#define STUB_SPARE_SEGS		( LV_PAGE_SIZE + 1 )

typedef struct {
  int blockSize;
  int blockMax;
  int segmentIndex;
  int segmentMax;
  i_str_t *ptr;
  i_str_t **segments;
} stub_t;

private stub_t stubTable[ ZONE_SIZE ][ STUB_SPARE + 1 ];

private byte stubIndexTable[ STR_SIZE ];

public void IstrInit()
{
  int i, zone, blockSize, segmentMax;
  byte stubIndex;

  /*
   * create stub index table
   */
  blockSize = STR_SIZE / 2;
  stubIndex = STUB_SPARE - 1;

  for( i = STR_SIZE - 1 ; i >= 0 ; i-- ){
    stubIndexTable[ i ] = stubIndex;
    if( i == blockSize && stubIndex > 0 ){
      blockSize /= 2;
      stubIndex--;
    }
  }

  /*
   * create stub table
   */
  for( zone = 0 ; zone < ZONE_SIZE ; zone++ ){
    blockSize = STR_SIZE;
    segmentMax = LV_PAGE_SIZE;

    for( i = STUB_SPARE ; i >= 0 ; i-- ){
      if( 0 == blockSize || 0 == segmentMax )
	fprintf( stderr, "lv: invalid stub table\n" ), exit( -1 );

      stubTable[ zone ][ i ].blockSize = blockSize;
      stubTable[ zone ][ i ].blockMax = STR_SIZE / blockSize;
      stubTable[ zone ][ i ].ptr = NULL;
      stubTable[ zone ][ i ].segmentIndex = 0;
      if( STUB_SPARE == i )
	stubTable[ zone ][ i ].segmentMax = STUB_SPARE_SEGS;
      else
	stubTable[ zone ][ i ].segmentMax = segmentMax;
      stubTable[ zone ][ i ].segments
	= (i_str_t **)Malloc( segmentMax * sizeof( i_str_t * ) );
      if( STUB_SPARE != i ){
	blockSize /= 2;
	segmentMax /= 2;
      }
    }
  }
}

private i_str_t *SegmentAlloc( int zone, int stubIndex )
{
  stub_t *stub;
  int i, len, istrLength;
  byte far *istr;
  byte far *ptr;
  byte far *prev;

  stub = &stubTable[ zone ][ stubIndex ];

  if( stub->segmentIndex == stub->segmentMax )
    return NULL;

#ifdef MSDOS
#undef i_str_t
#endif /* MSDOS */
  istrLength = stub->blockSize * sizeof( i_str_t );
  len = stub->blockMax * sizeof( long ) + STR_SIZE * sizeof( i_str_t );
#ifdef MSDOS
#define i_str_t i_str_t far
#endif /* MSDOS */
#ifdef MSDOS
  if( NULL == (istr = FarMalloc( len )) )
    return NULL;
#else
  if( NULL == (istr = Malloc( len )) )
    return NULL;
#endif /* MSDOS */
  stub->segments[ stub->segmentIndex ] = (i_str_t *)istr;
  ptr = istr + len;

  ptr -= istrLength;
  *(i_str_t * far *)ptr = (i_str_t *)NULL;
  prev = ptr;
  ptr -= sizeof( long );
  *(int far *)ptr = ( zone << 8 ) | stubIndex;

  for( i = 1 ; i < stubTable[ zone ][ stubIndex ].blockMax ; i++ ){
    ptr -= istrLength;
    *(i_str_t * far *)ptr = (i_str_t *)prev;
    prev = ptr;
    ptr -= sizeof( long );
    *(int far *)ptr = ( zone << 8 ) | stubIndex;
  }

  return (i_str_t *)istr;
}

private i_str_t *LongSegmentAlloc( int zone, int length )
{
  stub_t *stub;
  int i, idx, len, istrLength;
  byte far *istr;
  i_str_t *ptr;

  stub = &stubTable[ zone ][ STUB_SPARE ];

  if( NULL != stub->ptr ){
    ptr = (i_str_t *)((byte far *)stub->ptr - sizeof( long ));
    for( i = 0 ; i < stub->segmentMax ; i++ ){
      if( ptr == stub->segments[ i ] )
	break;
    }
    stub->ptr = *(i_str_t * far *)stub->ptr;
    idx = i;
#ifdef MSDOS
    FarFree( stub->segments[ idx ] );
#else
    free( stub->segments[ idx ] );
#endif /* MSDOS */
  } else {
    if( stub->segmentIndex == stub->segmentMax ){
      fprintf( stderr, "segment:%d\n", stub->segmentIndex );
      return NULL;
    }
    idx = stub->segmentIndex++;
  }

#ifdef MSDOS
#undef i_str_t
#endif /* MSDOS */
  istrLength = length * sizeof( i_str_t );
  len = sizeof( long ) + istrLength;
#ifdef MSDOS
#define i_str_t i_str_t far
#endif /* MSDOS */
#ifdef MSDOS
  if( NULL == (istr = FarMalloc( len )) )
    return NULL;
#else
  if( NULL == (istr = Malloc( len )) )
    return NULL;
#endif /* MSDOS */
  stub->segments[ idx ] = (i_str_t *)istr;

  *(i_str_t * far *)(istr + sizeof( long )) = (i_str_t *)NULL;
  *(int far *)istr = ( zone << 8 ) | STUB_SPARE;

  return (i_str_t *)istr;
}

public i_str_t *IstrAlloc( int zone, int istrLength )
{
  stub_t *stub;
  i_str_t *ptr;
  int stubIndex;

  if( istrLength > STR_SIZE ){
    if( NULL == (ptr = LongSegmentAlloc( zone, istrLength )) ){
      fprintf( stderr, "lv: exhausted SPARE stub table\n" );
      NotEnoughMemory();
    }
    return (i_str_t *)( (byte far *)ptr + sizeof( long ) );
  }

  stubIndex = istrLength - 1;
  if( stubIndex < 0 || stubIndex >= STR_SIZE )
    fprintf( stderr, "lv: istr size must be in range 0 .. STR_SIZE-1\n" ), exit( -1 );

  stubIndex = (int)stubIndexTable[ stubIndex ];
  stub = &stubTable[ zone ][ stubIndex ];

  if( NULL == (ptr = stub->ptr) ){
    if( NULL == (ptr = SegmentAlloc( zone, stubIndex )) ){
      if( stubIndex < STUB_SPARE ){
	if( NULL != (ptr = IstrAlloc( zone, stubTable[ zone ][ stubIndex + 1 ].blockSize )) )
	  return ptr;
      }
      fprintf( stderr, "lv: exhausted stub table\n" );
      NotEnoughMemory();
    }
    stub->segmentIndex++;
    ptr = (i_str_t *)( (byte far *)ptr + sizeof( long ) );
  }

  stub->ptr = *(i_str_t * far *)ptr;

  return ptr;
}

public void IstrFree( i_str_t *istr )
{
  stub_t *stub;
  i_str_t *ptr;
  int key, zone, stubIndex;

  key = *(int far *)( (byte far *)istr - sizeof( long ) );
  zone = key >> 8;
  stubIndex = key & 0x00ff;

  stub = &stubTable[ zone ][ stubIndex ];

  ptr = stub->ptr;
  stub->ptr = istr;

  *(i_str_t * far *)istr = ptr;
}

public void IstrFreeZone( int zone )
{
  int i, j;

  for( i = STUB_SPARE ; i >= 0 ; i-- ){
    stubTable[ zone ][ i ].ptr = NULL;
    if( 0 < stubTable[ zone ][ i ].segmentIndex ){
      for( j = stubTable[ zone ][ i ].segmentIndex - 1 ; j >= 0 ; j-- )
#ifdef MSDOS
        FarFree( stubTable[ zone ][ i ].segments[ j ] );
#else
	free( stubTable[ zone ][ i ].segments[ j ] );
#endif /* MSDOS */
      stubTable[ zone ][ i ].segmentIndex = 0;
    }
  }
}

public void IstrFreeAll()
{
  int zone;

  for( zone = 0 ; zone < ZONE_SIZE ; zone++ )
    IstrFreeZone( zone );
}
