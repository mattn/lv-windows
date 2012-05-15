/*
 * position.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: position.h,v 1.5 2004/01/05 07:30:15 nrt Exp $
 */

#ifndef __POSITION_H__
#define __POSITION_H__

#include <file.h>
#include <uty.h>

#define PositionGet( pos, s, b, o, p )					\
{									\
  (s) = (pos).seg;							\
  (b) = (pos).blk;							\
  (o) = (pos).off;							\
  (p) = (pos).phy;							\
}

#define PositionSet( pos, s, b, o, p )					\
{									\
  (pos).seg = (s);							\
  (pos).blk = (b);							\
  (pos).off = (o);							\
  (pos).phy = (p);							\
}

#define PositionAssign( toPos, fromPos )				\
{									\
  (toPos).seg = (fromPos).seg;						\
  (toPos).blk = (fromPos).blk;						\
  (toPos).off = (fromPos).off;						\
  (toPos).phy = (fromPos).phy;						\
}

#define PositionDec( f, seg, blk, off, phy )				\
{									\
  if( --(phy) < 0 ){							\
    if( --(off) < 0 ){							\
      if( (seg) == 0 ){		/* top of file */			\
	(phy)++;							\
	(off)++;							\
	/*								\
	 * BREAK for EXTERNAL FOR-LOOP					\
	 */								\
	break;								\
      } else {								\
	if( FALSE == FetchLine( (f), (seg) - 1, LV_PAGE_SIZE - 1 ) ){	\
	  /*								\
	   * memory shortage?						\
	   */								\
	  fprintf( stderr, "memory shortage?\n" );			\
	  FatalErrorOccurred();						\
        } else {		/* successfully loaded */		\
	  (seg)--;							\
	  (blk) = Block( (seg) );					\
	  (off) = (f)->page[ (blk) ].lines - 1;				\
	}								\
      }									\
    }									\
    (phy) = (f)->page[ (blk) ].line[ (off) ].heads - 1;			\
  }									\
}

#define PositionInc( f, seg, blk, off, phy )				\
{									\
  if( ++(phy) >= (f)->page[ (blk) ].line[ (off) ].heads ){		\
    if( ++(off) >= (f)->page[ (blk) ].lines ){				\
      if( FALSE == FetchLine( (f), (seg) + 1, 0 ) ){			\
				/* end of file */			\
	(phy)--;							\
	(off)--;							\
	/*								\
	 * BREAK for EXTERNAL FOR-LOOP					\
	 */								\
	break;								\
      } else {			/* successfully loaded */		\
	(seg)++;							\
	(blk) = Block( (seg) );						\
	(off) = 0;							\
      }									\
    }									\
    (phy) = 0;								\
  }									\
}

#endif /* __POSITION_H__ */
