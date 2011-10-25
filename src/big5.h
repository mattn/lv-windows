/*
 * big5.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __BIG5_H__
#define __BIG5_H__

#include <itable.h>
#include <ctable.h>

public boolean_t allow_unify_big5;

public ic_t BIG5toCNS( ic_t big5, char *cset );
public ic_t CNStoBIG5( ic_t cns, char *cset );

public void DecodeBig5( state_t *state, char codingSystem );

public void EncodeBig5( i_str_t *istr, int head, int tail,
		       char codingSystem, boolean_t binary );

#endif /* __BIG5_H__ */
