/*
 * istr.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __ISTR_H__
#define __ISTR_H__

#include <ichar.h>

public int IstrWidth( i_str_t *istr );

public void IstrInit();

public i_str_t *IstrAlloc( int istrLength );
public void IstrFree( i_str_t *istr );
public void IstrFreeAll();

#endif /* __ISTR_H__ */
