/*
 * unimap.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __UNIMAP_H__
#define __UNIMAP_H__

#include <ichar.h>

public boolean_t unimap_iso8859;

public ic_t UNItoBIG5( ic_t ic, char *cset );
public ic_t UNItoCNS( ic_t ic, char *cset );
public ic_t UNItoGB( ic_t ic, char *cset );
public ic_t UNItoJIS( ic_t ic, char *cset );
public ic_t UNItoKSC( ic_t ic, char *cset );

#endif /* __UNIMAP_H__ */
