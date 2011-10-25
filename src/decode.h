/*
 * decode.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
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
public char *STR;

#define GetChar( c )							\
{									\
  if( SHIGH == SIDX ){							\
    /*									\
     * BREAK for EXTERNAL LOOP						\
     */									\
    break;								\
  }									\
  (c) = STR[ SIDX++ ];							\
  if( LF == (c) )							\
    /*									\
     * BREAK for EXTERNAL LOOP						\
     */									\
    break;								\
}

#define GetCharRet( c )							\
{									\
  if( SHIGH == SIDX )							\
    return FALSE;							\
  (c) = STR[ SIDX++ ];							\
  if( LF == (c) )							\
    return FALSE;							\
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

public void DecodeAddSpace( char attr );
public void DecodeAddTab( char attr );
public void DecodeAddControl( char ch );
public void DecodeAddBs();

public void DecodeAddIchar( char charset, ic_t ic, char attr );

public boolean_t DecodeAddShifted( state_t *state, char ch );

public i_str_t *DecodeSimple( char *str, int *shigh );
public i_str_t *Decode( char codingSystem, char *str, int *shigh );

#endif /* __DECODE_H__ */
