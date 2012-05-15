/*
 * console.h
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio
 * $Id: console.h,v 1.4 2003/11/13 03:08:19 nrt Exp $
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <boolean.h>
#include <str.h>

public int WIDTH, HEIGHT;		/* one based */

public boolean_t allow_ansi_esc;
public boolean_t no_scroll;

public boolean_t allow_interrupt;
public boolean_t kb_interrupted;
public boolean_t window_changed;

public byte *ansi_standout;
public byte *ansi_reverse;
public byte *ansi_blink;
public byte *ansi_underline;
public byte *ansi_hilight;

public byte *cur_left;
public byte *cur_right;
public byte *cur_up;
public byte *cur_down;
public byte *cur_ppage;
public byte *cur_npage;

public void ConsoleInit();
public void ConsoleResetAnsiSequence();

public void ConsoleTermInit();
public void ConsoleSetUp();
public void ConsoleSetDown();

public void ConsoleShellEscape();
public void ConsoleReturnToProgram();

public void ConsoleGetWindowSize();

public void ConsoleSuspend();

public void ConsoleEnableInterrupt();
public void ConsoleDisableInterrupt();

public int ConsolePrint( byte i );

public void ConsolePrints( byte *str );
public void ConsolePrintsStr( str_t *str, int length );

public void ConsoleFlush();

public int ConsoleGetChar();

public void ConsoleSetCur( int x, int y );	/* zero based */
public void ConsoleOnCur();
public void ConsoleOffCur();
public void ConsoleGoAhead();
public void ConsoleClearScreen();
public void ConsoleClearRight();
public void ConsoleScrollUp();
public void ConsoleScrollDown();
public void ConsoleSetAttribute( byte attr );

#endif /* __CONSOLE_H__ */
