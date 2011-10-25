/*
 * itable_t.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __ITABLE_T_H__
#define __ITABLE_T_H__

#include <ascii.h>
#include <attr.h>
#include <ichar.h>

/*
 * character sets's name space (char)
 */

#define ISO646_US	0	/* ISO 646 United states (ANSI X3.4-1968) */
#define X0201ROMAN	1	/* JIS X0201-1976 Japanese Roman */

#define X0201KANA	2	/* JIS X0201-1976 Japanese Katakana */

#define ISO8859_1	3	/* ISO 8859/1 Latin1 alphabet */
#define ISO8859_2	4	/* ISO 8859/2 Latin2 alphabet */
#define ISO8859_3	5	/* ISO 8859/3 Latin3 alphabet */
#define ISO8859_4	6	/* ISO 8859/4 Latin4 alphabet */
#define ISO8859_5	7	/* ISO 8859/5 Cyrillic alphabet */
#define ISO8859_6	8	/* ISO 8859/6 Arabic alphabet */
#define ISO8859_7	9	/* ISO 8859/7 Greek alphabet */
#define ISO8859_8	10	/* ISO 8859/8 Hebrew alphabet */
#define ISO8859_9	11	/* ISO 8859/9 Latin5 alphabet */

#define C6226		12	/* JIS C 6226-1978 Japanese kanji */
#define GB2312		13	/* GB 2312-80 Chinese kanji */
#define X0208		14	/* JIS X 0208-1983 Japanese kanji */
#define KSC5601		15	/* KS C 5601-1987 Korean graphic charset */
#define X0212		16	/* JIS X 0212-1990 Supplementary charset */
#define ISO_IR_165	17	/* ISO-IR-165 */
#define CNS_1		18	/* CNS 11643-1992 Plane 1 */
#define CNS_2		19	/* CNS 11643-1992 Plane 2 */
#define CNS_3		20	/* CNS 11643-1992 Plane 3 */
#define CNS_4		21	/* CNS 11643-1992 Plane 4 */
#define CNS_5		22	/* CNS 11643-1992 Plane 5 */
#define CNS_6		23	/* CNS 11643-1992 Plane 6 */
#define CNS_7		24	/* CNS 11643-1992 Plane 7 */

#define BIG5		25	/* Big5 Traditional Chinese */

#define UNICODE		26	/* Unicode */

#define PSEUDO		27

#define SPACE		28
#define HTAB		29
#define CNTRL		30

#define I_TABLE_SIZE	31

#define NOSET		I_TABLE_SIZE

#define ASCII		ISO646_US

/*
 * international charset table
 */

typedef struct {
  char       charset;
  char       fin;		/* final character */
  boolean_t  multi;		/* is multi bytes charset */
  boolean_t  set94;		/* is 94 chars charset */
  int        length;		/* code length for the charset */
  int        width;		/* graphical width of each char */
} i_table_t;

#define SET94		TRUE
#define SET96		FALSE

public boolean_t allow_unify;
public ic_t unicode_width_threshold;

public void ItableInit();
public char ItableLookup( char fin, boolean_t multi, boolean_t set94 );

public int IcharWidth( char charset, ic_t c );
public int IstrWidth( i_str_t *istr );

#endif /* __ITABLE_T_H__ */
