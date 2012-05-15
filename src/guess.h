/*
 * guess.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: guess.h,v 1.5 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __GUESS_H__
#define __GUESS_H__

#include <ichar.h>

public boolean_t adjust_charset;

public byte GuessCodingSystem( byte *str, int length, byte defaultEuc );
public byte GuessHz( byte *str, int length );
public void AdjustPatternCharset( byte inputCodingSystem,
				 byte keyboardCodingSystem,
				 byte defaultCodingSystem,
				 i_str_t *istr );

#endif /* __GUESS_H__ */
