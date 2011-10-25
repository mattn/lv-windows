/*
 * iso2jp.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __ISO2JP_H__
#define __ISO2JP_H__

#include <itable.h>
#include <ctable.h>

public void EncodeISO2022jp( i_str_t *istr, int head, int tail,
			    char codingSystem, boolean_t binary );

#endif /* __ISO2JP_H__ */
