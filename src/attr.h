/*
 * attr.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __ATTR_H__
#define __ATTR_H__

#define ATTR_NULL		0x00

#define ATTR_COLOR		0x07
#define ATTR_COLOR_R		0x01
#define ATTR_COLOR_B		0x02
#define ATTR_COLOR_G		0x04

#define ATTR_HILIGHT		0x08
#define ATTR_UNDERLINE		0x10
#define ATTR_BLINK		0x20
#define ATTR_REVERSE		0x40

#define ATTR_STANDOUT		0x80	/* INTERNAL USE */

#endif /* __ATTR_H__ */
