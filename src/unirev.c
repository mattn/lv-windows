/*
 * unirev.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

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

private ic_t ISO8859toUNI( ic_t ic, char *cset )
{
  *cset = UNICODE;
  return ic | 0x80;
}

private ic_t CNStoUNI( ic_t ic, char *cset )
{
  ic_t res;

  res = CNStoBIG5( ic, cset );
  if( BIG5 == *cset )
    return RevUNI( res, cset );
  else
    return res;
}

private ic_t JIS0201ROMANtoUNI( ic_t ic, char *cset )
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

private ic_t JIS0201KANAtoUNI( ic_t ic, char *cset )
{
  *cset = UNICODE;
  return ic + 0xff40U;
}

typedef ic_t (*uniconv_t)( ic_t ic, char *cset );

typedef struct {
  codes_t	*map;
  int		size;
  uniconv_t	conv;
} rev_table_t;

private rev_table_t revTable[ PSEUDO ] = {
  { NULL, 0, NULL },				/* ASCII */
  { NULL, 0, JIS0201ROMANtoUNI },		/* X0201ROMAN */
  { NULL, 0, JIS0201KANAtoUNI },		/* X0201KANA */
  { NULL, 0, ISO8859toUNI },			/* ISO8859_1 */
#ifdef ISO8859_TO_UNI
  { revISO8859_2, 95, NULL },			/* ISO8859_2 */
  { revISO8859_3, 88, NULL },			/* ISO8859_3 */
  { revISO8859_4, 95, NULL },			/* ISO8859_4 */
  { revISO8859_5, 95, NULL },			/* ISO8859_5 */
  { revISO8859_6, 50, NULL },			/* ISO8859_6 */
  { revISO8859_7, 89, NULL },			/* ISO8859_7 */
  { revISO8859_8, 57, NULL },			/* ISO8859_8 */
  { revISO8859_9, 95, NULL },			/* ISO8859_9 */
#else
  { NULL, 0, NULL },
  { NULL, 0, NULL },
  { NULL, 0, NULL },
  { NULL, 0, NULL },
  { NULL, 0, NULL },
  { NULL, 0, NULL },
  { NULL, 0, NULL },
  { NULL, 0, NULL },
#endif
#ifdef JIS_TO_UNI
  { revJIS0208, 6878, NULL },			/* C6226 */
#else
  { NULL, 0, NULL },
#endif
#ifdef GB_TO_UNI
  { revGB2312, 7444, NULL },			/* GB2312 */
#else
  { NULL, 0, NULL },
#endif
#ifdef JIS_TO_UNI
  { revJIS0208, 6878, NULL },			/* X0208 */
#else
  { NULL, 0, NULL },
#endif
#ifdef KSC_TO_UNI
  { revKSC5601, 8223, NULL },			/* KSC5601 */
#else
  { NULL, 0, NULL },
#endif
#ifdef JIS_TO_UNI
  { revJIS0212, 6066, NULL },			/* X0212 */
#else
  { NULL, 0, NULL },
#endif
  { NULL, 0, NULL },				/* ISO_IR_165 */
  { NULL, 0, CNStoUNI },			/* CNS_1 */
  { NULL, 0, CNStoUNI },			/* CNS_2 */
  { NULL, 0, CNStoUNI },			/* CNS_3 */
  { NULL, 0, CNStoUNI },			/* CNS_4 */
  { NULL, 0, NULL },				/* CNS_5 */
  { NULL, 0, NULL },				/* CNS_6 */
  { NULL, 0, NULL },				/* CNS_7 */
#ifdef BIG5_TO_UNI
  { revBIG5, 13702, NULL }			/* BIG5 */
#else
  { NULL, 0, NULL }
#endif
};

public ic_t RevUNI( ic_t ic, char *cset )
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
  }

  *cset = ASCII;
  return (ic_t)'?';
}
