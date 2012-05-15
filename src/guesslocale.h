/*
 * guesslocale.h
 *
 * All rights reserved. Copyright (C) 2003 by Tomohiro KUBOTA
 * $Id: guesslocale.h,v 1.1 2003/11/06 06:53:52 nrt Exp $
 */

#ifndef __GUESSLOCALE_H__
#define __GUESSLOCALE_H__

public byte LocaleCodingSystem( char *language );
public byte DetermineEUC( char *language, char defaultEuc );
public byte Determine8bit( char *language );

#endif /* __GUESSLOCALE_H__ */
