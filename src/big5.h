/*
 * big5.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: big5.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __BIG5_H__
#define __BIG5_H__

#include <itable.h>
#include <ctable.h>

public ic_t BIG5toCNS( ic_t big5, byte *cset );
public ic_t CNStoBIG5( ic_t cns, byte *cset );

public void ConvertCNStoBIG5( i_str_t *istr );
public void ConvertBIG5toCNS( i_str_t *istr );

public void DecodeBig5( state_t *state, byte codingSystem );

public void EncodeBig5( i_str_t *istr, int head, int tail,
		       byte codingSystem, boolean_t binary );

#endif /* __BIG5_H__ */
