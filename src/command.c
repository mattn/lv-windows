/*
 * command.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdlib.h>
#include <string.h>

#ifdef MSDOS
#include <dos.h>
#endif /* MSDOS */

#ifdef UNIX
#include <signal.h>
#endif /* UNIX */

#include <import.h>
#include <itable.h>
#include <istr.h>
#include <encode.h>
#include <decode.h>
#include <find.h>
#include <screen.h>
#include <console.h>
#include <display.h>
#include <position.h>
#include <version.h>
#include <dfa.h>
#include <uty.h>
#include <fetch.h>
#include <begin.h>
#include <command.h>

/*#define REGEXPTEST*/

#define COM_FIND_FORWARD	'/'
#define COM_FIND_BACKWARD	'?'

#define IsNumber( c )		( (c) >= '0' && (c) <= '9' )

#ifdef MSDOS
#define HISTORY_SIZE		2
#else
#define HISTORY_SIZE		4
#endif /* MSDOS */

private file_t *f;

private boolean_t quit;
private char *message;
private char *label;

private int historyIndex;

#define STRING_SIZE		128

typedef struct {
  boolean_t	used;
  int		length;
  char		str[ STRING_SIZE ];
} history_t;

private history_t history[ HISTORY_SIZE ];
private history_t saveStr;

public boolean_t CommandInit()
{
  int i;

  historyIndex = 0;
  for( i = 0 ; i < HISTORY_SIZE ; i++ )
    history[ i ].used = FALSE;

  return TRUE;
}

private void Copy( char *toStr, char *fromStr, int length )
{
  int i;

  for( i = 0 ; i < length ; i++ )
    toStr[ i ]= (str_t)fromStr[ i ];
}

private void StrCopy( char *toStr, str_t *fromStr, int length )
{
  int i;

  for( i = 0 ; i < length ; i++ )
    toStr[ i ]= (char)fromStr[ i ];
}

private void CommandCopyToHistory( str_t *str, int length )
{
  history[ historyIndex ].used = TRUE;
  history[ historyIndex ].length = length;

  StrCopy( history[ historyIndex ].str, str, length );

  if( ++historyIndex == HISTORY_SIZE )
    historyIndex = 0;
}

private i_str_t *CommandGetLine( file_t *f, char prompt )
{
  int ch, ptr, iptr, width, index;
  i_str_t *istr;
  char str[ STRING_SIZE ];

  ConsoleGoAhead();
  ConsoleClearRight();
  ConsolePrint( prompt );

  index = historyIndex;

  ptr = 0;
  iptr = 0;
  istr = NULL;
  width = 0;
  for( ; ; ){
    ConsoleFlush();
    ch = ConsoleGetChar();
    switch( ch ){
    case EOF:
    case BEL:
      if( NULL != istr )
	IstrFree( istr );
      return NULL;
    case CR:
      if( NULL == istr )
	return NULL;
      if( NOSET == istr[ 0 ].charset ){
	IstrFree( istr );
	return NULL;
      }
      Encode( istr, 0, iptr,
	     f->keyboardCodingSystem, TRUE,
	     encode_str, &ptr );
      CommandCopyToHistory( encode_str, ptr );
      return istr;
    case BS:
      if( 0 == iptr ){
	if( NULL == istr )
	  return NULL;
	if( NOSET == istr[ 0 ].charset ){
	  IstrFree( istr );
	  return NULL;
	}
	return istr;
      }
      iptr--;
      Encode( istr, 0, iptr,
	     f->keyboardCodingSystem, TRUE,
	     encode_str, &ptr );
      StrCopy( str, encode_str, ptr );
      break;
    case NAK:
      if( NULL != istr )
	IstrFree( istr );
      ptr = 0;
      iptr = 0;
      istr = NULL;
      width = 0;
      break;
    case DLE:
      if( --index < 0 )
	index = HISTORY_SIZE - 1;
      if( FALSE == history[ index ].used )
	continue;
      Copy( str, history[ index ].str, history[ index ].length );
      ptr = history[ index ].length;
      break;
    default:
      if( ptr >= STRING_SIZE || width >= (f->width - 6) ) /* for CNTRL */
	continue;
      str[ ptr++ ] = ch;
    }

    if( NULL != istr )
      IstrFree( istr );

    iptr = ptr;
    istr = Decode( f->keyboardCodingSystem, str, &iptr );

    width = IstrWidth( istr );

    Encode( istr, 0, iptr,
	   f->outputCodingSystem, FALSE,
	   encode_str, &encode_length );

    ConsoleGoAhead();
    ConsoleClearRight();
    ConsolePrint( prompt );
    ConsolePrintsStr( encode_str, encode_length );
  }
}

private boolean_t CommandSaveIstr( i_str_t *istr, char codingSystem )
{
  int length, ptr;

  if( NULL == istr ){
    saveStr.str[ 0 ] = NULL;
    saveStr.length = 0;
    return FALSE;
  }

  for( length = 0 ; NOSET != istr[ length ].charset ; length++ )
    ;

  Encode( istr, 0, length, codingSystem, TRUE, encode_str, &ptr );
  saveStr.length = ptr;
  StrCopy( saveStr.str, encode_str, ptr );

  return TRUE;
}

private i_str_t *CommandRestoreIstr( char codingSystem )
{
  int length;

  length = saveStr.length;

  return Decode( codingSystem, saveStr.str, &length );
}

private boolean_t CommandGetNumber( unsigned int *number, int *newCom )
{
  unsigned int n;
  int ch;
  char buf[ 128 ];

  n = *number;

  for( ; ; ){
    if( 0 == n )
      buf[ 0 ] = NULL;
    else
      sprintf( buf, "%u", n );

    ConsoleGoAhead();
    ConsoleClearRight();
    ConsolePrint( ':' );
    ConsolePrints( buf );

    ConsoleFlush();
    ch = ConsoleGetChar();
    if( EOF == ch )
      return FALSE;
    else if( BS == ch ){
      n /= 10U;
      if( 0 == n )
	return FALSE;
    } else if( !IsNumber( ch ) ){
      *newCom = ch;
      *number = n;
      return TRUE;
    } else {
      n = n * 10U + ch - '0';
    }
  }
}

#define FORWARD		TRUE
#define BACKWARD	FALSE

private char *CommandFindContinue( file_t *f, boolean_t direction )
{
  int res;
  boolean_t first;

  first = f->find.first;

  ConsoleGoAhead();
  ConsoleClearRight();
  ConsolePrints( "Wait a minute..." );
  ConsoleFlush();

  if( FORWARD == direction )
    res = FindForward( f );
  else
    res = FindBackward( f );

  if( -1 == res ){
    return "not found";
  } else if( -2 == res
	    || FALSE == f->find.displayed
	    || ( TRUE == f->find.displayed && TRUE == first )
	    ){
    f->find.displayed = TRUE;
    DisplayFull( f );
  } else {
    if( FORWARD == direction )
      DisplayBot( f, res );
    else
      DisplayTop( f, res );
  }

  return NULL;
}

private char *CommandFindSetPattern( file_t *f, boolean_t direction )
{
  i_str_t *istr;
  char prompt, *res;

  if( FORWARD == direction )
    prompt = COM_FIND_FORWARD;
  else
    prompt = COM_FIND_BACKWARD;

  ConsoleGoAhead();
  ConsoleClearRight();
  istr = CommandGetLine( f, prompt );
  if( NULL == istr ){
    if( TRUE == FindClearPattern( f ) ){
      if( TRUE == f->find.displayed ){
	f->find.displayed = FALSE;
	DisplayFull( f );
      }
    }
    return NULL;
  } else {
    if( NULL == (res = FindSetPattern( f, istr )) )
      res = CommandFindContinue( f, direction );
    if( NULL != res && TRUE == f->find.displayed ){
      DisplayFull( f );
      f->find.displayed = FALSE;
    }
    return res;
  }
}

private void CommandQuit( unsigned int arg )
{
  quit = TRUE;
}

private void CommandShellEscape( unsigned int arg )
{
  i_str_t *istr;
  boolean_t flagPattern = FALSE;
#ifdef MSDOS
  char *shell;
#endif /* MSDOS */

  ConsoleShellEscape();

  if( NULL != f->find.pattern ){
    flagPattern = TRUE;
    CommandSaveIstr( f->find.pattern, f->keyboardCodingSystem );
    FindClearPattern( f );
  }

  FileFreeHead( f );
  FileRefresh( f );
  IstrFreeAll();

#ifdef MSDOS
  if( shell = getenv( "SHELL" ) )
    spawnlp( 0, shell, shell, NULL );
  else if( shell = getenv( "COMSPEC" ) )
    spawnlp( 0, shell, shell, NULL );
  else
    label = "shell unknown";
#endif /* MSDOS */

#ifdef UNIX
  ConsoleSuspend();
#endif /* UNIX */

  ConsoleReturnToProgram();

  f->find.displayed = FALSE;
  if( TRUE == flagPattern ){
    if( NULL != (istr = CommandRestoreIstr( f->keyboardCodingSystem ))
       && NULL == FindSetPattern( f, istr ) )
      f->find.displayed = TRUE;
  }
  ScreenRefresh( f );

  DisplayFull( f );
}

private void CommandTopOfFile( unsigned int arg )
{
  if( TRUE == ScreenTop( f, arg ) )
    DisplayFull( f );
}

private void CommandPercent( unsigned int arg )
{
  if( FALSE == f->done ){
    ConsoleGoAhead();
    ConsoleClearRight();
    ConsolePrints( "Just a moment..." );
    ConsoleFlush();
    if( FALSE == ScreenBot( f ) )
      return;
  }

  if( arg >= 100 )
    arg = 1 + f->totalLines;
  else
    arg = (unsigned int)( (long)( 1 + f->totalLines ) * (long)arg / 100L );

  CommandTopOfFile( arg );
}

private void CommandBottomOfFile( unsigned int arg )
{
  if( 0 < arg ){
    CommandTopOfFile( arg );
    return;
  }

  ConsoleGoAhead();
  ConsoleClearRight();
  ConsolePrints( "Just a moment..." );
  ConsoleFlush();
  if( TRUE == ScreenBot( f ) )
    DisplayFull( f );
}

private void CommandPrev( unsigned int arg )
{
  if( 0 < (arg = ScreenPrev( f, arg )) )
    DisplayTop( f, arg );
}

private void CommandPrevPage( unsigned int arg )
{
  if( 0 == arg )
    arg = f->height;
  CommandPrev( arg );
}

private void CommandPrevHalf( unsigned int arg )
{
  if( 0 == arg )
    arg = f->height / 2;
  CommandPrev( arg );
}

private void CommandPrevLine( unsigned int arg )
{
  if( 0 == arg )
    arg = 1;
  CommandPrev( arg );
}

private void CommandNext( unsigned int arg )
{
  if( 0 < (arg = ScreenNext( f, arg )) )
    DisplayBot( f, arg );
}

private void CommandNextLine( unsigned int arg )
{
  if( 0 == arg )
    arg = 1;
  CommandNext( arg );
}

private void CommandNextHalf( unsigned int arg )
{
  if( 0 == arg )
    arg = f->height / 2;
  CommandNext( arg );
}

private void CommandNextPage( unsigned int arg )
{
  if( 0 == arg )
    arg = f->height;
  CommandNext( arg );
}

private void CommandRepeatForward( unsigned int arg )
{
  if( NULL != f->find.pattern )
    message = CommandFindContinue( f, FORWARD );
}

private void CommandRepeatBackward( unsigned int arg )
{
  if( NULL != f->find.pattern )
    message = CommandFindContinue( f, BACKWARD );
}

private void CommandFindForward( unsigned int arg )
{
  message = CommandFindSetPattern( f, FORWARD );
}

private void CommandFindBackward( unsigned int arg )
{
  message = CommandFindSetPattern( f, BACKWARD );
}

private void CommandFileStatus( unsigned int arg )
{
  position_t pos;

  if( FALSE == f->done ){
    ConsoleGoAhead();
    ConsoleClearRight();
    ConsolePrints( "Just a moment..." );
    ConsoleFlush();

    PositionAssign( pos, f->screen.top );
    ConsoleEnableInterrupt();
    FileStretch( f, PAGE_SIZE * SLOT_SIZE );
    ConsoleDisableInterrupt();
    ScreenTopPhysical( f, &pos );
  }

  label = FileStatus( f );
}

private void CommandVersion( unsigned int arg )
{
  label = VERSION;
}

private void CommandRedisplay( unsigned int arg )
{
  DisplayFull( f );
}

private void CommandRefresh( unsigned int arg )
{
  i_str_t *istr;
  boolean_t flagPattern = FALSE;

  if( NULL != f->find.pattern ){
    flagPattern = TRUE;
    CommandSaveIstr( f->find.pattern, f->keyboardCodingSystem );
    FindClearPattern( f );
  }
  FileFreeHead( f );
  FileRefresh( f );
  IstrFreeAll();
  f->find.displayed = FALSE;
  if( TRUE == flagPattern ){
    if( NULL != (istr = CommandRestoreIstr( f->keyboardCodingSystem ))
       && NULL == FindSetPattern( f, istr ) )
	f->find.displayed = TRUE;
  }
  ScreenRefresh( f );
  DisplayFull( f );
}

private void CommandReload( unsigned int arg )
{
  FILE *fp;
  int logical;
  char *fileName;
  i_str_t *istr;
  boolean_t flagPattern = FALSE;
  char inputCodingSystem, outputCodingSystem, keyboardCodingSystem;

  if( NULL != f->sp ){
    label = "cannot reload non-regular files";
    return;
  }

  if( NULL == (fp = fopen( f->fileName, "rb" )) ){
    label = "cannot reload current file";
    return;
  } else {
    fclose( f->fp );
  }

  ConsoleGoAhead();
  ConsoleClearRight();
  ConsolePrints( "Now reloading..." );
  ConsoleFlush();

  fileName = f->fileName;
  inputCodingSystem = f->inputCodingSystem;
  outputCodingSystem = f->outputCodingSystem;
  keyboardCodingSystem = f->keyboardCodingSystem;

  logical = 1 + f->screen.top.seg * PAGE_SIZE + f->screen.top.off;

  if( NULL != f->find.pattern ){
    flagPattern = TRUE;
    CommandSaveIstr( f->find.pattern, f->keyboardCodingSystem );
    FindClearPattern( f );
  }
  IstrFreeAll();

  FileClose( f );

  f = FileOpen( fileName, fp, NULL,
	       WIDTH, HEIGHT - 1,
	       inputCodingSystem,
	       outputCodingSystem,
	       keyboardCodingSystem );

  f->find.displayed = FALSE;
  if( TRUE == flagPattern ){
    if( NULL != (istr = CommandRestoreIstr( f->keyboardCodingSystem ))
       && NULL == FindSetPattern( f, istr ) ){
      f->find.displayed = TRUE;
      f->find.first = FALSE;
    }
  }
  ScreenTop( f, logical );
  DisplayFull( f );
}

private void CommandNull( unsigned int arg )
{
}

#ifdef REGEXPTEST
private void CommandRegexpNFA( unsigned int arg )
{
  int i;

  for( i = 0 ; i < HEIGHT - 1 ; i++ ){
    ConsoleSetCur( 0, i );
    ConsoleClearRight();
  }
  ConsoleSetCur( 0, 0 );

  ReShowNFA();
}

private void CommandRegexpDFA( unsigned int arg )
{
  int i;

  for( i = 0 ; i < HEIGHT - 1 ; i++ ){
    ConsoleSetCur( 0, i );
    ConsoleClearRight();
  }
  ConsoleSetCur( 0, 0 );

  ReShowDFA();
}
#endif /* REGEXPTEST */

#include <keybind.h>

public void Command( file_t *file )
{
  int com;
  unsigned int arg;

  f = file;

  quit = FALSE;
  message = NULL;
  label = NULL;

  line_truncated = FALSE;
  f->find.displayed = FALSE;

  ScreenTop( f, 0 );
  DisplayFull( f );

  for( ; ; ){
    if( TRUE == kb_interrupted ){
      kb_interrupted = FALSE;
      label = "Interrupted";
      DisplayFull( f );
    }

    if( TRUE == window_changed ){
      window_changed = FALSE;
      CommandRefresh( 0 );
    }

    if( TRUE == f->dirty )
      DisplayFull( f );

    if( TRUE == line_truncated ){
      line_truncated = FALSE;
      label = "There are some truncated lines";
    }

    ConsoleSetCur( 0, HEIGHT - 1 );
    ConsoleClearRight();

    if( NULL == label ){
      if( TRUE == f->bottom ){
	if( TRUE == f->truncated )
	  label = "(TRUNCATED)";
	else
	  label = "(END)";
      }
      if( TRUE == f->top )
	label = f->fileName;
    }

    if( NULL != label ){
      ConsoleSetAttribute( ATTR_STANDOUT );
      ConsolePrints( label );
      ConsoleSetAttribute( NULL );
      label = NULL;
    }

    ConsolePrint( ':' );

    if( NULL != message ){
      ConsoleSetAttribute( ATTR_STANDOUT );
      ConsolePrints( message );
      ConsoleSetAttribute( NULL );
      message = NULL;
    }

    ConsoleFlush();
    com = ConsoleGetChar();
    if( com < 0x00 || com > 0x7f )
      continue;

    arg = 0;
    if( IsNumber( com ) ){
      arg = com - '0';
      if( FALSE == CommandGetNumber( &arg, &com ) )
	continue;
      if( com < 0x00 || com > 0x7f )
	continue;
    }

    ConsoleGoAhead();
    ConsoleClearRight();

    (*keyTable[ com ])( arg );

    if( TRUE == quit )
      break;
  }

  FileClose( f );
}
