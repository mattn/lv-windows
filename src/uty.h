/*
 * uty.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __UTY_H__
#define __UTY_H__

#include <ichar.h>

typedef struct {
  ic_t code, peer;
} codes_t;

typedef struct {
  ic_t code, peer;
  char cset;
} codes_cset_t;

public ic_t BinarySearch( codes_t *array, int high, ic_t code );
public ic_t BinarySearchCset( codes_cset_t *array, int high, ic_t code,
			     char *cset );

public void NotEnoughMemory();
public void FatalErrorOccurred();

public void *Malloc( unsigned int size );
public char *TokenAlloc( char *s );

#ifdef MSDOS
public void far *FarMalloc( unsigned int size );
public void FarFree( void far *ptr );
#endif /* MSDOS */

public boolean_t IsAtty( int fd );
public char *Exts( char *s );

#endif /* __UTY_H__ */
