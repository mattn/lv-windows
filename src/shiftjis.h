/*
 * shiftjis.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __SHIFTJIS_H__
#define __SHIFTJIS_H__

#include <itable.h>
#include <ctable.h>

public void DecodeShiftJis( state_t *state, char codingSystem );

public void EncodeShiftJis( i_str_t *istr, int head, int tail,
			   char codingSystem, boolean_t binary );

#endif /* __SHIFTJIS_H__ */
