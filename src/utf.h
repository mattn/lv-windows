/*
 * utf.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __UTF_H__
#define __UTF_H__

#include <itable.h>
#include <ctable.h>

public void DecodeUTF( state_t *state, char codingSystem );

public void EncodeUTF7( i_str_t *istr, int head, int tail,
		       char codingSystem, boolean_t binary );

public void EncodeUTF8( i_str_t *istr, int head, int tail,
		       char codingSystem, boolean_t binary );

#endif /* __UTF_H__ */
