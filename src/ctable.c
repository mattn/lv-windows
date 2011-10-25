/*
 * ctable.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
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
  { AUTOSELECT, FALSE, "a",
      {{0, 1}, {ASCII, KSC5601, ASCII, ASCII}, NULL, NULL } },

  /*
   * ISO 2022 7bit encoding
   */
  { ISO_2022_CN, FALSE, "c",
      {{0, 1}, {ASCII, ASCII, ASCII, ASCII}, NULL, NULL } },
  { ISO_2022_JP, FALSE, "j",
      {{0, 1}, {ASCII, X0201KANA, ASCII, ASCII}, NULL, NULL } },
  { ISO_2022_KR, FALSE, "k",
      {{0, 1}, {ASCII, KSC5601, ASCII, ASCII}, NULL, NULL } },

  /*
   * EUC encoding
   */
  { EUC_CHINA, FALSE, "ec",
      {{0, 1}, {ASCII, GB2312, ASCII, ASCII}, NULL, NULL } },
  { EUC_JAPAN, FALSE, "ej",
      {{0, 1}, {ASCII, X0208, X0201KANA, X0212}, NULL, NULL } },
  { EUC_KOREA, FALSE, "ek",
      {{0, 1}, {ASCII, KSC5601, ASCII, ASCII}, NULL, NULL } },
  { EUC_TAIWAN, FALSE, "et",
      {{0, 1}, {ASCII, CNS_1, CNS_1, ASCII}, NULL, NULL } },

  /*
   * non-iso2022 encoding
   */
  { SHIFT_JIS, FALSE, "s",
      {{0, 1}, {ASCII, X0201KANA, ASCII, ASCII}, NULL, NULL } },
  { BIG_FIVE, FALSE, "b",
      {{0, 1}, {ASCII, ASCII, ASCII, ASCII}, NULL, NULL } },

  /*
   * ISO 2022 8bit encoding
   */
  { ISO_8859_1, TRUE, "l",
      {{0, 1}, {ASCII, ISO8859_1, ASCII, ASCII}, NULL, NULL } },
  { ISO_8859_2, TRUE, "l2",
      {{0, 1}, {ASCII, ISO8859_2, ASCII, ASCII}, NULL, NULL } },
  { ISO_8859_3,	TRUE, "l3",
      {{0, 1}, {ASCII, ISO8859_3, ASCII, ASCII}, NULL, NULL } },
  { ISO_8859_4, TRUE, "l4",
      {{0, 1}, {ASCII, ISO8859_4, ASCII, ASCII}, NULL, NULL } },
  { ISO_8859_5, TRUE, "cy",
      {{0, 1}, {ASCII, ISO8859_5, ASCII, ASCII}, NULL, NULL } },
  { ISO_8859_6, TRUE, "ar",
      {{0, 1}, {ASCII, ISO8859_6, ASCII, ASCII}, NULL, NULL } },
  { ISO_8859_7, TRUE, "gr",
      {{0, 1}, {ASCII, ISO8859_7, ASCII, ASCII}, NULL, NULL } },
  { ISO_8859_8, TRUE, "he",
      {{0, 1}, {ASCII, ISO8859_8, ASCII, ASCII}, NULL, NULL } },
  { ISO_8859_9, TRUE, "l5",
      {{0, 1}, {ASCII, ISO8859_9, ASCII, ASCII}, NULL, NULL } },

  /*
   * UTF
   */
  { UTF_7, FALSE, "u7",
      {{0, 1}, {ASCII, ASCII, ASCII, ASCII}, NULL, NULL } },
  { UTF_8, FALSE, "u8",
      {{0, 1}, {ASCII, ASCII, ASCII, ASCII}, NULL, NULL } },

  /*
   * Raw encoding
   */
  { RAW, FALSE, "r",
      {{0, 1}, {ASCII, ASCII, ASCII, ASCII}, NULL, NULL } }
};

public void CtableInit()
{
  int i;

  for( i = 0 ; i < C_TABLE_SIZE ; i++ )
    if( cTable[ i ].codingSystem != i )
      fprintf( stderr, "lv: invalid charset table\n" ), exit( -1 );
}
