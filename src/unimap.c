/*
 * unimap.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: unimap.c,v 1.7 2003/11/13 03:08:19 nrt Exp $
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
#include <iso88592.map>
#include <iso88593.map>
#include <iso88594.map>
#include <iso88595.map>
#include <iso88596.map>
#include <iso88597.map>
#include <iso88598.map>
#include <iso88599.map>
#include <iso885910.map>
#include <iso885911.map>
#include <iso885913.map>
#include <iso885914.map>
#include <iso885915.map>
#include <iso885916.map>

typedef struct {
  codes_t	*map;
  int		size;
} iso8859_map_table_t;

private iso8859_map_table_t mapTable[ ISO8859_16 - ISO8859_1 ] = {
  { mapISO8859_2, 95 },	/* ISO8859_2 */
  { mapISO8859_3, 88 },	/* ISO8859_3 */
  { mapISO8859_4, 95 },	/* ISO8859_4 */
  { mapISO8859_5, 95 },	/* ISO8859_5 */
  { mapISO8859_6, 50 },	/* ISO8859_6 */
  { mapISO8859_7, 91 },	/* ISO8859_7 */
  { mapISO8859_8, 57 },	/* ISO8859_8 */
  { mapISO8859_9, 95 },	/* ISO8859_9 */
  { mapISO8859_10, 87 },/* ISO8859_10 */
  { mapISO8859_11, 87 },/* ISO8859_11 */
  { mapISO8859_13, 95 },/* ISO8859_13 */
  { mapISO8859_14, 95 },/* ISO8859_14 */
  { mapISO8859_15, 95 },/* ISO8859_15 */
  { mapISO8859_16, 95 }	/* ISO8859_16 */
};
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

public ic_t UNItoISO8859( ic_t ic, byte *cset, byte codingSystem )
{
#ifdef UNI_TO_ISO8859
  ic_t res;

  if( ISO_8859_1 == codingSystem ){
    if( ic <= 0xff && ic >= 0xa0 ){
      *cset = ISO8859_1;
      return ic & 0x7f;
    }
  } else if( ISO_8859_2 <= codingSystem && codingSystem <= ISO_8859_9 ){
    if( 0 < (res = BinarySearch( mapTable[ codingSystem - ISO_8859_2 ].map,
				mapTable[ codingSystem - ISO_8859_2 ].size, ic )) ){
      *cset = ISO8859_2 + ( codingSystem - ISO_8859_2 );
      return res;
    }
  }
#endif

  *cset = ASCII;
  return (ic_t)'?';
}

private ic_t UNItoISO8859_any( ic_t ic, byte *cset )
{
#ifdef UNI_TO_ISO8859
  ic_t res;
  int plane;

  if( ic <= 0xff && ic >= 0xa0 ){
    *cset = ISO8859_1;
    return ic & 0x7f;
  }

  for( plane = ISO8859_2 ; plane <= ISO8859_16 ; plane++ ){
    if( 0 < (res = BinarySearch( mapTable[ plane - ISO8859_2 ].map,
				mapTable[ plane - ISO8859_2 ].size, ic )) ){
      *cset = plane;
      return res;
    }
  }
#endif

  return 0;
}

public ic_t UNItoBIG5( ic_t ic, byte *cset )
{
#ifdef UNI_TO_BIG5
  ic_t res;

/*
  if( TRUE == unimap_iso8859 && ic <= 0x2116 ){
    if( 0 < (res = UNItoISO8859_any( ic, cset )) )
      return res;
  }
*/

  if( 0 < (res = BinarySearch( mapBIG5, 13702, ic )) ){
    *cset = BIG5;
    return res;
  }
#endif

  *cset = ASCII;
  return (ic_t)'?';
}

public ic_t UNItoGB( ic_t ic, byte *cset )
{
#ifdef UNI_TO_GB
  ic_t res;

  if( TRUE == unimap_iso8859 && ic <= 0x2116 ){
    if( 0 < (res = UNItoISO8859_any( ic, cset )) )
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

public ic_t UNItoCNS( ic_t ic, byte *cset )
{
#ifdef UNI_TO_CNS
  ic_t res;

  if( TRUE == unimap_iso8859 && ic <= 0x2116 ){
    if( 0 < (res = UNItoISO8859_any( ic, cset )) )
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

public ic_t UNItoJIS( ic_t ic, byte *cset )
{
#ifdef UNI_TO_JIS
  ic_t res;

  if( TRUE == unimap_iso8859 && ic <= 0x2116 ){
    if( 0 < (res = UNItoISO8859_any( ic, cset )) )
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

public ic_t UNItoKSC( ic_t ic, byte *cset )
{
#ifdef UNI_TO_KSC
  ic_t res;

  if( TRUE == unimap_iso8859 && ic <= 0x2116 ){
    if( 0 < (res = UNItoISO8859_any( ic, cset )) )
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

public ic_t UNItoChinese( ic_t ic, byte *cset )
{
  ic_t res;
  byte cs;

  cs = *cset;
  res = UNItoGB( ic, &cs );
  if( ASCII == cs ){
    cs = *cset;
    res = UNItoCNS( ic, &cs );
/*
    if( ASCII == cs ){
      cs = *cset;
      res = UNItoKSC( ic, &cs );
      if( ASCII == cs ){
	cs = *cset;
	res = UNItoJIS( ic, &cs );
      }
    }
*/
  }
  *cset = cs;

  return res;
}

public ic_t UNItoJapanese( ic_t ic, byte *cset )
{
  ic_t res;
  byte cs;

  cs = *cset;
  res = UNItoJIS( ic, &cs );
  if( ASCII == cs ){
    cs = *cset;
    res = UNItoGB( ic, &cs );
    if( ASCII == cs ){
      cs = *cset;
      res = UNItoCNS( ic, &cs );
      if( ASCII == cs ){
	cs = *cset;
	res = UNItoKSC( ic, &cs );
      }
    }
  }

  *cset = cs;

  return res;
}

public ic_t UNItoKorean( ic_t ic, byte *cset )
{
  ic_t res;
  byte cs;

  cs = *cset;
  res = UNItoKSC( ic, &cs );
/*
  if( ASCII == cs ){
    cs = *cset;
    res = UNItoGB( ic, &cs );
    if( ASCII == cs ){
      cs = *cset;
      res = UNItoCNS( ic, &cs );
      if( ASCII == cs ){
	cs = *cset;
	res = UNItoJIS( ic, &cs );
      }
    }
  }
*/
  *cset = cs;

  return res;
}

public void ConvertFromUNI( i_str_t *istr, byte codingSystem )
{
  int i;

  switch( codingSystem ){
  case ISO_2022_CN:
  case EUC_CHINA:
  case HZ_GB:
    for( i = 0 ; NOSET != istr[ i ].charset ; i++ )
      if( UNICODE == istr[ i ].charset )
	istr[ i ].c = UNItoChinese( istr[ i ].c, &istr[ i ].charset );
    break;
  case ISO_2022_JP:
  case EUC_JAPAN:
  case SHIFT_JIS:
    for( i = 0 ; NOSET != istr[ i ].charset ; i++ )
      if( UNICODE == istr[ i ].charset )
	istr[ i ].c = UNItoJapanese( istr[ i ].c, &istr[ i ].charset );
    break;
  case ISO_2022_KR:
  case EUC_KOREA:
    for( i = 0 ; NOSET != istr[ i ].charset ; i++ )
      if( UNICODE == istr[ i ].charset )
	istr[ i ].c = UNItoKorean( istr[ i ].c, &istr[ i ].charset );
    break;
  case BIG_FIVE:
    for( i = 0 ; NOSET != istr[ i ].charset ; i++ )
      if( UNICODE == istr[ i ].charset )
	istr[ i ].c = UNItoBIG5( istr[ i ].c, &istr[ i ].charset );
    break;
  case EUC_TAIWAN:
    for( i = 0 ; NOSET != istr[ i ].charset ; i++ )
      if( UNICODE == istr[ i ].charset )
	istr[ i ].c = UNItoCNS( istr[ i ].c, &istr[ i ].charset );
    break;
  }
}
