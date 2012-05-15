/*
 * utf.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: utf.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __UTF_H__
#define __UTF_H__

#include <itable.h>
#include <ctable.h>

#define IsUtfEncoding( cset )	( UTF_7 == (cset) || UTF_8 == (cset) )

public void DecodeUTF( state_t *state, byte codingSystem );

public void EncodeUTF7( i_str_t *istr, int head, int tail,
		       byte codingSystem, boolean_t binary );

public void EncodeUTF8( i_str_t *istr, int head, int tail,
		       byte codingSystem, boolean_t binary );

#endif /* __UTF_H__ */
