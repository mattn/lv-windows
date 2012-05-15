/*
 * re.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: re.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __RE_H__
#define __RE_H__

#include <itable.h>

/*
 * regular expression
 *
 * regexp = [ '^' ] <exp> [ '$' ]
 *   exp = <exp1> { '\|' <exp1> }
 *     exp1 = <exp2> { <exp2> }
 *     exp2 = <term> [ '*' | '?' | '+' ]
 *     term = <char> | '.' | '\1' | '\2'
 *          | '\(' <exp> '\)'
 *          | '[' [ '^' ] <charset> ']'
 *       char = <ichar> | '\' <ichar>
 *       charset = <charset1> { <charset1> }
 *         charset1 = <char> [ '-' <char> ]
 */

#define OP_LEAF		0
#define OP_SIMPLE_LEAF	1
#define OP_HAT		2
#define OP_DOLLAR	3
#define OP_RANGE	4
#define OP_COMPLEMENT	5
#define OP_COMPRANGE	6
#define OP_CATEGORY1	7
#define OP_CATEGORY2	8
#define OP_CATEGORY3	9
#define OP_PERIOD	10
#define OP_IGETA	11

#define OP_LEAF_MAX	12

#define OP_OR		12
#define OP_CLOSURE	13
#define OP_QUESTION	14
#define OP_CAT		15

/*
 * ic list
 */

typedef struct RE_T {
  int op;
#ifdef MSDOS
#undef i_str_t
#endif /* MSDOS */
  i_str_t *ic;
#ifdef MSDOS
#define i_str_t i_str_t far
#endif /* MSDOS */
  struct RE_T *left, *right;
  void *firstpos, *lastpos, *followpos;
} re_t;

typedef struct SET_T {
  re_t *re;
  struct SET_T *next;
} set_t;

public byte *reMessage;

public void SetFreeAll( set_t *set );
public void ReFreeAll( re_t *re );
public re_t *ReMakeTree( i_str_t *istr );

#endif /* __RE_H__ */
