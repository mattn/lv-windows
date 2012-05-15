/*
 * itable.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: itable.c,v 1.6 2003/11/13 03:08:19 nrt Exp $
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
#include <stdlib.h>

#include <import.h>
#include <begin.h>
#include <itable_t.h>

/*
 * international character set table
 */

public i_table_t iTable[ I_TABLE_SIZE ] = {
  { ISO646_US,	'B', FALSE, SET94, 1, 1 },
  { X0201ROMAN,	'J', FALSE, SET94, 1, 1 },

  { X0201KANA,	'I', FALSE, SET94, 1, 1 },

  { ISO8859_1,	'A', FALSE, SET96, 1, 1 },
  { ISO8859_2,	'B', FALSE, SET96, 1, 1 },
  { ISO8859_3,	'C', FALSE, SET96, 1, 1 },
  { ISO8859_4,	'D', FALSE, SET96, 1, 1 },
  { ISO8859_5,	'L', FALSE, SET96, 1, 1 },
  { ISO8859_6,	'G', FALSE, SET96, 1, 1 },
  { ISO8859_7,	'F', FALSE, SET96, 1, 1 },
  { ISO8859_8,	'H', FALSE, SET96, 1, 1 },
  { ISO8859_9,	'M', FALSE, SET96, 1, 1 },
  { ISO8859_10,	'V', FALSE, SET96, 1, 1 },
  { ISO8859_11,	'T', FALSE, SET96, 1, 1 },
  { ISO8859_13,	'Y', FALSE, SET96, 1, 1 },
  { ISO8859_14,	'_', FALSE, SET96, 1, 1 },
  { ISO8859_15,	'b', FALSE, SET96, 1, 1 },
  { ISO8859_16,	'f', FALSE, SET96, 1, 1 },

  { C6226,	'@', TRUE,  SET94, 2, 2 },
  { GB2312,	'A', TRUE,  SET94, 2, 2 },
  { X0208,	'B', TRUE,  SET94, 2, 2 },
  { KSC5601,	'C', TRUE,  SET94, 2, 2 },
  { X0212,	'D', TRUE,  SET94, 2, 2 },
  { ISO_IR_165,	'E', TRUE,  SET94, 2, 2 },
  { CNS_1,	'G', TRUE,  SET94, 2, 2 },
  { CNS_2,	'H', TRUE,  SET94, 2, 2 },
  { CNS_3,	'I', TRUE,  SET94, 2, 2 },
  { CNS_4,	'J', TRUE,  SET94, 2, 2 },
  { CNS_5,	'K', TRUE,  SET94, 2, 2 },
  { CNS_6,	'L', TRUE,  SET94, 2, 2 },
  { CNS_7,	'M', TRUE,  SET94, 2, 2 },

  { X0213_1,	'O', TRUE,  SET94, 2, 2 },
  { X0213_2,	'P', TRUE,  SET94, 2, 2 },

  { BIG5,	'0', TRUE,  SET94, 2, 2 },	/* non-registered final char */

  { UNICODE,	'2', TRUE,  SET94, 2, 2 },

  { PSEUDO,	  0, FALSE, SET94, 0, 0 },	/* pseudo-charset */

  { SPACE,	'B', FALSE, SET94, 1, 1 },
  { HTAB,	'B', FALSE, SET94, 1, 0 },
  { CNTRL,	'B', FALSE, SET94, 1, 0 },
  { LINE_FEED,	'B', FALSE, SET94, 1, 0 }
};

#define I_TABLE_CACHE_SIZE	4

private int iTableCacheIndex = 0;
private boolean_t iTableCacheUsed[ I_TABLE_CACHE_SIZE ];
private i_table_t iTableCache[ I_TABLE_CACHE_SIZE ];

public void ItableInit()
{
  int i;

  for( i = 0 ; i < I_TABLE_SIZE ; i++ )
    if( iTable[ i ].charset != i )
      fprintf( stderr, "lv: invalid ichar table\n" ), exit( -1 );

  for( i = 0 ; i < I_TABLE_CACHE_SIZE ; i++ )
    iTableCacheUsed[ i ] = FALSE;
}

public byte ItableLookup( byte fin, boolean_t multi, boolean_t set94 )
{
  int i;

  for( i = iTableCacheIndex ; i >= 0 ; i-- ){
    if( TRUE == iTableCacheUsed[ i ]
       && multi == iTableCache[ i ].multi
       && set94 == iTableCache[ i ].set94
       && fin == iTableCache[ i ].fin )
	return iTableCache[ i ].charset;
  }
  for( i = I_TABLE_CACHE_SIZE - 1 ; i > iTableCacheIndex ; i-- ){
    if( TRUE == iTableCacheUsed[ i ]
       && multi == iTableCache[ i ].multi
       && set94 == iTableCache[ i ].set94
       && fin == iTableCache[ i ].fin )
	return iTableCache[ i ].charset;
  }
  for( i = 0 ; i < PSEUDO ; i++ ){
    if( multi == iTable[ i ].multi
       && set94 == iTable[ i ].set94
       && fin == iTable[ i ].fin ){
      iTableCacheIndex++;
      if( iTableCacheIndex >= I_TABLE_CACHE_SIZE )
	iTableCacheIndex = 0;
      iTableCache[ iTableCacheIndex ] = iTable[ i ];
      iTableCacheUsed[ iTableCacheIndex ] = TRUE;

      return i;
    }
  }
  if( TRUE == allow_unify ){
    if( FALSE == multi && TRUE == set94 )
      return ASCII;
  }
  return NOSET;
}

public int IcharWidth( byte charset, ic_t c )
{
  if( charset < PSEUDO ){
    switch( charset ){
    case UNICODE:
      if( c < unicode_width_threshold )
	return 1;
      else
	return 2;
    }
  } else {
    switch( charset ){
    case HTAB:
    case CNTRL:
      return MakeByte1( c );
    }
  }
  return iTable[ (int)charset ].width;
}

public int IstrWidth( i_str_t *istr )
{
  int i, w;

  w = 0;
  for( i = 0 ; NOSET != istr[ i ].charset ; i++ )
    w += IcharWidth( istr[ i ].charset, istr[ i ].c );

  return w;
}
