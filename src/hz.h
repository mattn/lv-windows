/*
 * hz.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: hz.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __HZ_H__
#define __HZ_H__

#include <itable.h>

public void EncodeHz( i_str_t *istr, int head, int tail,
		     byte codingSystem, boolean_t binary );

#endif /* __HZ_H__ */
