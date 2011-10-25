/*
 * unimap.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdio.h>

#include <import.h>
#include <itable.h>
#include <uty.h>
#include <big5.h>
#include <begin.h>
#include <unimap.h>

#define UNI_TO_ISO8859
#define UNI_TO_BIG5
#define UNI_TO_GB
#define UNI_TO_CNS
#define UNI_TO_JIS
#define UNI_TO_KSC

#ifdef UNI_TO_ISO8859
#include <iso8859.map>
#endif

#ifdef UNI_TO_BIG5
#include <big5.map>
#endif

#ifdef UNI_TO_GB
#include <gb2312.map>
#endif

#ifdef UNI_TO_JIS
#include <jis.map>
#endif

#ifdef UNI_TO_KSC
#include <ksc5601.map>
#endif

private ic_t UNItoISO8859( ic_t ic, char *cset )
{
#ifdef UNI_TO_ISO8859
  ic_t res;

  if( ic <= 0xff && ic >= 0xa0 ){
    *cset = ISO8859_1;
    return ic & 0x7f;
  } else if( 0 < (res = BinarySearchCset( mapISO8859, 357, ic, cset )) )
    return res;
#endif

  return 0;
}

public ic_t UNItoBIG5( ic_t ic, char *cset )
{
#ifdef UNI_TO_BIG5
  ic_t res;

  if( TRUE == unimap_iso8859 && ic <= 0x2116 ){
    if( 0 < (res = UNItoISO8859( ic, cset )) )
      return res;
  }

  if( 0 < (res = BinarySearch( mapBIG5, 13702, ic )) ){
    *cset = BIG5;
    return res;
  }
#endif

  *cset = ASCII;
  return (ic_t)'?';
}

public ic_t UNItoGB( ic_t ic, char *cset )
{
#ifdef UNI_TO_GB
  ic_t res;

  if( TRUE == unimap_iso8859 && ic <= 0x2116 ){
    if( 0 < (res = UNItoISO8859( ic, cset )) )
      return res;
  }

  if( 0 < (res = BinarySearch( mapGB2312, 7443, ic )) ){
    *cset = GB2312;
    return res;
  }
#endif

  *cset = ASCII;
  return (ic_t)'?';
}

public ic_t UNItoCNS( ic_t ic, char *cset )
{
#ifdef UNI_TO_CNS
  ic_t res;

  if( TRUE == unimap_iso8859 && ic <= 0x2116 ){
    if( 0 < (res = UNItoISO8859( ic, cset )) )
      return res;
  }

  res = UNItoBIG5( ic, cset );
  if( BIG5 == *cset )
    return BIG5toCNS( res, cset );
  else
    return res;
#else
  *cset = ASCII;
  return (ic_t)'?';
#endif
}

public ic_t UNItoJIS( ic_t ic, char *cset )
{
#ifdef UNI_TO_JIS
  ic_t res;

  if( TRUE == unimap_iso8859 && ic <= 0x2116 ){
    if( 0 < (res = UNItoISO8859( ic, cset )) )
      return res;
  }

  if( 0 < (res = BinarySearchCset( mapJIS, 12945, ic, cset )) )
    return res;

  if( 0x00a5 == ic ){
    *cset = X0201ROMAN;
    return (ic_t)0x5c;
  } else if( 0x203e == ic ){
    *cset = X0201ROMAN;
    return (ic_t)0x7e;
  } else if( ic >= 0xff61U && ic <= 0xff9fU ){
    *cset = X0201KANA;
    return ic - 0xff40U;
  }
#endif

  *cset = ASCII;
  return (ic_t)'?';
}

public ic_t UNItoKSC( ic_t ic, char *cset )
{
#ifdef UNI_TO_KSC
  ic_t res;

  if( TRUE == unimap_iso8859 && ic <= 0x2116 ){
    if( 0 < (res = UNItoISO8859( ic, cset )) )
      return res;
  }

  if( 0 < (res = BinarySearch( mapKSC5601, 8223, ic )) ){
    *cset = KSC5601;
    return res;
  }
#endif

  *cset = ASCII;
  return (ic_t)'?';
}
