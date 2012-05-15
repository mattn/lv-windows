/*
 * istr.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: istr.h,v 1.4 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __ISTR_H__
#define __ISTR_H__

#include <ichar.h>

#define ZONE_PAGE0		0
#define ZONE_PAGE1		1

#ifndef MSDOS /* if NOT defined */
#define ZONE_PAGE2		2
#define ZONE_PAGE3		3
#define ZONE_FREE		4
#else
#define ZONE_FREE		2
#endif /* MSDOS */

#define ZONE_SIZE		( ZONE_FREE + 1 )

public int IstrWidth( i_str_t *istr );

public void IstrInit();

public i_str_t *IstrAlloc( int zone, int istrLength );
public void IstrFree( i_str_t *istr );
public void IstrFreeZone( int zone );
public void IstrFreeAll();

#endif /* __ISTR_H__ */
