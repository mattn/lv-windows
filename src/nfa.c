/*
 * nfa.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: nfa.c,v 1.3 2003/11/13 03:08:19 nrt Exp $
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
#include <re.h>
#include <uty.h>
#include <begin.h>
#include <nfa.h>

private set_t *SetAlloc( re_t *re )
{
  set_t *set;

  set = (set_t *)Malloc( sizeof( set_t ) );

  set->re = re;
  set->next = NULL;

  return set;
}

public void ReInclude( set_t **root, re_t *target )
{
  set_t *new, *ptr, *last, *res;

  if( NULL == target )
    return;

  res = NULL;
  for( last = NULL, ptr = *root ; ptr ; last = ptr, ptr = ptr->next ){
    if( ptr->re == target ){
      res = ptr;
      break;
    }
    if( ptr->re->op < target->op )
      break;
  }

  if( NULL == res ){
    /*
     * insert new into root in descendent order
     */
    new = SetAlloc( target );

    if( NULL == last ){
      new->next = *root;
      *root = new;
    } else {
      new->next = last->next;
      last->next = new;
    }
  }
}

public void SetInclude( set_t **root, set_t *target )
{
  set_t *set;

  for( set = target ; set ; set = set->next )
    ReInclude( root, set->re );
}

private int ReNullable( re_t *re )
{
  if( NULL == re )
    return FALSE;

  if( OP_OR == re->op )
    return ReNullable( re->left ) || ReNullable( re->right );
  else if( OP_CAT == re->op )
    return ReNullable( re->left ) && ReNullable( re->right );
  else if( OP_CLOSURE == re->op || OP_QUESTION == re->op )
    return TRUE;
  else
    return FALSE;
}

public set_t *ReFirstpos( re_t *re )
{
  set_t *set = NULL;

  if( NULL == re )
    return NULL;

  if( NULL != re->firstpos )
    return (set_t *)re->firstpos;

  if( OP_OR == re->op ){
    SetInclude( (set_t **)&set, ReFirstpos( re->left ) );
    SetInclude( (set_t **)&set, ReFirstpos( re->right ) );
  } else if( OP_CAT == re->op ){
    if( ReNullable( re->left ) ){
      SetInclude( (set_t **)&set, ReFirstpos( re->left ) );
      SetInclude( (set_t **)&set, ReFirstpos( re->right ) );
    } else {
      SetInclude( (set_t **)&set, ReFirstpos( re->left ) );
    }
  } else if( OP_CLOSURE == re->op || OP_QUESTION == re->op ){
    SetInclude( (set_t **)&set, ReFirstpos( re->left ) );
  } else
    set = SetAlloc( re );

  re->firstpos = (void *)set;

  return set;
}

public set_t *ReLastpos( re_t *re )
{
  set_t *set = NULL;

  if( NULL == re )
    return NULL;

  if( NULL != re->lastpos )
    return (set_t *)re->lastpos;

  if( OP_OR == re->op ){
    SetInclude( (set_t **)&set, ReLastpos( re->right ) );
    SetInclude( (set_t **)&set, ReLastpos( re->left ) );
  } else if( OP_CAT == re->op ){
    if( ReNullable( re->right ) ){
      SetInclude( (set_t **)&set, ReLastpos( re->right ) );
      SetInclude( (set_t **)&set, ReLastpos( re->left ) );
    } else {
      SetInclude( (set_t **)&set, ReLastpos( re->right ) );
    }
  } else if( OP_CLOSURE == re->op || OP_QUESTION == re->op ){
    SetInclude( (set_t **)&set, ReLastpos( re->left ) );
  } else
    set = SetAlloc( re );

  re->lastpos = (void *)set;

  return set;
}

public void ReFollowpos( re_t *re )
{
  set_t *set;

  if( NULL == re )
    return;

  if( NULL != re->followpos )
    return;

  if( OP_CAT == re->op ){
    for( set = ReLastpos( re->left ) ; set ; set = set->next )
      SetInclude( (set_t **)&set->re->followpos, ReFirstpos( re->right ) );
  } else if( OP_CLOSURE == re->op ){
    for( set = ReLastpos( re ) ; set ; set = set->next )
      SetInclude( (set_t **)&set->re->followpos, ReFirstpos( re ) );
  } else if( OP_QUESTION == re->op ){
    /* nothing follows */
  }
}

public void ReMakeFollowpos( re_t *re )
{
  if( NULL == re )
    return;

  ReFirstpos( re->left );
  ReFirstpos( re->right );
  ReFirstpos( re );

  ReLastpos( re->left );
  ReLastpos( re->right );
  ReLastpos( re );

  ReMakeFollowpos( re->left );
  ReMakeFollowpos( re->right );
  ReFollowpos( re );
}
