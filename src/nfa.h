/*
 * nfa.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
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
