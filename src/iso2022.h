/*
 * iso2022.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: iso2022.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __ISO2022_H__
#define __ISO2022_H__

#include <itable.h>
#include <ctable.h>

public void DecodeISO2022( state_t *state, byte codingSystem );

#endif /* __ISO2022_H__ */
