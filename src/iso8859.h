/*
 * iso8859.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: iso8859.h,v 1.2 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __ISO8859_H__
#define __ISO8859_H__

#include <itable.h>
#include <ctable.h>

public void EncodeISO8859( i_str_t *istr, int head, int tail,
			  byte codingSystem, boolean_t binary );

#endif /* __ISO8859_H__ */
