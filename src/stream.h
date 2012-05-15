/*
 * stream.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: stream.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __STREAM_H__
#define __STREAM_H__

#include <stdio.h>

typedef struct {
  FILE *fp;
  FILE *sp;
  int  pid;
} stream_t;

public stream_t *StreamOpen( byte *file );
public boolean_t StreamClose( stream_t *st );
public stream_t *StreamReconnectStdin();

#endif /* __STREAM_H__ */
