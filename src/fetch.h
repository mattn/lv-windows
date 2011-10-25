/*
 * fetch.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
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
