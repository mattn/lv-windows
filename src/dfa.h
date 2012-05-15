/*
 * dfa.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: dfa.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __DFA_H__
#define __DFA_H__

#include <istr.h>

/*#define REGEXP_TEST*/

public boolean_t regexp_short_cut;
public ic_t regexp_first_letter;

public byte *ReMakeDFA( i_str_t *istr );
public boolean_t ReFreeDFA();

public boolean_t ReRun( i_str_t *istr, int *ptr );

public void ReShowNFA();
public void ReShowDFA();

#endif /* __DFA_H__ */
