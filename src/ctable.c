/*
 * ctable.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: ctable.c,v 1.5 2004/01/05 07:23:29 nrt Exp $
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
#include <itable.h>
#include <begin.h>
#include <ctable_t.h>

/*
 * coding system table
 */

public c_table_t cTable[ C_TABLE_SIZE ] = {
  /*
   * Auto select: decoding on iso-2022-kr and encoding on iso-2022-jp
   */
  { AUTOSELECT, FALSE, "AUTO",
      {{0, 1}, {ASCII, KSC5601, ASCII, ASCII}, 0, 0 } },

  /*
   * 7bit
   */
  { UTF_7, FALSE, "UTF-7",
      {{0, 1}, {ASCII, ASCII, ASCII, ASCII}, 0, 0 } },
  { HZ_GB, FALSE, "HZ",
      {{0, 1}, {ASCII, GB2312, ASCII, ASCII}, 0, 0 } },

  /*
   * EUC encoding
   */
  { EUC_KOREA, FALSE, "EUC-KR",
      {{0, 1}, {ASCII, KSC5601, ASCII, ASCII}, 0, 0 } },
  { EUC_JAPAN, FALSE, "EUC-JP",
      {{0, 1}, {ASCII, X0208, X0201KANA, X0212}, 0, 0 } },
  { EUC_TAIWAN, FALSE, "EUC-TW",
      {{0, 1}, {ASCII, CNS_1, CNS_1, ASCII}, 0, 0 } },
  { EUC_CHINA, FALSE, "EUC-CN",
      {{0, 1}, {ASCII, GB2312, ASCII, ASCII}, 0, 0 } },

  /*
   * non iso-2022 encoding
   */
  { BIG_FIVE, FALSE, "BIG5",
      {{0, 1}, {ASCII, ASCII, ASCII, ASCII}, 0, 0 } },
  { SHIFT_JIS, FALSE, "SHIFT-JIS",
      {{0, 1}, {ASCII, X0201KANA, ASCII, ASCII}, 0, 0 } },
  { UTF_8, FALSE, "UTF-8",
      {{0, 1}, {ASCII, ASCII, ASCII, ASCII}, 0, 0 } },

  /*
   * ISO 2022 8bit encoding
   */
  { ISO_8859_1, TRUE, "ISO-8859-1",
      {{0, 1}, {ASCII, ISO8859_1, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_2, TRUE, "ISO-8859-2",
      {{0, 1}, {ASCII, ISO8859_2, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_3,	TRUE, "ISO-8859-3",
      {{0, 1}, {ASCII, ISO8859_3, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_4, TRUE, "ISO-8859-4",
      {{0, 1}, {ASCII, ISO8859_4, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_5, TRUE, "ISO-8859-5",
      {{0, 1}, {ASCII, ISO8859_5, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_6, TRUE, "ISO-8859-6",
      {{0, 1}, {ASCII, ISO8859_6, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_7, TRUE, "ISO-8859-7",
      {{0, 1}, {ASCII, ISO8859_7, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_8, TRUE, "ISO-8859-8",
      {{0, 1}, {ASCII, ISO8859_8, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_9, TRUE, "ISO-8859-9",
      {{0, 1}, {ASCII, ISO8859_9, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_10, TRUE, "ISO-8859-10",
      {{0, 1}, {ASCII, ISO8859_10, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_11, TRUE, "ISO-8859-11",
      {{0, 1}, {ASCII, ISO8859_11, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_13, TRUE, "ISO-8859-13",
      {{0, 1}, {ASCII, ISO8859_13, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_14, TRUE, "ISO-8859-14",
      {{0, 1}, {ASCII, ISO8859_14, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_15, TRUE, "ISO-8859-15",
      {{0, 1}, {ASCII, ISO8859_15, ASCII, ASCII}, 0, 0 } },
  { ISO_8859_16, TRUE, "ISO-8859-16",
      {{0, 1}, {ASCII, ISO8859_16, ASCII, ASCII}, 0, 0 } },

  /*
   * ISO 2022 7bit encoding
   */
  { ISO_2022_CN, FALSE, "ISO-2022-CN",
      {{0, 1}, {ASCII, GB2312, ASCII, ASCII}, 0, 0 } },
  { ISO_2022_JP, FALSE, "ISO-2022-JP",
      {{0, 3}, {ASCII, X0201KANA, X0201KANA, X0208}, 0, 0 } },
  { ISO_2022_KR, FALSE, "ISO-2022-KR",
      {{0, 1}, {ASCII, KSC5601, ASCII, ASCII}, 0, 0 } },

  /*
   * Raw encoding
   */
  { RAW, FALSE, "RAW",
      {{0, 1}, {ASCII, ASCII, ASCII, ASCII}, 0, 0 } }
};

public void CtableInit()
{
  int i;

  for( i = 0 ; i < C_TABLE_SIZE ; i++ )
    if( cTable[ i ].codingSystem != i )
      fprintf( stderr, "lv: invalid charset table\n" ), exit( -1 );
}
