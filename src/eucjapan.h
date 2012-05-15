/*
 * eucjapan.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: eucjapan.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __EUCJAPAN_H__
#define __EUCJAPAN_H__

#include <itable.h>

public void EncodeEUCjp( i_str_t *istr, int head, int tail,
			byte codingSystem, boolean_t binary );

#endif /* __EUCJAPAN_H__ */
