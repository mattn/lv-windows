/*
 * find.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: find.h,v 1.4 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __FIND_H__
#define __FIND_H__

#include <boolean.h>
#include <itable.h>
#include <file.h>

typedef ic_t (*tolower_func_t)( byte, ic_t );
public tolower_func_t to_lower_func;

typedef boolean_t (*find_func_t)( line_t * );
public find_func_t find_func;

typedef boolean_t (*find_continue_func_t)( i_str_t * );
public find_continue_func_t find_only_func;

public boolean_t casefold_search;
public boolean_t regexp_search;

#define ToLower( cset, c )						\
  ( FALSE == casefold_search ? (c)					\
    : ( 'Z' < (c) || 'A' > (c) || ASCII != (cset) ) ? (c)		\
        : (c) + 0x20 )

public void FindSetup();

public byte *FindResetPattern( file_t *f, i_str_t *istr );
public byte *FindSetPattern( file_t *f, i_str_t *istr );
public boolean_t FindClearPattern( file_t *f );

public int FindForward( file_t *f );
public int FindBackward( file_t *f );

#endif /* __FIND_H__ */
