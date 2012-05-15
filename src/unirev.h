/*
 * unirev.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: unirev.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __UNIREV_H__
#define __UNIREV_H__

#include <ichar.h>

public ic_t RevUNI( ic_t ic, byte *cset );
public void ConvertToUNI( i_str_t *istr );

#endif /* __UNIREV_H__ */
