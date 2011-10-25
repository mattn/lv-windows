/*
 * display.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <file.h>

public boolean_t smooth_paging;

public void DisplayFull( file_t *f );
public void DisplayTop( file_t *f, int arg );
public void DisplayBot( file_t *f, int arg );

#endif /* __DISPLAY_H__ */
