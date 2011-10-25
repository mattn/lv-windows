/*
 * encode.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __ENCODE_H__
#define __ENCODE_H__

#include <itable.h>
#include <ctable.h>
#include <istr.h>
#include <str.h>

#define CODE_SIZE	STR_SIZE

public int CIDX;
public str_t *CSTR;

public int encode_length;
public str_t encode_str[ CODE_SIZE ];

#define EncodeAddCharAbsolutely( a, c )					\
{									\
  CSTR[ CIDX++ ] = (a) | (c);						\
}

#define EncodeAddChar( a, c )						\
{									\
  CSTR[ CIDX++ ] = (a) | (c);						\
  if( CIDX >= ( CODE_SIZE - 16 ) )					\
    /*									\
     * BREAK for EXTERNAL LOOP						\
     */									\
    break;								\
}

#define EncodeAddCharRet( a, c )					\
{									\
  CSTR[ CIDX++ ] = (a) | (c);						\
  if( CIDX >= ( CODE_SIZE - 16 ) )					\
    return FALSE;							\
}

public boolean_t EncodeAddPseudo( int attr, ic_t ic, char cset,
				 boolean_t binary );
public boolean_t EncodeAddInvalid( int attr, ic_t ic, char cset );

public void EncodeAddEscapeDollar( char attr );
public void EncodeAdd7bitSS2( char attr );
public void EncodeAdd7bitSS3( char attr );

public void Encode( i_str_t *istr, int head, int tail,
		   char codingSystem, boolean_t binary,
		   str_t *code, int *length );

#endif /* __ENCODE_H__ */
