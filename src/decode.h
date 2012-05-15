/*
 * decode.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: decode.h,v 1.5 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __DECODE_H__
#define __DECODE_H__

#include <itable.h>
#include <ctable.h>
#include <istr.h>
#include <str.h>

/*
 * ISO2022 & ISO2375 style escape sequence
 */

#define IsIchar( c )		( (c) >= 0x20 && (c) <= 0x2f )
#define IsFchar( c )		( (c) >= 0x30 && (c) <= 0x7e )

#define IsKatakana( c )							\
  ( (c) >= 0x21 && (c) <= 0x5f )

#define IsKatakanaByte( c )						\
  ( (c) >= 0xa1 && (c) <= 0xdf )

#define IsShiftJisByte1( c )						\
  ( ( (c) >= 0x81 && (c) <= 0x9f ) || ( (c) >= 0xe0 && (c) <= 0xfc ) )

#define IsShiftJisByte2( c )						\
  ( ( (c) >= 0x40 && (c) <= 0x7e ) || ( (c) >= 0x80 && (c) <= 0xfc ) )

#define IsBig5Byte1( c )						\
  ( ( (c) >= 0xa1 && (c) <= 0xfe ) )

#define IsBig5Byte2( c )						\
  ( ( (c) >= 0x40 && (c) <= 0x7e ) || ( (c) >= 0xa1 && (c) <= 0xfe ) )

#define IsEucByte( c )							\
  ( (c) >= 0xa1 && (c) <= 0xfe )

#define IsGLchar( c )							\
  ( (c) >= SP && (c) <= DEL )

#define IsGRchar( c )							\
  ( (c) >= 0xa0 && (c) <= 0xff )

#define IsGraphicChar94( c )						\
  ( (c) > SP && (c) < DEL )

#define IsGraphicChar( cset, c )					\
  ( TRUE == iTable[ (int)(cset) ].set94 ?				\
   ( (c) > SP && (c) < DEL ) : ( (c) >= SP && (c) <= DEL ) )

public int ISIDX, SIDX, SHIGH;
public i_str_t *ISTR;
public byte *STR;

#define GetChar( c )							\
{									\
  if( SHIGH == SIDX ){							\
    /*									\
     * BREAK for EXTERNAL LOOP						\
     */									\
    break;								\
  }									\
  (c) = STR[ SIDX++ ];							\
}

#define GetCharRet( c )							\
{									\
  if( SHIGH == SIDX )							\
    return FALSE;							\
  (c) = STR[ SIDX++ ];							\
}

#define IncStringIndex()						\
{									\
  if( SHIGH != SIDX ){							\
    SIDX++;								\
  }									\
}

#define DecodeAddChar( cset, s, attr )					\
{									\
  if( TRUE == iTable[ (int)(cset) ].multi )				\
    DecodeAddIchar( (cset), MakeIchar( (s)[ 0 ], (s)[ 1 ] ), (attr) );	\
  else									\
    DecodeAddIchar( (cset), (ic_t)(s)[ 0 ], (attr) );			\
}

#define CSET( region )	(state->cset[ (int)state->gset[ (int)(region) ] ])
#define SSET		(state->cset[ (int)state->sset ])

public boolean_t binary_decode;
public boolean_t hz_detection;
public int decoding_penalty;

public void DecodeAddLineFeed( byte ch );
public void DecodeAddSpace( byte attr );
public void DecodeAddTab( byte attr );
public void DecodeAddControl( byte ch );
public void DecodeAddBs();

public void DecodeAddIchar( byte charset, ic_t ic, byte attr );

public boolean_t DecodeAddShifted( state_t *state, byte ch );

public i_str_t *DecodeSimple( i_str_t *istr, byte *str, int *shigh );
public i_str_t *Decode( i_str_t *istr,
		       byte codingSystem, byte *str, int *shigh );

public void DecodeInit();

#endif /* __DECODE_H__ */
