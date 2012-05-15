/*
 * display.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: display.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <file.h>

public boolean_t smooth_paging;
public boolean_t carefully_divide;

public void DisplayFull( file_t *f );
public void DisplayTop( file_t *f, int arg );
public void DisplayBot( file_t *f, int arg );

#endif /* __DISPLAY_H__ */
