/*
 * ctable_t.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __CTABLE_T_H__
#define __CTABLE_T_H__

/* coding systems (char) */

#define AUTOSELECT		0	/* pseudo coding system */

#define ISO_2022_CN		1	/* iso-2022-cn */
#define ISO_2022_JP		2	/* iso-2022-jp */
#define ISO_2022_KR		3	/* iso-2022-kr */

#define EUC_CHINA		4	/* Extended unix code */
#define EUC_JAPAN		5	/* Extended unix code */
#define EUC_KOREA		6	/* Extended unix code */
#define EUC_TAIWAN		7	/* Extended unix code */

#define SHIFT_JIS		8	/* shift-jis encoding */
#define BIG_FIVE		9	/* big5 encoding */

#define ISO_8859_1		10	/* iso-2022-8bit-ss2 */
#define ISO_8859_2		11	/* iso-2022-8bit-ss2 */
#define ISO_8859_3		12	/* iso-2022-8bit-ss2 */
#define ISO_8859_4		13	/* iso-2022-8bit-ss2 */
#define ISO_8859_5		14	/* iso-2022-8bit-ss2 */
#define ISO_8859_6		15	/* iso-2022-8bit-ss2 */
#define ISO_8859_7		16	/* iso-2022-8bit-ss2 */
#define ISO_8859_8		17	/* iso-2022-8bit-ss2 */
#define ISO_8859_9		18	/* iso-2022-8bit-ss2 */

#define UTF_7			19	/* UTF-7 */
#define UTF_8			20	/* UTF-8 */

#define RAW			21	/* raw mode */

#define C_TABLE_SIZE		22	/* pseudo coding system */

#define GL			0
#define GR			1

#define G0			0
#define G1			1
#define G2			2
#define G3			3

typedef struct {
  char      gset[ 2 ];
  char      cset[ 4 ];
  char      sset;
  char      attr;
} state_t;

typedef struct {
  char      codingSystem;
  boolean_t bit8;
  char      *codingSystemName;
  state_t   state;
} c_table_t;

public void CtableInit();

#endif /* __CTABLE_T_H__ */
