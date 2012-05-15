/*
 * iso2jp.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: iso2jp.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __ISO2JP_H__
#define __ISO2JP_H__

#include <itable.h>
#include <ctable.h>

public void EncodeISO2022jp( i_str_t *istr, int head, int tail,
			    byte codingSystem, boolean_t binary );

#endif /* __ISO2JP_H__ */
