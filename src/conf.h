/*
 * conf.h
 *
 * All rights reserved. Copyright (C) 1998 by NARITA Tomio
 * $Id: conf.h,v 1.4 2003/11/13 03:30:49 nrt Exp $
 */

#ifndef __CONF_H__
#define __CONF_H__

#include <stream.h>

typedef struct {
  byte		**file;
  stream_t	*st;
  int		width;
  int		height;
  boolean_t	options;
  byte		inputCodingSystem;
  byte		outputCodingSystem;
  byte		keyboardCodingSystem;
  byte		pathnameCodingSystem;
  byte		defaultCodingSystem;
  boolean_t	keyCodingSystemVirgin;
  byte		*pattern;
} conf_t;

public void ConfInit( byte **argv );
public void Conf( conf_t *conf, byte **argv );
public byte *ConfFilename( conf_t *conf );

#endif /* __CONF_H__ */
