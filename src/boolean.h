/*
 * boolean.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __BOOLEAN_H__
#define __BOOLEAN_H__

#ifndef IRIX
#undef NULL
#endif

#ifndef NULL
#define NULL		0
#endif

#ifdef SYSV

#ifndef FALSE
#define FALSE		NULL
#endif

#ifndef TRUE
#define TRUE		!NULL
#endif
#ifndef boolean_t
#define boolean_t	int
#endif

#else /* SYSV */

typedef enum { FALSE, TRUE } boolean_t;

#define Not( flag )	( (flag) ? FALSE : TRUE )

#endif /* SYSV */

#endif /* __BOOLEAN_H__ */
