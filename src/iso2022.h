/*
 * iso2022.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __ISO2022_H__
#define __ISO2022_H__

#include <itable.h>
#include <ctable.h>

public void DecodeISO2022( state_t *state, char codingSystem );

#endif /* __ISO2022_H__ */
