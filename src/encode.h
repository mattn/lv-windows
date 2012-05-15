/*
 * encode.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: encode.h,v 1.5 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __ENCODE_H__
#define __ENCODE_H__

#include <itable.h>
#include <ctable.h>
#include <istr.h>
#include <str.h>

#define CODE_EXTRA_LEN	16
#define CODE_SIZE	( STR_SIZE + CODE_EXTRA_LEN )

public int CIDX;
public int CHIGH;
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
  if( CIDX >= CHIGH )							\
    /*									\
     * BREAK for EXTERNAL LOOP						\
     */									\
    break;								\
}

#define EncodeAddCharRet( a, c )					\
{									\
  CSTR[ CIDX++ ] = (a) | (c);						\
  if( CIDX >= CHIGH )							\
    return FALSE;							\
}

public boolean_t EncodeAddPseudo( int attr, ic_t ic, byte cset,
				 boolean_t binary );
public boolean_t EncodeAddInvalid( int attr, ic_t ic, byte cset );

public void EncodeAddEscapeDollar( int attr );
public void EncodeAdd7bitSS2( int attr );
public void EncodeAdd7bitSS3( int attr );

public void Encode( i_str_t *istr, int head, int tail,
		   byte codingSystem, boolean_t binary,
		   str_t *code, int *length );

public byte *EncodeStripAttribute( str_t *str, int length );

#endif /* __ENCODE_H__ */
