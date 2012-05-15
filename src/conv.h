/*
 * conv.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: conv.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __CONV_H__
#define __CONV_H__

#include <file.h>

public boolean_t grep_mode;
public boolean_t grep_inverted;
public boolean_t line_number;

public boolean_t Conv( file_t *f, boolean_t showFileName );

#endif /* __CONV_H__ */
