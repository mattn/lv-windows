/*
 * fetch.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: fetch.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __FETCH_H__
#define __FETCH_H__

#include <file.h>

public boolean_t line_truncated;

#define LineHead( line, index )						\
  ( 0 == (index) ? 0 : (line)->head[ (index)-1 ].ptr )

#define LineLength( line, index )					\
  ( LineHead( (line), (index)+1 ) - LineHead( (line), (index) ) )

#define LineWidth( line, index )					\
  ( (line)->head[ (index) ].width )

public boolean_t FetchLine( file_t *f, unsigned int segment, int offset );

#endif /* __FETCH_H__ */
