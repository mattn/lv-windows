/*
 * console.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#ifdef MSDOS
#include <dos.h>
#endif /* MSDOS */

#ifdef WINDOWS
#include <windows.h>

#ifndef FOREGROUND_WHITE
#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#endif
#ifndef BACKGROUND_WHITE
#define BACKGROUND_WHITE (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
#endif
#ifndef FOREGROUND_BLACK
#define FOREGROUND_BLACK	FOREGROUND_INTENSITY
#endif
#ifndef BACKGROUND_BLACK
#define BACKGROUND_BLACK	BACKGROUND_INTENSITY
#endif
#ifndef FOREGROUND_MASK
#define FOREGROUND_MASK (FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_INTENSITY)
#endif
#ifndef BACKGROUND_MASK
#define BACKGROUND_MASK (BACKGROUND_RED|BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_INTENSITY)
#endif
#endif

#ifdef UNIX
#include <unistd.h>
#ifdef OLDBSD
#include <sys/ioctl.h>
#else
#include <termios.h>
#endif /* OLDBSD */
#endif /* UNIX */

#ifdef TERMCAP
#define TGETNUM
#endif /* TERMCAP */

#ifdef TERMINFO
#include <termio.h>
#include <curses.h>
#include <term.h>
#endif /* TERMINFO */

#include <import.h>
#include <ascii.h>
#include <attr.h>
#include <begin.h>
#include <console.h>

#define ANSI_ATTR_LENGTH	8

#ifdef MSDOS
private char tbuf[ 16 ];

private char *clr_eol			= "\x1b[K";
private char *delete_line		= "\x1b[M";
private char *insert_line		= "\x1b[L";
private char *enter_standout_mode	= "\x1b[7m";
private char *enter_underline_mode	= "\x1b[4m";
private char *enter_bold_mode		= "\x1b[1m";
private char *exit_attribute_mode	= "\x1b[m";
private char *cursor_visible		= NULL;
private char *cursor_invisible		= NULL;
#endif /* MSDOS */

#ifdef WINDOWS
private WORD console_attr = 0;
private HANDLE stdout_handle = NULL;
private HANDLE console_handle = NULL;
private DWORD initial_mode;
#endif

#ifdef UNIX
#ifdef OLDBSD
private struct sgttyb ttyOld, ttyNew;
#else
private struct termios ttyOld, ttyNew;
#endif /* OLDBSD */

#ifdef BSD
int (*foo)(char) = ConsolePrint;
#endif /* BSD */

#ifdef SYSV
#ifdef IRIX
private int (*foo)(int) = putchar;
#else
private int (*foo)(char) = putchar;
#endif /* IRIX */
#endif /* SYSV */

#endif /* UNIX */

#ifdef TERMCAP
private char entry[ 1024 ];
private char func[ 1024 ];

extern char *tgetstr(), *tgoto();
extern int tgetent(), tgetflag(), tgetnum(), tputs();

private char *cursor_address		= NULL;
private char *clr_eol			= NULL;
private char *insert_line		= NULL;
private char *delete_line		= NULL;
private char *enter_standout_mode	= NULL;
private char *exit_standout_mode	= NULL;
private char *enter_underline_mode	= NULL;
private char *exit_underline_mode	= NULL;
private char *enter_bold_mode		= NULL;
private char *exit_attribute_mode	= NULL;
private char *cursor_visible		= NULL;
private char *cursor_invisible		= NULL;
private char *terminit			= NULL;
private char *termend			= NULL;
#endif /* TERMCAP */

public void ConsoleInit()
{
  allow_interrupt	= FALSE;
  kb_interrupted	= FALSE;
  window_changed	= FALSE;
}

public void ConsoleResetAnsiSequence()
{
  ansi_standout		= "7";
  ansi_reverse		= "7";
  ansi_blink		= "5";
  ansi_underline	= "4";
  ansi_hilight		= "1";
}

#ifdef MSDOS
private void InterruptIgnoreHandler( int arg )
{
  signal( SIGINT, InterruptIgnoreHandler );
}
#endif /* MSDOS */

private void InterruptHandler( int arg )
{
  kb_interrupted = TRUE;

#ifndef BSD /* IF NOT DEFINED */
  signal( SIGINT, InterruptHandler );
#endif /* BSD */
}

public void ConsoleEnableInterrupt()
{
#ifdef MSDOS
  allow_interrupt = TRUE;
  signal( SIGINT, InterruptHandler );
#endif /* MSDOS */

#ifdef UNIX
  signal( SIGTSTP, SIG_IGN );
#ifdef OLDBSD
  ttyNew.sg_flags &= ~RAW;
  ioctl( 0, TIOCSETN, &ttyNew );
#else
  ttyNew.c_lflag |= ISIG;
  tcsetattr( 0, TCSADRAIN, &ttyNew );
#endif /* OLDBSD */
#endif /* UNIX */
}

public void ConsoleDisableInterrupt()
{
#ifdef MSDOS
  allow_interrupt = FALSE;
  signal( SIGINT, InterruptIgnoreHandler );
#endif /* MSDOS */

#ifdef UNIX
#ifdef OLDBSD
  ttyNew.sg_flags |= RAW;
  ioctl( 0, TIOCSETN, &ttyNew );
#else
  ttyNew.c_lflag &= ~ISIG;
  tcsetattr( 0, TCSADRAIN, &ttyNew );
#endif /* OLDBSD */
  signal( SIGTSTP, SIG_DFL );
#endif /* UNIX */
}

public void ConsoleGetWindowSize()
{
#ifdef UNIX
  struct winsize winSize;

  ioctl( 0, TIOCGWINSZ, &winSize );
  WIDTH = winSize.ws_col;
  HEIGHT = winSize.ws_row;
  if( 0 >= WIDTH || 0 >= HEIGHT ){
#ifdef TGETNUM
    WIDTH = tgetnum( "columns" );
    HEIGHT = tgetnum( "lines" );
#else
    WIDTH = tigetnum( "columns" );
    HEIGHT = tigetnum( "lines" );
#endif /* TGETNUM */
    if( 0 >= WIDTH || 0 >= HEIGHT )
      WIDTH = 80, HEIGHT = 24;
  }
#endif /* UNIX */
 
#ifdef WINDOWS
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  SMALL_RECT rect;
  GetConsoleScreenBufferInfo( console_handle, &csbi );
  WIDTH = csbi.dwSize.X;
  HEIGHT = csbi.dwSize.Y - 1;
#endif
}

#ifdef UNIX
private void WindowChangeHandler( int arg )
{
  window_changed = TRUE;

  ConsoleGetWindowSize();

#ifdef SYSV
  signal( SIGWINCH, WindowChangeHandler );
#endif /* SYSV */
}
#endif /* UNIX */

public void ConsoleTermInit()
{
#ifdef MSDOS
  char *ptr;

#define ANSI		0
#define FMRCARD		1

  int term = ANSI;

  if( ptr = getenv("TERM") ){
    if( !strcmp( ptr, "fmr4020" ) ){
      term = FMRCARD;
      WIDTH = 40;
      HEIGHT = 19;
    } else if( !strcmp( ptr, "fmr8025" ) ){
      term = FMRCARD;
      WIDTH = 80;
      HEIGHT = 24;
    }
  }

  switch( term ){
  case FMRCARD:
    delete_line      = "\x1bR";
    insert_line      = "\x1b" "E";
    cursor_visible   = "\x1b[v";
    cursor_invisible = "\x1b[1v";
    break;
  default:
    WIDTH  = 80;
    HEIGHT = 24;
  }
#endif /* MSDOS */

#ifdef WINDOWS
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  stdout_handle = GetStdHandle( STD_OUTPUT_HANDLE );
  GetConsoleScreenBufferInfo( stdout_handle, &csbi );
  SMALL_RECT rect;
  rect = csbi.srWindow;
  WIDTH = rect.Right - rect.Left;
  HEIGHT = rect.Bottom - rect.Top;
#endif /* WINDOWS */

#ifdef TERMCAP
  char *term;
  char *ptr;

  if( NULL == (term = getenv( "TERM" )) )
    fprintf( stderr, "lv: environment variable TERM is required\n" ), exit( -1 );
  if( 0 >= tgetent( entry, term ) )
    fprintf( stderr, "lv: %s not found in termcap\n", term ), exit( -1 );

  ConsoleGetWindowSize();

  ptr = func;

  cursor_address	= tgetstr( "cm", &ptr );
  clr_eol		= tgetstr( "ce", &ptr );
  insert_line		= tgetstr( "al", &ptr );
  delete_line		= tgetstr( "dl", &ptr );
  enter_standout_mode	= tgetstr( "so", &ptr );
  exit_standout_mode	= tgetstr( "se", &ptr );
  enter_underline_mode	= tgetstr( "us", &ptr );
  exit_underline_mode	= tgetstr( "ue", &ptr );
  enter_bold_mode	= tgetstr( "md", &ptr );
  exit_attribute_mode	= tgetstr( "me", &ptr );
  cursor_visible	= tgetstr( "ve", &ptr );
  cursor_invisible	= tgetstr( "vi", &ptr );
  terminit		= tgetstr( "ti", &ptr );
  termend		= tgetstr( "te", &ptr );

  if( NULL == cursor_address || NULL == clr_eol )
    fprintf( stderr, "lv: termcap cm, ce are required\n" ), exit( -1 );

  if( NULL == insert_line || NULL == delete_line )
    no_scroll = TRUE;

  if( terminit )
    tputs( terminit, 1, foo );
#endif /* TERMCAP */

#ifdef TERMINFO
  char *term;
  int state;

  if( NULL == (term = getenv( "TERM" )) )
    fprintf( stderr, "lv: environment variable TERM is required\n" ), exit( -1 );

  setupterm( term, 1, &state );
  if( 1 != state )
    fprintf( stderr, "lv: cannot initialize terminal\n" ), exit( -1 );

  def_shell_mode();

  ConsoleGetWindowSize();

  if( NULL == cursor_address || NULL == clr_eol )
    fprintf( stderr, "lv: terminfo cursor_address, clr_eol are required\n" ), exit( -1 );

  if( NULL == insert_line || NULL == delete_line )
    no_scroll = TRUE;

  if( enter_ca_mode )
    tputs( enter_ca_mode, 1, foo );
#endif /* TERMINFO */
}

public void ConsoleSetUp()
{
#ifdef MSDOS
  signal( SIGINT, InterruptIgnoreHandler );
#endif /* MSDOS */

#ifdef WINDOWS
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  SMALL_RECT rect;
  COORD coord;
  DWORD mode, written;
  console_handle = CreateConsoleScreenBuffer(
		  GENERIC_READ | GENERIC_WRITE, 0, NULL,
		  CONSOLE_TEXTMODE_BUFFER, NULL );
  GetConsoleScreenBufferInfo( console_handle, &csbi );
  console_attr = csbi.wAttributes;
  rect = csbi.srWindow;
  coord.X = rect.Right - rect.Left;
  coord.Y = rect.Bottom - rect.Top;
  SetConsoleScreenBufferSize( console_handle, coord );
  GetConsoleMode( GetStdHandle( STD_INPUT_HANDLE ), &initial_mode );
  mode = initial_mode;
  mode &= ~ENABLE_LINE_INPUT;
  mode &= ~ENABLE_ECHO_INPUT;
  mode &= ~ENABLE_PROCESSED_INPUT;
  SetConsoleMode( GetStdHandle( STD_INPUT_HANDLE ), mode );
  SetConsoleActiveScreenBuffer( console_handle );
#endif

#ifdef UNIX

#ifdef BSD
  struct sigvec sigVec;

  sigVec.sv_handler = WindowChangeHandler;
  sigVec.sv_mask = sigmask( SIGWINCH );
  sigVec.sv_flags = SV_INTERRUPT;
  sigvec( SIGWINCH, &sigVec, NULL );

  sigVec.sv_handler = InterruptHandler;
  sigVec.sv_mask = sigmask( SIGINT );
  sigVec.sv_flags = SV_INTERRUPT;
  sigvec( SIGINT, &sigVec, NULL );
#endif /* BSD */

#ifdef SYSV
  signal( SIGWINCH, WindowChangeHandler );
  signal( SIGINT, InterruptHandler );
#endif /* SYSV */

#ifdef OLDBSD
  ioctl( 0, TIOCGETP, &ttyOld );
  ttyNew = ttyOld;
  ttyNew.sg_flags &= ~ECHO;
  ttyNew.sg_flags |= RAW;
  ttyNew.sg_flags |= CRMOD;
  ioctl( 0, TIOCSETN, &ttyNew );
#else
  tcgetattr( 0, &ttyOld );
  ttyNew = ttyOld;
  ttyNew.c_iflag &= ~ISTRIP;
  ttyNew.c_iflag &= ~INLCR;
  ttyNew.c_iflag &= ~ICRNL;
  ttyNew.c_iflag &= ~IGNCR;
  ttyNew.c_lflag &= ~ISIG;
  ttyNew.c_lflag &= ~ICANON;
  ttyNew.c_lflag &= ~ECHO;
  ttyNew.c_cc[ VMIN ] = 1;
  ttyNew.c_cc[ VTIME ] = 0;
  tcsetattr( 0, TCSADRAIN, &ttyNew );
#endif /* OLDBSD */
#endif /* UNIX */
}

public void ConsoleSetDown()
{
#ifdef MSDOS
  ConsoleSetCur( 0, HEIGHT - 1 );
  ConsolePrint( CR );
  ConsolePrint( LF );
#endif /* MSDOS */

#ifdef WINDOWS
  CloseHandle( console_handle );
  SetConsoleMode( GetStdHandle( STD_INPUT_HANDLE ), initial_mode );
#endif /* WINDOWS */

#ifdef UNIX
#ifdef OLDBSD
  ioctl( 0, TIOCSETN, &ttyOld );
#else
  tcsetattr( 0, TCSADRAIN, &ttyOld );
#endif /* OLDBSD */
#endif /* UNIX */

#ifdef TERMCAP
  if( termend )
    tputs( termend, 1, foo );
  else {
    ConsoleSetCur( 0, HEIGHT - 1 );
    ConsolePrint( CR );
    ConsolePrint( LF );
  }
#endif /* TERMCAP */

#ifdef TERMINFO
  reset_shell_mode();
  if( exit_ca_mode )
    tputs( exit_ca_mode, 1, foo );
  else {
    ConsoleSetCur( 0, HEIGHT - 1 );
    ConsolePrint( CR );
    ConsolePrint( LF );
  }
#endif /* TERMINFO */
}

public void ConsoleShellEscape()
{
#if defined(MSDOS) || defined(WINDOWS)
  ConsoleSetCur( 0, HEIGHT - 1 );
#endif /* MSDOS,WINDOWS */

#ifdef UNIX
#ifdef OLDBSD
  ioctl( 0, TIOCSETN, &ttyOld );
#else
  tcsetattr( 0, TCSADRAIN, &ttyOld );
#endif /* OLDBSD */
#endif /* UNIX */

#ifdef TERMCAP
  if( termend )
    tputs( termend, 1, foo );
  else
    ConsoleSetCur( 0, HEIGHT - 1 );
#endif /* TERMCAP */

#ifdef TERMINFO
  reset_shell_mode();
  if( exit_ca_mode )
    tputs( exit_ca_mode, 1, foo );
  else
    ConsoleSetCur( 0, HEIGHT - 1 );
#endif /* TERMINFO */

  ConsoleFlush();
}

public void ConsoleReturnToProgram()
{
#ifdef TERMCAP
  if( terminit )
    tputs( terminit, 1, foo );
#endif /* TERMCAP */

#ifdef TERMINFO
  if( enter_ca_mode )
    tputs( enter_ca_mode, 1, foo );
#endif /* TERMINFO */

#ifdef UNIX
#ifdef OLDBSD
  ioctl( 0, TIOCSETN, &ttyNew );
#else
  tcsetattr( 0, TCSADRAIN, &ttyNew );
#endif /* OLDBSD */
#endif /* UNIX */
}

public void ConsoleSuspend()
{
#ifdef BSD
  int pgrp;

  ioctl( 0, TIOCGPGRP, &pgrp );
  killpg( pgrp, SIGSTOP );
#endif /* BSD */

#ifdef SYSV
  kill( - getpgrp(), SIGSTOP );
#endif /* SYSV */
}

public int ConsoleGetChar()
{
#if defined(MSDOS) || defined(WINDOWS)
  return getch();
#endif /* MSDOS,WINDOWS */

#ifdef UNIX
  char buf;

  if( 0 > read( 0, &buf, 1 ) )
    return EOF;
  else
    return (int)buf;
#endif /* UNIX */
}

public int ConsolePrint( char c )
{
#ifdef MSDOS
  bdos( 0x06, 0xff != c ? c : 0, 0 );
#endif /* MSDOS */

#ifdef WINDOWS
  DWORD written;
  WriteFile( console_handle, &c, 1, &written, NULL );
#endif /* WINDOWS */

#ifdef UNIX
  return putchar( c );
#endif /* UNIX */
}

public void ConsolePrints( char *str )
{
#ifdef MSDOS
  while( *str )
    ConsolePrint( *str++ );
#endif /* MSDOS */

#ifdef WINDOWS
  while( *str )
    ConsolePrint( *str++ );
#endif /* WINDOWS */

#ifdef UNIX
  printf( "%s", str );
#endif /* UNIX */
}

public void ConsolePrintsStr( str_t *str, int length )
{
  int i;
  char attr, lastAttr;

  attr = lastAttr = ATTR_NULL;
  for( i = 0 ; i < length ; i++ ){
    attr = ( 0xff00 & str[ i ] ) >> 8;
    if( lastAttr != attr )
      ConsoleSetAttribute( attr );
    lastAttr = attr;
    ConsolePrint( 0xff & str[ i ] );
  }
  if( NULL != attr )
    ConsoleSetAttribute( 0 );
}

public void ConsoleFlush()
{
#ifdef UNIX
  fflush( stdout );
#endif /* UNIX */
}

public void ConsoleSetCur( int x, int y )
{
#ifdef MSDOS
  sprintf( tbuf, "\x1b[%d;%dH", y + 1, x + 1 );
  ConsolePrints( tbuf );
#endif /* MSDOS */

#ifdef WINDOWS
  COORD coord;
  if( x != -1 ) coord.X = x;
  if( y != -1 ) coord.Y = y;
  SetConsoleCursorPosition( console_handle, coord );
#endif /* WINDOWS */

#ifdef TERMCAP
  tputs( tgoto( cursor_address, x, y ), 1, foo );
#endif /* TERMCAP */

#ifdef TERMINFO
  tputs( tparm( cursor_address, y, x ), 1, foo );
#endif /* TERMINFO */
}

public void ConsoleOnCur()
{
#ifdef MSDOS
  if( cursor_visible )
    ConsolePrints( cursor_visible );
#endif /* MSDOS */

#ifdef WINDOWS
  CONSOLE_CURSOR_INFO cci;
  GetConsoleCursorInfo( console_handle, &cci );
  cci.bVisible = TRUE;
  SetConsoleCursorInfo( console_handle, &cci );
#endif /* WINDOWS */

#ifdef UNIX
  if( cursor_visible )
    tputs( cursor_visible, 1, foo );
#endif /* UNIX */
}

public void ConsoleOffCur()
{
#ifdef MSDOS
  if( cursor_invisible )
    ConsolePrints( cursor_invisible );
#endif /* MSDOS */

#ifdef WINDOWS
  CONSOLE_CURSOR_INFO cci;
  GetConsoleCursorInfo( console_handle, &cci );
  cci.bVisible = FALSE;
  SetConsoleCursorInfo( console_handle, &cci );
#endif /* WINDOWS */

#ifdef UNIX
  if( cursor_invisible )
    tputs( cursor_invisible, 1, foo );
#endif /* UNIX */
}

public void ConsoleClearRight()
{
#ifdef MSDOS
  ConsolePrints( clr_eol );
#endif /* MSDOS */

#ifdef WINDOWS
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  COORD coord;
  DWORD size, written;
  GetConsoleScreenBufferInfo( console_handle, &csbi );
  coord = csbi.dwCursorPosition;
  size = csbi.dwSize.X - coord.X;
  FillConsoleOutputCharacter( console_handle, ' ', size, coord, &written );
  FillConsoleOutputAttribute( console_handle,
    csbi.wAttributes, size, coord, &written );
  SetConsoleCursorPosition( console_handle, csbi.dwCursorPosition );
#endif /* WINDOWS */

#ifdef UNIX
  tputs( clr_eol, 1, foo );
#endif /* UNIX */
}

public void ConsoleGoAhead()
{
#ifdef MSDOS
  ConsolePrint( 0x0d );
#endif /* MSDOS */

#ifdef WINDOWS
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  COORD coord;
  GetConsoleScreenBufferInfo( console_handle, &csbi );
  csbi.dwCursorPosition.X = 0;
  SetConsoleCursorPosition( console_handle, csbi.dwCursorPosition );
#endif /* WINDOWS */

#ifdef UNIX
  putchar( 0x0d );
#endif /* UNIX */
}

public void ConsoleScrollUp()
{
#ifdef MSDOS
  if( delete_line )
    ConsolePrints( delete_line );
#endif /* MSDOS */

#ifdef WINDOWS
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  SMALL_RECT rect;
  CHAR_INFO ci;
  COORD coord;
  GetConsoleScreenBufferInfo( console_handle, &csbi );
  rect.Top = csbi.dwCursorPosition.Y;
  rect.Bottom = csbi.srWindow.Bottom - 1;
  rect.Left = 0;
  rect.Right = csbi.dwSize.X - 1;
  coord.X = 0;
  coord.Y = rect.Top - 1;
  ci.Char.AsciiChar = ' ';
  ci.Attributes = csbi.wAttributes;
  ScrollConsoleScreenBuffer( console_handle, &rect, &rect, coord, &ci );
  SetConsoleCursorPosition( console_handle, csbi.dwCursorPosition );
#endif /* WINDOWS */

#ifdef UNIX
  if( delete_line )
    tputs( delete_line, 1, foo );
#endif /* UNIX */
}

public void ConsoleScrollDown()
{
#ifdef MSDOS
  if( insert_line )
    ConsolePrints( insert_line );
#endif /* MSDOS */

#ifdef WINDOWS
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  SMALL_RECT rect;
  CHAR_INFO ci;
  COORD coord;
  GetConsoleScreenBufferInfo( console_handle, &csbi );
  rect.Top = csbi.dwCursorPosition.Y;
  rect.Bottom = csbi.srWindow.Bottom - 1;
  rect.Left = 0;
  rect.Right = csbi.dwSize.X - 1;
  coord.X = 0;
  coord.Y = rect.Top + 1;
  ci.Char.AsciiChar = ' ';
  ci.Attributes = csbi.wAttributes;
  ScrollConsoleScreenBuffer( console_handle, &rect, &rect, coord, &ci );
  SetConsoleCursorPosition( console_handle, csbi.dwCursorPosition );
#endif /* WINDOWS */

#ifdef UNIX
  if( insert_line )
    tputs( insert_line, 1, foo );
#endif /* UNIX */
}

private char prevAttr = NULL;

public void ConsoleSetAttribute( char attr )
{
#ifndef MSDOS /* IF NOT DEFINED */
  if( TRUE == allow_ansi_esc ){
#endif /* MSDOS */
    ConsolePrints( "\x1b[0" );
    if( NULL != attr ){
      if( ATTR_STANDOUT & attr ){
	ConsolePrint( ';' );
	ConsolePrints( ansi_standout );
      } else if( ATTR_COLOR & attr ){
	if( ATTR_REVERSE & attr ){
	  if( ATTR_COLOR_B & attr ){
	    ConsolePrints( ";30;4" );
	    ConsolePrint( ( ATTR_COLOR & attr ) + '0' );
	  } else {
	    ConsolePrints( ";37;4" );
	    ConsolePrint( ( ATTR_COLOR & attr ) + '0' );
	  }
	} else {
	  ConsolePrints( ";3" );
	  ConsolePrint( ( ATTR_COLOR & attr ) + '0' );
	}
      } else if( ATTR_REVERSE & attr ){
	ConsolePrint( ';' );
	ConsolePrints( ansi_reverse );
      }
      if( ATTR_BLINK & attr ){
	ConsolePrint( ';' );
	ConsolePrints( ansi_blink );
      }
      if( ATTR_UNDERLINE & attr ){
	ConsolePrint( ';' );
	ConsolePrints( ansi_underline );
      }
      if( ATTR_HILIGHT & attr ){
	ConsolePrint( ';' );
	ConsolePrints( ansi_hilight );
      }
    }
    ConsolePrint( 'm' );
#ifndef MSDOS /* IF NOT DEFINED */
#ifdef WINDOWS
  } else {
    /*
     * non ansi sequence
     */
    WORD attr_new = FOREGROUND_WHITE;
    if( ATTR_STANDOUT & attr ){
      attr_new =
	((attr_new & FOREGROUND_MASK) << 4) |
	((attr_new & BACKGROUND_MASK) >> 4);
    } else if( ATTR_COLOR & attr ){
      if( ATTR_REVERSE & attr ){
	if( ATTR_COLOR_B & attr ){
	  attr_new = (attr_new & BACKGROUND_MASK)
	    | FOREGROUND_INTENSITY;
		;
	  if((ATTR_COLOR & attr) & 1) attr_new |= BACKGROUND_RED;
	  if((ATTR_COLOR & attr) & 2) attr_new |= BACKGROUND_GREEN;
	  if((ATTR_COLOR & attr) & 4) attr_new |= BACKGROUND_BLUE;
	} else {
	  attr_new = (attr_new & BACKGROUND_MASK)
	    | FOREGROUND_INTENSITY | FOREGROUND_WHITE;
	  if((ATTR_COLOR & attr) & 1) attr_new |= BACKGROUND_RED;
	  if((ATTR_COLOR & attr) & 2) attr_new |= BACKGROUND_GREEN;
	  if((ATTR_COLOR & attr) & 4) attr_new |= BACKGROUND_BLUE;
	}
      } else {
	  attr_new = (attr_new & BACKGROUND_MASK);
	  if((ATTR_COLOR & attr) & 1) attr_new |= FOREGROUND_RED;
	  if((ATTR_COLOR & attr) & 2) attr_new |= FOREGROUND_GREEN;
	  if((ATTR_COLOR & attr) & 4) attr_new |= FOREGROUND_BLUE;
      }
    } else if( ATTR_REVERSE & attr ){
      attr_new =
	((attr_new & FOREGROUND_MASK) << 4) |
	((attr_new & BACKGROUND_MASK) >> 4);
    }
    if( ATTR_BLINK & attr ){
      attr_new |= FOREGROUND_INTENSITY;
    }
    if( ATTR_UNDERLINE & attr ){
      attr_new |= FOREGROUND_INTENSITY;
    }
    if( ATTR_HILIGHT & attr ){
      attr_new |= FOREGROUND_INTENSITY;
    }
    SetConsoleTextAttribute( console_handle, attr_new );
  }
  prevAttr = attr;
#else
  } else {
    /*
     * non ansi sequence
     */
    if( ( ATTR_HILIGHT & prevAttr ) && 0 == ( ATTR_HILIGHT & attr ) )
      if( exit_attribute_mode )
	tputs( exit_attribute_mode, 1, foo );
    if( ( ATTR_UNDERLINE & prevAttr ) && 0 == ( ATTR_UNDERLINE & attr ) )
      if( exit_underline_mode )
	tputs( exit_underline_mode, 1, foo );
    if( ( ATTR_STANDOUT & prevAttr ) && 0 == ( ATTR_STANDOUT & attr ) )
      if( exit_standout_mode )
	tputs( exit_standout_mode, 1, foo );

    if( ATTR_HILIGHT & attr )
      if( enter_bold_mode )
	tputs( enter_bold_mode, 1, foo );
    if( ATTR_UNDERLINE & attr )
      if( enter_underline_mode )
	tputs( enter_underline_mode, 1, foo );
    if( ATTR_STANDOUT & attr )
      if( enter_standout_mode )
	tputs( enter_standout_mode, 1, foo );
  }
#endif
  prevAttr = attr;
#endif /* MSDOS */
}
