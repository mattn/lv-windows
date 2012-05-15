/*
 * nfa.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: nfa.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __NFA_H__
#define __NFA_H__

#include <re.h>

public void ReInclude( set_t **root, re_t *target );
public void SetInclude( set_t **root, set_t *target );
public void ReMakeFollowpos( re_t *re );

public set_t *ReFirstpos( re_t *re );
public set_t *ReLastpos( re_t *re );
public void ReFollowpos( re_t *re );

#endif /* __NFA_H__ */
