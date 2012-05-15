/*
 * ascii.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: ascii.h,v 1.3 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __ASCII_H__
#define __ASCII_H__

#define NUL			0x00		/* C-@ */
#define SOH			0x01		/* C-a */
#define STX			0x02		/* C-b */
#define ETX			0x03		/* C-c */
#define EOT			0x04		/* C-d */
#define ENQ			0x05		/* C-e */
#define ACK			0x06		/* C-f */
#define BEL			0x07		/* C-g */
#define BS			0x08		/* C-h */
#define HT			0x09		/* C-i */
#define LF			0x0a		/* C-j */
#define VT			0x0b		/* C-k */
#define FF			0x0c		/* C-l */
#define CR			0x0d		/* C-m */
#define SO			0x0e		/* C-n */
#define SI			0x0f		/* C-o */
#define DLE			0x10		/* C-p */
#define DC1			0x11		/* C-q */
#define DC2			0x12		/* C-r */
#define DC3			0x13		/* C-s */
#define DC4			0x14		/* C-t */
#define NAK			0x15		/* C-u */
#define SYN			0x16		/* C-v */
#define ETB			0x17		/* C-w */
#define CAN			0x18		/* C-x */
#define EM			0x19		/* C-y */
#define SUB			0x1a		/* C-z */
#define ESC			0x1b		/* C-[ */
#define FS			0x1c		/* C-\ */
#define GS			0x1d		/* C-] */
#define RS			0x1e		/* C-^ */
#define US			0x1f		/* C-_ */

#define SP			0x20
#define DEL			0x7f

#define LS0			0x0f
#define LS1			0x0e

#define SS2			0x8e
#define SS3			0x8f

#endif /* __ASCII_H__ */
