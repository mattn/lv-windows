/*
 * itable_t.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: itable_t.h,v 1.6 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __ITABLE_T_H__
#define __ITABLE_T_H__

#include <ascii.h>
#include <attr.h>
#include <ichar.h>

/*
 * character sets's name space (byte)
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
#define ISO8859_10	12	/* ISO 8859/10 Latin6 alphabet */
#define ISO8859_11	13	/* ISO 8859/13 Thai alphabet */
#define ISO8859_13	14	/* ISO 8859/13 Latin7 alphabet */
#define ISO8859_14	15	/* ISO 8859/14 Latin8 alphabet */
#define ISO8859_15	16	/* ISO 8859/15 Latin9 alphabet */
#define ISO8859_16	17	/* ISO 8859/16 Latin10 alphabet */

#define C6226		18	/* JIS C 6226-1978 Japanese kanji */
#define GB2312		19	/* GB 2312-80 Chinese kanji */
#define X0208		20	/* JIS X 0208-1983 Japanese kanji */
#define KSC5601		21	/* KS C 5601-1987 Korean graphic charset */
#define X0212		22	/* JIS X 0212-1990 Supplementary charset */
#define ISO_IR_165	23	/* ISO-IR-165 */
#define CNS_1		24	/* CNS 11643-1992 Plane 1 */
#define CNS_2		25	/* CNS 11643-1992 Plane 2 */
#define CNS_3		26	/* CNS 11643-1992 Plane 3 */
#define CNS_4		27	/* CNS 11643-1992 Plane 4 */
#define CNS_5		28	/* CNS 11643-1992 Plane 5 */
#define CNS_6		29	/* CNS 11643-1992 Plane 6 */
#define CNS_7		30	/* CNS 11643-1992 Plane 7 */
			
#define X0213_1		31	/* JIS X 0213-2000 Plane 1 */
#define X0213_2		32	/* JIS X 0213-2000 Plane 2 */
			
#define BIG5		33	/* Big5 Traditional Chinese */
			
#define UNICODE		34	/* Unicode */
			
#define PSEUDO		35
			
#define SPACE		36
#define HTAB		37
#define CNTRL		38
			
#define LINE_FEED	39
			
#define I_TABLE_SIZE	40

#define NOSET		I_TABLE_SIZE

#define ASCII		ISO646_US

/*
 * international charset table
 */

typedef struct {
  byte       charset;
  byte       fin;		/* final character */
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
public byte ItableLookup( byte fin, boolean_t multi, boolean_t set94 );

public int IcharWidth( byte charset, ic_t c );
public int IstrWidth( i_str_t *istr );

#endif /* __ITABLE_T_H__ */
