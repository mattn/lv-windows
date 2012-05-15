/*
 * uty.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: uty.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __UTY_H__
#define __UTY_H__

#include <ichar.h>

typedef struct {
  ic_t code, peer;
} codes_t;

typedef struct {
  ic_t code, peer;
  byte cset;
} codes_cset_t;

public ic_t BinarySearch( codes_t *array, int high, ic_t code );
public ic_t BinarySearchCset( codes_cset_t *array, int high, ic_t code,
			     byte *cset );

public void NotEnoughMemory();
public void FatalErrorOccurred();

public void *Malloc( unsigned int size );
public byte *TokenAlloc( byte *s );

#ifdef MSDOS
public void far *FarMalloc( unsigned int size );
public void FarFree( void far *ptr );
#endif /* MSDOS */

public boolean_t IsAtty( int fd );
public byte *Exts( byte *s );

#endif /* __UTY_H__ */
