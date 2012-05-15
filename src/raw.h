/*
 * raw.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: raw.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __RAW_H__
#define __RAW_H__

#include <itable.h>

public void DecodeRaw( state_t *state, byte codingSystem );

public void EncodeRaw( i_str_t *istr, int head, int tail,
		      byte codingSystem, boolean_t binary );

#endif /* __RAW_H__ */
