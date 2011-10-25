/*
 * re.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdio.h>
#include <stdlib.h>

#include <import.h>
#include <uty.h>
#include <find.h>
#include <begin.h>
#include <re.h>

/*#define SHOWFUNC*/

private int isIdx;
private i_str_t *iStr;

#ifdef MSDOS
#undef i_str_t
#endif /* MSDOS */

#define GetChar( ic )		( (ic) = iStr[ isIdx++ ] )
#define LookAhead( ic )		( (ic) = iStr[ isIdx ] )

private boolean_t aborted;
private boolean_t complement;

typedef struct IC_LIST_T {
  i_str_t *ic;
  struct IC_LIST_T *next;
} ic_list_t;

private ic_list_t *icRoot = NULL;

private i_str_t *IcAlloc( i_str_t *ic )
{
  i_str_t *res;
  ic_list_t *ptr, *last, *new;

  res = NULL;
  for( last = NULL, ptr = icRoot ; ptr ; last = ptr, ptr = ptr->next ){
    if( ptr->ic->charset > ic->charset )
      break;
    else if( ptr->ic->charset == ic->charset ){
      if( ptr->ic->c == ic->c ){
	res = ptr->ic;
	break;
      }
      if( ptr->ic->c > ic->c )
	break;
    }
  }
  if( NULL == res ){
    res = (i_str_t *)Malloc( sizeof( i_str_t ) );

    res->charset = ic->charset;
    res->c = ic->c;

    new = (ic_list_t *)Malloc( sizeof( ic_list_t ) );

    new->ic = res;

    if( NULL == last ){
      new->next = icRoot;
      icRoot = new;
    } else {
      new->next = last->next;
      last->next = new;
    }
  }

  return res;
}

private void IcListFreeAll()
{
  ic_list_t *list, *next;

  list = icRoot;

  while( list ){
    next = list->next;
    free( list->ic );
    free( list );
    list = next;
  }

  icRoot = NULL;
}

private re_t *ReAlloc( int op, i_str_t *ic, re_t *left, re_t *right )
{
  re_t *re;

  re = (re_t *)Malloc( sizeof( re_t ) );

  re->op = op;

  if( NULL != ic )
    re->ic = IcAlloc( ic );
  else
    re->ic = NULL;

  re->left  = left;
  re->right = right;

  re->firstpos = NULL;
  re->lastpos = NULL;
  re->followpos = NULL;

  return re;
}

private re_t *ReCopy( re_t *re )
{
  re_t *new, *left, *right;

  if( NULL == re )
    return NULL;

  left = ReCopy( re->left );
  right = ReCopy( re->right );

  new = ReAlloc( re->op, re->ic, left, right );

  return new;
}

private re_t *ReChar()
{
  i_str_t ic;

  GetChar( ic );

  if( ASCII == ic.charset && ic.c == '\\' )
    GetChar( ic );

  if( TRUE == casefold_search && ASCII == ic.charset ){
    if( 'Z' >= ic.c && 'A' <= ic.c )
      ic.c += 0x20;
  }

  if( PSEUDO > ic.charset ){
    return ReAlloc( OP_SIMPLE_LEAF, &ic, NULL, NULL );
  } else {
    ic.c &= 0x00ff;
    return ReAlloc( OP_LEAF, &ic, NULL, NULL );
  }
}

private re_t *ReCharset1()
{
  i_str_t ic;
  re_t *re, *re1, *re2;

  re = ReChar();

  if( TRUE == aborted )
    return re;

  LookAhead( ic );
  if( ASCII == ic.charset && '-' == ic.c ){
    isIdx++;

    LookAhead( ic );
    if( ASCII == ic.charset && ']' == ic.c ){
      isIdx--;

      if( TRUE == complement )
	re->op = OP_COMPLEMENT;

      return re;
    }

    re1 = re;
    re2 = ReChar();

    if( TRUE == complement )
      re = ReAlloc( OP_COMPRANGE, &ic, re1, re2 );
    else
      re = ReAlloc( OP_RANGE, &ic, re1, re2 );

    if( ( OP_LEAF != re1->op && OP_SIMPLE_LEAF != re1->op )
       || ( OP_LEAF != re2->op && OP_SIMPLE_LEAF != re2->op ) ){
      aborted = TRUE;
      reMessage = "miscomposed range";
      return re;
    }

    if( re->left->ic->charset != re->right->ic->charset ){
      aborted = TRUE;
      reMessage = "overcrossing range";
      return re;
    }
  } else if( TRUE == complement ){
    re->op = OP_COMPLEMENT;
  }

  return re;
}

private re_t *ReCharset()
{
  i_str_t ic;
  re_t *re, *re1, *re2;

  re1 = NULL;
  for( ; ; ){
    re2 = ReCharset1();

    if( NULL != re1 ){
      re = ReAlloc( OP_OR, NULL, re1, re2 );
      re1 = re;
    } else {
      re = re1 = re2;
    }

    LookAhead( ic );
    if( NOSET == ic.charset || (ASCII == ic.charset && ']' == ic.c ) )
      break;

    if( TRUE == aborted )
      return re;
  }

  return re;
}

private re_t *ReExp();	/* forward declaration */

private re_t *ReTerm()
{
  i_str_t ic;
  re_t *re;

  LookAhead( ic );

  if( NOSET == ic.charset ){
    re = NULL;
    reMessage = "unexpected eol";
    aborted = TRUE;

    return re;
  }

  if( ASCII == ic.charset ){
    if( '.' == ic.c ){
      isIdx++;
      return ReAlloc( OP_PERIOD, NULL, NULL, NULL );
    } else if( '\\' == ic.c ){
      isIdx++;
      LookAhead( ic );
      if( ASCII == ic.charset ){
	if( '1' == ic.c ){
	  isIdx++;
	  return ReAlloc( OP_CATEGORY1, NULL, NULL, NULL );
	} else if ( '2' == ic.c ){
	  isIdx++;
	  return ReAlloc( OP_CATEGORY2, NULL, NULL, NULL );
	} else if( '(' == ic.c ){
	  isIdx++;
	  /*
	   * recursive exp
	   */
	  re = ReExp();

	  if( TRUE == aborted )
	    return re;

	  GetChar( ic );
	  if( ASCII != ic.charset || ')' != ic.c ){	/* originally, '\)' */
	    reMessage = "unmatched (";
	    aborted = TRUE;
	  }

	  return re;
	} else
	  isIdx--;
      } else
	isIdx--;
    } else if( '[' == ic.c ){
      /*
       * range
       */
      isIdx++;
      LookAhead( ic );
      if( ASCII == ic.charset && '^' == ic.c ){
	complement = TRUE;
	isIdx++;
      }

      re = ReCharset();

      if( TRUE == complement )
	complement = FALSE;

      if( TRUE == aborted )
	return re;

      GetChar( ic );
      if( ASCII != ic.charset || ']' != ic.c ){
	reMessage = "unmatched [";
	aborted = TRUE;
      }

      return re;
    }
  }

  /*
   * leaf
   */
  return ReChar();
}

private re_t *ReExp2()
{
  i_str_t ic;
  re_t *re;

  re = ReTerm();

  if( TRUE == aborted )
    return re;

  LookAhead( ic );
  if( ASCII == ic.charset ){
    if( '*' == ic.c ){	
      isIdx++;
      re = ReAlloc( OP_CLOSURE, NULL, re, NULL );
    } else if( '?' == ic.c ){
      isIdx++;
      re = ReAlloc( OP_QUESTION, NULL, re, NULL );
    } else if( '+' == ic.c ){
      isIdx++;
      re = ReAlloc( OP_CAT, NULL,
		   re,
		   ReAlloc( OP_CLOSURE, NULL, ReCopy( re ), NULL ) );
    }
  }

  return re;
}

private re_t *ReExp1()
{
  i_str_t ic;
  re_t *re, *re1, *re2;

  re1 = NULL;
  for( ; ; ){
    re2 = ReExp2();

    if( NULL != re1 ){
      re1 = re = ReAlloc( OP_CAT, NULL, re1, re2 );
    } else {
      re1 = re = re2;
    }

    if( TRUE == aborted )
      return re;

    LookAhead( ic );

    if( NOSET == ic.charset )
      break;

    /*
     * check out terminating condition
     */

    if( ASCII == ic.charset ){
      if( '\\' == ic.c ){
	isIdx++;
	LookAhead( ic );
	if( ASCII == ic.charset && ( ')' == ic.c || '|' == ic.c ) )
	  /*
	   * note: '\\' is removed
	   */
	  break;
	isIdx--;
      } else if( '$' == ic.c ){
	isIdx++;
	LookAhead( ic );
	isIdx--;
	if( NOSET == ic.charset )
	  break;
      }
    }
  }

  return re;
}

private re_t *ReExp()
{
  i_str_t ic;
  re_t *re, *re1, *re2;

  re1 = NULL;
  for( ; ; ){
    re2 = ReExp1();

    if( NULL != re1 ){
      re = ReAlloc( OP_OR, NULL, re1, re2 );
      re1 = re;
    } else {
      re = re1 = re2;
    }

    if( TRUE == aborted )
      return re;

    LookAhead( ic );
    if( ASCII == ic.charset && '|' == ic.c )	/* originally, '\|' */
      isIdx++;
    else
      break;
  }

  return re;
}

private re_t *ReRegexp()
{
  i_str_t ic;
  re_t *re;

  LookAhead( ic );
  if( ASCII == ic.charset && '^' == ic.c ){
    isIdx++;
    re = ReAlloc( OP_CAT, NULL,
		 ReAlloc( OP_HAT, NULL, NULL, NULL ),
		 ReExp() );
  } else {
    re = ReExp();
  }

  if( TRUE == aborted )
    return re;

  LookAhead( ic );
  if( ASCII == ic.charset && '$' == ic.c ){
    isIdx++;
    LookAhead( ic );
    if( NOSET == ic.charset ){
      re = ReAlloc( OP_CAT, NULL,
		   re,
		   ReAlloc( OP_DOLLAR, NULL, NULL, NULL ) );
    }
  }

  return re;
}

public void SetFreeAll( set_t *set )
{
  set_t *next;

  while( set ){
    next = set->next;
    free( set );
    set = next;
  }
}

public void ReFreeAll( re_t *re )
{
  if( NULL == re )
    return;

  ReFreeAll( re->right );
  ReFreeAll( re->left );

  SetFreeAll( re->firstpos );
  SetFreeAll( re->lastpos );
  SetFreeAll( re->followpos );

  free( re );
}

#ifdef MSDOS
#define i_str_t i_str_t far
#endif /* MSDOS */

public re_t *ReMakeTree( i_str_t *istr )
{
  re_t *re;

  IcListFreeAll();

  reMessage = NULL;
  aborted = FALSE;
  complement = FALSE;

  isIdx = 0;
  iStr = istr;

  re = ReRegexp();

  if( TRUE == aborted ){
    ReFreeAll( re );
    return NULL;
  } else {
    re = ReAlloc( OP_CAT, NULL,
		 re,
		 ReAlloc( OP_IGETA, NULL, NULL, NULL ) );
    return re;
  }
}
