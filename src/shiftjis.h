/*
 * shiftjis.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: shiftjis.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __SHIFTJIS_H__
#define __SHIFTJIS_H__

#include <itable.h>
#include <ctable.h>

public void DecodeShiftJis( state_t *state, byte codingSystem );

public void EncodeShiftJis( i_str_t *istr, int head, int tail,
			   byte codingSystem, boolean_t binary );

#endif /* __SHIFTJIS_H__ */
