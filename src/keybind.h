/*
 * keybind.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: keybind.h,v 1.6 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __KEYBIND_H__
#define __KEYBIND_H__

typedef void (*key_table_t)( unsigned int );

private key_table_t keyTable[ 128 ] = {
  CommandNull,		/* C-@ */	/* 0/0 */
  CommandNull,		/* C-a */
  CommandPrevPage,	/* C-b */
  CommandNull,		/* C-c */
  CommandNextHalf,	/* C-d */
  CommandNextLine,	/* C-e */
  CommandNextPage,	/* C-f */
  CommandFileStatus,	/* C-g */
  CommandPrevLine,	/* C-h */
  CommandNull,		/* C-i */
  CommandNextLine,	/* C-j */
  CommandPrevLine,	/* C-k */
  CommandRedisplay,	/* C-l */
  CommandNextLine,	/* C-m */
  CommandNextLine,	/* C-n */
  CommandNull,		/* C-o */
  CommandPrevLine,	/* C-p */	/* 1/0 */
  CommandNull,		/* C-q */
  CommandRefresh,	/* C-r */
  CommandNull,		/* C-s */
  CommandToggleHz,	/* C-t */
  CommandPrevHalf,	/* C-u */
  CommandNextPage,	/* C-v */
  CommandNull,		/* C-w */
  CommandNull,		/* C-x */
  CommandPrevLine,	/* C-y */
  CommandShellEscape,	/* C-z */
  CommandCursor,	/* C-[ */
  CommandNull,		/* C-\ */
  CommandNull,		/* C-] */
  CommandNull,		/* C-^ */
  CommandNull,		/* C-_ */
  CommandNextPage,	/* ' ' */	/* 2/0 */
  CommandNull,		/* '!' */
  CommandNull,		/* '"' */
  CommandNull,		/* '#' */
  CommandNull,		/* '$' */
  CommandNull,		/* '%' */
  CommandNull,		/* '&' */
  CommandNull,		/* ''' */
  CommandNull,		/* '(' */
  CommandNull,		/* ')' */
  CommandNull,		/* '*' */
  CommandNull,		/* '+' */
  CommandNull,		/* ',' */
  CommandNull,		/* '-' */
  CommandNull,		/* '.' */
  CommandFindForward,	/* '/' */
  CommandNull,		/* '0' */	/* 3/0 */
  CommandNull,		/* '1' */
  CommandNull,		/* '2' */
  CommandNull,		/* '3' */
  CommandNull,		/* '4' */
  CommandNull,		/* '5' */
  CommandNull,		/* '6' */
  CommandNull,		/* '7' */
  CommandNull,		/* '8' */
  CommandNull,		/* '9' */
  CommandColon,		/* ':' */
  CommandNull,		/* ';' */
  CommandTopOfFile,	/* '<' */
  CommandFileStatus,	/* '=' */
  CommandBottomOfFile,	/* '>' */
  CommandFindBackward,	/* '?' */
  CommandNull,		/* '@' */	/* 4/0 */
  CommandNull,		/* 'A' */
  CommandNull,		/* 'B' */
  CommandNull,		/* 'C' */
  CommandNull,		/* 'D' */
  CommandNull,		/* 'E' */
  CommandPoll,		/* 'F' */
  CommandBottomOfFile,	/* 'G' */
  CommandPrevLine,	/* 'H' */
  CommandNull,		/* 'I' */
  CommandNull,		/* 'J' */
  CommandPrevPage,	/* 'K' */
  CommandNull,		/* 'L' */
  CommandNextPage,	/* 'M' */
  CommandRepeatBackward,/* 'N' */
  CommandNull,		/* 'O' */
  CommandNextLine,	/* 'P' */	/* 5/0 */
  CommandQuit,		/* 'Q' */
  CommandReload,	/* 'R' */
  CommandNull,		/* 'S' */
  CommandReverseCset,	/* 'T' */
  CommandNull,		/* 'U' */
  CommandVersion,	/* 'V' */
  CommandNull,		/* 'W' */
  CommandNull,		/* 'X' */
  CommandNull,		/* 'Y' */
  CommandNull,		/* 'Z' */
  CommandNull,		/* '[' */
  CommandNull,		/* '\' */
  CommandNull,		/* ']' */
  CommandNull,		/* '^' */
  CommandNull,		/* '_' */
  CommandNull,		/* '`' */	/* 6/0 */
  CommandNull,		/* 'a' */
  CommandPrevPage,	/* 'b' */
  CommandNull,		/* 'c' */
  CommandNextHalf,	/* 'd' */
  CommandNextLine,	/* 'e' */
  CommandNextPage,	/* 'f' */
  CommandTopOfFile,	/* 'g' */
  CommandNull,		/* 'h' */
  CommandNull,		/* 'i' */
  CommandNextLine,	/* 'j' */
  CommandPrevLine,	/* 'k' */
  CommandNull,		/* 'l' */
  CommandNull,		/* 'm' */
  CommandRepeatForward,	/* 'n' */
  CommandNull,		/* 'o' */
  CommandPercent,	/* 'p' */	/* 7/0 */
  CommandQuit,		/* 'q' */
  CommandRefresh,	/* 'r' */
#ifdef REGEXP_TEST
  CommandRegexpNFA,	/* 's' */
  CommandRegexpDFA,	/* 't' */
#else
  CommandNull,		/* 's' */
  CommandToggleCset,	/* 't' */
#endif /* REGEXP_TEST */
  CommandPrevHalf,	/* 'u' */
  CommandEdit,		/* 'v' */
  CommandPrevLine,	/* 'w' */
  CommandNull,		/* 'x' */
  CommandPrevLine,	/* 'y' */
  CommandNull,		/* 'z' */
  CommandNull,		/* '{' */
  CommandNull,		/* '|' */
  CommandNull,		/* '}' */
  CommandNull,		/* '~' */
  CommandNull		/* 7/15 */
};

#endif /* __KEYBIND_H__ */
