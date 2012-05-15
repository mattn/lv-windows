/*
 * ctable_t.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: ctable_t.h,v 1.4 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __CTABLE_T_H__
#define __CTABLE_T_H__

/* coding systems (byte) */

#define AUTOSELECT		0	/* pseudo coding system */

#define UTF_7			1	/* UTF-7 */
#define HZ_GB			2	/* HZ enabled euc-china */

#define EUC_KOREA		3	/* Extended unix code */
#define EUC_JAPAN		4	/* Extended unix code */
#define EUC_TAIWAN		5	/* Extended unix code */
#define EUC_CHINA		6	/* Extended unix code */

#define BIG_FIVE		7	/* big5 encoding */
#define SHIFT_JIS		8	/* shift-jis encoding */

#define UTF_8			9	/* UTF-8 */

#define ISO_8859_1		10	/* iso-2022-8bit-ss2 */
#define ISO_8859_2		11	/* iso-2022-8bit-ss2 */
#define ISO_8859_3		12	/* iso-2022-8bit-ss2 */
#define ISO_8859_4		13	/* iso-2022-8bit-ss2 */
#define ISO_8859_5		14	/* iso-2022-8bit-ss2 */
#define ISO_8859_6		15	/* iso-2022-8bit-ss2 */
#define ISO_8859_7		16	/* iso-2022-8bit-ss2 */
#define ISO_8859_8		17	/* iso-2022-8bit-ss2 */
#define ISO_8859_9		18	/* iso-2022-8bit-ss2 */
#define ISO_8859_10		19	/* iso-2022-8bit-ss2 */
#define ISO_8859_11		20	/* iso-2022-8bit-ss2 */
#define ISO_8859_13		21	/* iso-2022-8bit-ss2 */
#define ISO_8859_14		22	/* iso-2022-8bit-ss2 */
#define ISO_8859_15		23	/* iso-2022-8bit-ss2 */
#define ISO_8859_16		24	/* iso-2022-8bit-ss2 */

#define ISO_2022_CN		25	/* iso-2022-cn */
#define ISO_2022_JP		26	/* iso-2022-jp */
#define ISO_2022_KR		27	/* iso-2022-kr */

#define RAW			28	/* raw mode */

#define C_TABLE_SIZE		29	/* pseudo coding system */

#define GL			0
#define GR			1

#define G0			0
#define G1			1
#define G2			2
#define G3			3

typedef struct {
  byte      gset[ 2 ];
  byte      cset[ 4 ];
  byte      sset;
  byte      attr;
} state_t;

typedef struct {
  byte      codingSystem;
  boolean_t bit8;
  byte      *codingSystemName;
  state_t   state;
} c_table_t;

public void CtableInit();

#endif /* __CTABLE_T_H__ */
