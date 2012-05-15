/*
 * unirev.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: unirev.c,v 1.7 2004/01/05 07:23:29 nrt Exp $
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
#include <unirev.h>

#define ISO8859_TO_UNI
#define BIG5_TO_UNI
#define GB_TO_UNI
#define JIS_TO_UNI
#define KSC_TO_UNI

#ifdef ISO8859_TO_UNI
#include <iso88592.rev>
#include <iso88593.rev>
#include <iso88594.rev>
#include <iso88595.rev>
#include <iso88596.rev>
#include <iso88597.rev>
#include <iso88598.rev>
#include <iso88599.rev>
#include <iso885910.rev>
#include <iso885911.rev>
#include <iso885913.rev>
#include <iso885914.rev>
#include <iso885915.rev>
#include <iso885916.rev>
#endif

#ifdef BIG5_TO_UNI
#include <big5.rev>
#endif

#ifdef GB_TO_UNI
#include <gb2312.rev>
#endif

#ifdef JIS_TO_UNI
#include <jis0208.rev>
#include <jis0212.rev>
#endif

#ifdef KSC_TO_UNI
#include <ksc5601.rev>
#endif

private ic_t ISO8859_1toUNI( ic_t ic, byte *cset )
{
  *cset = UNICODE;
  return ic | 0x80;
}

private ic_t CNStoUNI( ic_t ic, byte *cset )
{
  ic_t res;

  res = CNStoBIG5( ic, cset );
  if( BIG5 == *cset )
    return RevUNI( res, cset );
  else
    return res;
}

private ic_t ASCIItoUNI( ic_t ic, byte *cset )
{
  *cset = ASCII;
  return ic;
}

private ic_t JIS0201ROMANtoUNI( ic_t ic, byte *cset )
{
  if( ic == 0x5c ){
    *cset = UNICODE;
    return (ic_t)0x00a5;
  } else if( ic == 0x7e ){
    *cset = UNICODE;
    return (ic_t)0x203e;
  }
  *cset = ASCII;
  return ic;
}

private ic_t JIS0201KANAtoUNI( ic_t ic, byte *cset )
{
  *cset = UNICODE;
  return ic + 0xff40U;
}

typedef ic_t (*uniconv_t)( ic_t ic, byte *cset );

typedef struct {
  ic_t		*directmap;
  codes_t	*map;
  int		size;
  uniconv_t	conv;
} rev_table_t;

private rev_table_t revTable[ PSEUDO ] = {
  { NULL, NULL, 0, ASCIItoUNI },		/* ASCII */
  { NULL, NULL, 0, JIS0201ROMANtoUNI },		/* X0201ROMAN */
  { NULL, NULL, 0, JIS0201KANAtoUNI },		/* X0201KANA */
  { NULL, NULL, 0, ISO8859_1toUNI },		/* ISO8859_1 */
#ifdef ISO8859_TO_UNI
  { revISO8859_2, NULL, 0, NULL },		/* ISO8859_2 */
  { revISO8859_3, NULL, 0, NULL },		/* ISO8859_3 */
  { revISO8859_4, NULL, 0, NULL },		/* ISO8859_4 */
  { revISO8859_5, NULL, 0, NULL },		/* ISO8859_5 */
  { revISO8859_6, NULL, 0, NULL },		/* ISO8859_6 */
  { revISO8859_7, NULL, 0, NULL },		/* ISO8859_7 */
  { revISO8859_8, NULL, 0, NULL },		/* ISO8859_8 */
  { revISO8859_9, NULL, 0, NULL },		/* ISO8859_9 */
  { revISO8859_10, NULL, 0, NULL },		/* ISO8859_10 */
  { revISO8859_11, NULL, 0, NULL },		/* ISO8859_11 */
  { revISO8859_13, NULL, 0, NULL },		/* ISO8859_13 */
  { revISO8859_14, NULL, 0, NULL },		/* ISO8859_14 */
  { revISO8859_15, NULL, 0, NULL },		/* ISO8859_15 */
  { revISO8859_16, NULL, 0, NULL },		/* ISO8859_16 */
#else
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
  { NULL, NULL, 0, NULL },
#endif
#ifdef JIS_TO_UNI
  { NULL, revJIS0208, 6878, NULL },		/* C6226 */
#else
  { NULL, NULL, 0, NULL },
#endif
#ifdef GB_TO_UNI
  { NULL, revGB2312, 7444, NULL },		/* GB2312 */
#else
  { NULL, NULL, 0, NULL },
#endif
#ifdef JIS_TO_UNI
  { NULL, revJIS0208, 6878, NULL },		/* X0208 */
#else
  { NULL, NULL, 0, NULL },
#endif
#ifdef KSC_TO_UNI
  { NULL, revKSC5601, 8223, NULL },		/* KSC5601 */
#else
  { NULL, NULL, 0, NULL },
#endif
#ifdef JIS_TO_UNI
  { NULL, revJIS0212, 6066, NULL },		/* X0212 */
#else
  { NULL, NULL, 0, NULL },
#endif
  { NULL, NULL, 0, NULL },			/* ISO_IR_165 */
  { NULL, NULL, 0, CNStoUNI },			/* CNS_1 */
  { NULL, NULL, 0, CNStoUNI },			/* CNS_2 */
  { NULL, NULL, 0, CNStoUNI },			/* CNS_3 */
  { NULL, NULL, 0, CNStoUNI },			/* CNS_4 */
  { NULL, NULL, 0, NULL },			/* CNS_5 */
  { NULL, NULL, 0, NULL },			/* CNS_6 */
  { NULL, NULL, 0, NULL },			/* CNS_7 */
  { NULL, NULL, 0, NULL },			/* JIS X 0213-2000 Plane 1 */
  { NULL, NULL, 0, NULL },			/* JIS X 0213-2000 Plane 2 */
#ifdef BIG5_TO_UNI
  { NULL, revBIG5, 13702, NULL }		/* BIG5 */
#else
  { NULL, NULL, 0, NULL }
#endif
};

public ic_t RevUNI( ic_t ic, byte *cset )
{
  ic_t res;

  if( revTable[ (int)*cset ].map ){
    if( 0 < (res = BinarySearch( revTable[ (int)*cset ].map,
				       revTable[ (int)*cset ].size,
				       ic )) ){
      *cset = UNICODE;
      return res;
    }
  } else if( revTable[ (int)*cset ].conv ){
    return (*revTable[ (int)*cset ].conv)( ic, cset );
  } else if( revTable[ (int)*cset ].directmap ){
    if( 0x20 <= ic && ic < 0x80 ){
      if( (ic_t)0 != (res = revTable[ (int)*cset ].directmap[ ic - 0x20 ]) ){
	*cset = UNICODE;
	return res;
      }
    }
  }

  *cset = ASCII;
  return (ic_t)'?';
}

public void ConvertToUNI( i_str_t *istr )
{
  int i;

  for( i = 0 ; NOSET != istr[ i ].charset ; i++ ){
    if( istr[ i ].charset < UNICODE )
      istr[ i ].c = RevUNI( istr[ i ].c, &istr[ i ].charset );
  }
}
