/*
 * unimap.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: unimap.h,v 1.4 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __UNIMAP_H__
#define __UNIMAP_H__

#include <ichar.h>

public boolean_t unimap_iso8859;

public ic_t UNItoISO8859( ic_t ic, byte *cset, byte codingSystem );

public ic_t UNItoBIG5( ic_t ic, byte *cset );
public ic_t UNItoCNS( ic_t ic, byte *cset );
public ic_t UNItoGB( ic_t ic, byte *cset );
public ic_t UNItoJIS( ic_t ic, byte *cset );
public ic_t UNItoKSC( ic_t ic, byte *cset );

public ic_t UNItoChinese( ic_t ic, byte *cset );
public ic_t UNItoJapanese( ic_t ic, byte *cset );
public ic_t UNItoKorean( ic_t ic, byte *cset );

public void ConvertFromUNI( i_str_t *istr, byte codingSystem );

#endif /* __UNIMAP_H__ */
