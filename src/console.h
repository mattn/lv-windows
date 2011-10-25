/*
 * console.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
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

public char *ansi_standout;
public char *ansi_reverse;
public char *ansi_blink;
public char *ansi_underline;
public char *ansi_hilight;

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

public int ConsolePrint( char i );

public void ConsolePrints( char *str );
public void ConsolePrintsStr( str_t *str, int length );

public void ConsoleFlush();

public int ConsoleGetChar();

public void ConsoleSetCur( int x, int y );	/* zero based */
public void ConsoleOnCur();
public void ConsoleOffCur();
public void ConsoleGoAhead();
public void ConsoleClearRight();
public void ConsoleScrollUp();
public void ConsoleScrollDown();
public void ConsoleSetAttribute( char attr );

#endif /* __CONSOLE_H__ */
