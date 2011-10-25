/*
 * raw.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __RAW_H__
#define __RAW_H__

#include <itable.h>

public void DecodeRaw( state_t *state, char codingSystem );

public void EncodeRaw( i_str_t *istr, int head, int tail,
		      char codingSystem, boolean_t binary );

#endif /* __RAW_H__ */
