/*
 * command.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: command.c,v 1.20 2004/01/05 07:42:31 nrt Exp $
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef MSDOS
#include <dos.h>
#endif /* MSDOS */

#ifdef UNIX
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
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
#include <stream.h>
#include <begin.h>
#include <command.h>

#define LV_FILES_UNLIMITED	-1
#ifndef MSDOS /* if NOT defined */
#define LV_FILES_MAX		LV_FILES_UNLIMITED
#else
#define LV_FILES_MAX		2
#endif /* MSDOS */

#define COM_FIND_FORWARD	'/'
#define COM_FIND_BACKWARD	'?'

#define COM_FILE_PREV		'p'
#define COM_FILE_NEXT		'n'

#define IsNumber( c )		( (c) >= '0' && (c) <= '9' )

#ifdef MSDOS
#define HISTORY_SIZE		2
#else
#define HISTORY_SIZE		4
#endif /* MSDOS */

private file_t *f;		/* current file */

typedef struct FILE_LIST_T {
  char *name;
  file_t *file;
  struct FILE_LIST_T *prev;
  struct FILE_LIST_T *next;
} file_list_t;

private file_list_t *files;

private boolean_t flagQuit;
private byte *message;
private byte *label;

private int historyIndex;

#define STRING_SIZE		256

typedef struct {
  boolean_t	used;
  int		length;
  byte		str[ STRING_SIZE ];
} history_t;

private history_t history[ HISTORY_SIZE ];
private history_t currentPattern;

public boolean_t CommandInit()
{
  int i;

  if ((!(editor_program = getenv("VISUAL"))) &&
      (!(editor_program = getenv("EDITOR")))) {
    editor_program = "vi -c %d";
  }

  historyIndex = 0;
  for( i = 0 ; i < HISTORY_SIZE ; i++ )
    history[ i ].used = FALSE;

  return TRUE;
}

private void Copy( byte *toStr, byte *fromStr, int length )
{
  int i;

  for( i = 0 ; i < length ; i++ )
    toStr[ i ]= (str_t)fromStr[ i ];
}

private void StrCopy( byte *toStr, str_t *fromStr, int length )
{
  int i;

  for( i = 0 ; i < length ; i++ )
    toStr[ i ]= (byte)fromStr[ i ];
}

private void CommandCopyToHistory( str_t *str, int length )
{
  history[ historyIndex ].used = TRUE;
  history[ historyIndex ].length = length;

  StrCopy( history[ historyIndex ].str, str, length );

  if( ++historyIndex == HISTORY_SIZE )
    historyIndex = 0;
}

private i_str_t *CommandGetLine( file_t *f, byte prompt )
{
  int ch, ptr, iptr, width, pre_width, index;
  i_str_t *istr;
  byte str[ STRING_SIZE ];

  ConsoleGoAhead();
  ConsoleClearRight();
  ConsolePrint( prompt );

  index = historyIndex;

  ptr = 0;
  iptr = 0;
  istr = NULL;
  width = 0;
  pre_width = -1;
  for( ; ; ){
    ConsoleFlush();
    ch = ConsoleGetChar();
    switch( ch ){
    case EOF:
    case BEL: /* C-g */
      if( NULL != istr )
	IstrFree( istr );
      return NULL;
    case CR: /* C-m */
      if( NULL == istr )
	return NULL;
      if( NOSET == istr[ 0 ].charset ){
	IstrFree( istr );
	return NULL;
      }
      ptr = CODE_SIZE;
      Encode( istr, 0, iptr,
	     f->keyboardCodingSystem, TRUE,
	     encode_str, &ptr );
      if( ptr > STRING_SIZE )
	ptr = STRING_SIZE;
      CommandCopyToHistory( encode_str, ptr );

      return istr;
    case BS: /* C-h */
      if( 0 == iptr ){
	if( NULL != istr )
	  IstrFree( istr );
	return NULL;
      }
backspace:
      iptr--;
      ptr = CODE_SIZE;
      Encode( istr, 0, iptr,
	     f->keyboardCodingSystem, TRUE,
	     encode_str, &ptr );
      if( ptr > STRING_SIZE )
	ptr = STRING_SIZE;
      StrCopy( str, encode_str, ptr );
      pre_width = -1;
      break;
    case NAK: /* C-u */
      if( NULL != istr )
	IstrFree( istr );
      ptr = 0;
      iptr = 0;
      istr = NULL;
      width = 0;
      pre_width = -1;
      break;
    case DLE: /* C-p */
      if( --index < 0 )
	index = HISTORY_SIZE - 1;
      if( FALSE == history[ index ].used )
	continue;
      Copy( str, history[ index ].str, history[ index ].length );
      ptr = history[ index ].length;
      pre_width = -1;
      break;
    case DEL: /* del */
      if( 0 == ptr ){
	if( NULL != istr )
	  IstrFree( istr );
	return NULL;
      }
      /* fall through */
    default:
      if( ptr >= STRING_SIZE || width >= (f->width - 6) ){ /* for CNTRL */
	if( DEL == ch )
	  goto backspace;
	else
	  continue;
      }
      str[ ptr++ ] = ch;
    }

    if( NULL != istr )
      IstrFree( istr );

    iptr = ptr;
    istr = Decode( IstrAlloc( ZONE_FREE, iptr + 1 ),
		  f->keyboardCodingSystem, str, &iptr );

    width = IstrWidth( istr );

    encode_length = CODE_SIZE;
    Encode( istr, 0, iptr,
	   f->outputCodingSystem, FALSE,
	   encode_str, &encode_length );

    ConsoleGoAhead();
    ConsoleClearRight();
    ConsolePrint( prompt );
    ConsolePrintsStr( encode_str, encode_length );

    if( -1 != pre_width && width < pre_width ){
      /*
       * probably character was deleted.
       * refresh str.
       */
      ptr = CODE_SIZE;
      Encode( istr, 0, iptr,
	     f->keyboardCodingSystem, TRUE,
	     encode_str, &ptr );
      if( ptr > STRING_SIZE )
	ptr = STRING_SIZE;
      StrCopy( str, encode_str, ptr );
    }
    pre_width = width;
  }
}

private boolean_t CommandSavePattern()
{
  int length, ptr;
  i_str_t *istr;

  istr = f->find.pattern;

  if( NULL == istr ){
    currentPattern.used = FALSE;
    return FALSE;
  }

  currentPattern.used = TRUE;

  for( length = 0 ; NOSET != istr[ length ].charset ; length++ )
    ;

  ptr = CODE_SIZE;
  Encode( istr, 0, length, f->keyboardCodingSystem, TRUE, encode_str, &ptr );
  currentPattern.length = ptr;
  StrCopy( currentPattern.str, encode_str, ptr );

  FindClearPattern( f );

  return TRUE;
}

private boolean_t CommandRestorePattern()
{
  i_str_t *istr;
  int length;

  f->find.displayed = TRUE/*FALSE*/;

  if( TRUE == currentPattern.used ){
    length = currentPattern.length;
    istr = Decode( IstrAlloc( ZONE_FREE, length + 1 ),
		  f->keyboardCodingSystem,
		  currentPattern.str,
		  &length );
    if( NULL != istr && NULL == FindSetPattern( f, istr ) ){
      f->find.first = FALSE;
      return TRUE;
    }
  }

  return FALSE;
}

private boolean_t CommandGetNumber( unsigned int *number, int *newCom )
{
  unsigned int n;
  int ch;
  byte buf[ 128 ];

  n = *number;

  for( ; ; ){
    if( 0 == n )
      buf[ 0 ] = 0x00;
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
    else if( BS == ch || DEL == ch ){
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

private byte *CommandFindContinue( file_t *f, boolean_t direction )
{
  int res;
  boolean_t first;

  first = f->find.first;	/* first を保存する必要あり */

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
	    || TRUE == first
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

private byte *CommandFindSetPattern( file_t *f, boolean_t direction )
{
  i_str_t *istr;
  byte prompt, *res;

  if( FORWARD == direction )
    prompt = COM_FIND_FORWARD;
  else
    prompt = COM_FIND_BACKWARD;

  ConsoleGoAhead();
  ConsoleClearRight();
  istr = CommandGetLine( f, prompt );
  if( NULL == istr ){
    /*
     * パターン有りから無し
     */
    if( TRUE == FindClearPattern( f ) ){
      if( TRUE == f->find.displayed ){
	/*
	 * 画面上のサーチ結果をクリアする.
	 */
	f->find.displayed = FALSE;
	DisplayFull( f );
      }
    }
    return NULL;
  } else {
    /*
     * パターンのセット
     */
    if( NULL == (res = FindSetPattern( f, istr )) ){
      /*
       * パターン有り & パターン一致あり
       */
      res = CommandFindContinue( f, direction );
    }
    /*
     * パターン有り & パターン一致無し
     */
    if( NULL != res && TRUE == f->find.displayed ){
	/*
	 * 画面上のサーチ結果をクリアする.
	 */
      f->find.displayed = FALSE;
      DisplayFull( f );
    }
    return res;
  }
}

private void CommandQuit( unsigned int arg )
{
  flagQuit = TRUE;
}

private void CommandShellEscape( unsigned int arg )
{
#ifdef MSDOS
  byte *shell;
#endif /* MSDOS */

  ConsoleShellEscape();

  CommandSavePattern();
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

  CommandRestorePattern();
  ScreenRefresh( f );
  DisplayFull( f );
}

private void CommandReload( unsigned int arg )
{
  FILE *fp;
  int logical;
  byte *fileName;
  byte inputCodingSystem, outputCodingSystem;
  byte keyboardCodingSystem, pathnameCodingSystem;
  byte defaultCodingSystem;
  stream_t st;

  if( NULL != f->sp ){
    label = "cannot reload non-regular files";
    return;
  }

  if( NULL == (fp = fopen( f->fileName, "rb" )) ){
    label = "cannot reload current file";
    return;
  } else {
    fclose( f->fp );
    st.fp = fp;
    st.sp = f->sp;
    st.pid = f->pid;
  }

  ConsoleGoAhead();
  ConsoleClearRight();
  ConsolePrints( "Now reloading..." );
  ConsoleFlush();

  fileName = f->fileName;
  inputCodingSystem = f->inputCodingSystem;
  outputCodingSystem = f->outputCodingSystem;
  keyboardCodingSystem = f->keyboardCodingSystem;
  pathnameCodingSystem = f->pathnameCodingSystem;
  defaultCodingSystem = f->defaultCodingSystem;

  logical = 1 + f->screen.top.seg * LV_PAGE_SIZE + f->screen.top.off;

  CommandSavePattern();

  FileDetach( f );

  f = FileAttach( fileName, &st,
		  WIDTH, HEIGHT - 1,
 		  inputCodingSystem,
		  outputCodingSystem,
		  keyboardCodingSystem,
		  pathnameCodingSystem,
	          defaultCodingSystem );

  files->file = f;
  FilePreload( f );

  CommandRestorePattern();
  ScreenTop( f, logical );
  DisplayFull( f );
}

private int CommandLaunchEditor( byte *editor, byte *filename, int line )
{
#define COM_SIZE 128
#define ARG_SIZE 64
  int argc;
  byte *ptr, *nptr, *argv[ ARG_SIZE ];
  byte com[ COM_SIZE ];
  byte num[ COM_SIZE ];
#ifndef MSDOS /* NOT DEFINED */
  int pid, status;
#endif /* MSDOS */

  if( strlen( editor ) + strlen( filename ) + 2 > COM_SIZE )
    return 1;

  strcpy( com, editor );

  ptr = com;
  argc = 0;
  while( 0x00 != *ptr && argc < ARG_SIZE - 2 ){
    argv[ argc ] = ptr;
    while( ' ' != *ptr && 0x00 != *ptr )
      ptr++;
    if( 0x00 != *ptr ){
      *ptr++ = 0x00;
      while( ' ' == *ptr && 0x00 != *ptr )
	ptr++;
    }
    if( 0x00 != (nptr = strchr( argv[ argc ], '%' )) && 'd' == *(++nptr ) ){
      sprintf( num, argv[ argc ], line );
      argv[ argc ] = num;
    }
    argc++;
  }
  argv[ argc++ ] = filename;
  argv[ argc ] = 0x00;

#ifdef NOT
  {
    int i;

    ConsolePrint( '!' );
    for( i = 0 ; i < argc ; i++ )
      ConsolePrints( argv[ i ] ), ConsolePrint( ' ' );
    ConsolePrint( '\r' ); ConsolePrint( '\n' );
    ConsoleFlush();
  }
#endif /* NOT */

#ifdef MSDOS
  return spawnvp( 0, argv[ 0 ], argv );
#else
  if( 0 == (pid = fork()) ){
    execvp( argv[ 0 ], (char **)argv );
    exit( 1 );
  } else if( -1 == pid ){
    return 1;
  } else {
    int rv;
    do {
	rv = wait( &status );
    } while (rv == -1 && errno == EINTR);
    return status;
  }
#endif /* MSDOS */
}

private void CommandEdit( unsigned int arg )
{
  byte *fileName;
  int lineNumber;

  if( NULL != f->sp ){
    label = "cannot edit non-regular files";
    return;
  }

  if( NULL == editor_program ){
    if( NULL == (editor_program = getenv( "EDITOR" )) ){
      label = "editor unknown";
      return;
    }
  }

  ConsoleShellEscape();

  fileName = f->fileName;
  lineNumber = 1 + f->screen.top.seg * LV_PAGE_SIZE + f->screen.top.off;

  CommandSavePattern();
  FileRefresh( f );
  IstrFreeAll();

  if( 0 != CommandLaunchEditor( editor_program, fileName, lineNumber ) )
    label = "editor failed";

  kb_interrupted = FALSE;

  ConsoleReturnToProgram();

  CommandRestorePattern();

  CommandReload( 0 );
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
    arg = (unsigned int)( (unsigned long)( 1 + f->totalLines )
			 * (unsigned long)arg / 100L );

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

private void CommandPoll( unsigned int arg )
{
  long pos;

  kb_interrupted = FALSE;

  if( NULL != f->sp ){
    label = "cannot poll non-regular files";
    return;
  }

  if( access( f->fileName, 0 ) ){
    label = "cannot access current file";
    return;
  }

  while( 1 ){
    CommandReload(arg);
    CommandBottomOfFile( arg );
    ConsoleGoAhead();
    ConsoleClearRight();
    ConsoleSetAttribute( ATTR_STANDOUT );
    ConsolePrints( "Waiting for data, break (^C) to return..." );
    ConsoleSetAttribute( 0 );
    ConsoleFlush();

    (void)fseek( f->fp, 0, SEEK_END );
    pos = ftell( f->fp );

    ConsoleEnableInterrupt();

    while( 1 ){
      usleep(300000); // 300msec
      if( kb_interrupted )
	break;

      (void)fseek( f->fp, 0, SEEK_END );
      if( ftell( f->fp ) > pos ){
	// it grew
	break;
      }
    }

    ConsoleDisableInterrupt();

    if( kb_interrupted )
      break;
  }

  // execute that just to be sure, also it will clear the
  // "waiting..." message.
  kb_interrupted = FALSE;
  CommandReload(arg);
  CommandBottomOfFile(arg);
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
    FileStretch( f, LV_PAGE_SIZE * SLOT_SIZE );
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
  CommandSavePattern();
  FileRefresh( f );
  IstrFreeAll();

  CommandRestorePattern();
  ScreenRefresh( f );
  DisplayFull( f );
}

private void CommandToggleHz( unsigned int arg )
{
  static byte oldCodingSystem = EUC_CHINA;

  if( FALSE == hz_detection ){
    oldCodingSystem = f->inputCodingSystem;
    f->inputCodingSystem = HZ_GB;
    hz_detection = TRUE;
  } else {
    f->inputCodingSystem = oldCodingSystem;
    hz_detection = FALSE;
  }
  CommandRefresh( arg );
}

private void CommandToggleCset( unsigned int arg )
{
  if( C_TABLE_SIZE == ++f->inputCodingSystem )
    f->inputCodingSystem = AUTOSELECT + 1;
  CommandRefresh( arg );
}

private void CommandReverseCset( unsigned int arg )
{
  if( 0 == f->inputCodingSystem || 0 == --f->inputCodingSystem )
    f->inputCodingSystem = C_TABLE_SIZE - 1;
  CommandRefresh( arg );
}

private void CommandCursor( unsigned int arg )
{
  int ch, step;
  boolean_t flagLeft, flagRight, flagUp, flagDown, flagPpage, flagNpage;

  flagLeft = flagRight = flagUp = flagDown = flagPpage = flagNpage = TRUE;
  step = 1;
  for( ; ; ){
    ch = ConsoleGetChar();
    if( NULL == cur_left || ch != cur_left[ step ] )
      flagLeft = FALSE;
    if( NULL == cur_right || ch != cur_right[ step ] )
      flagRight = FALSE;
    if( NULL == cur_up || ch != cur_up[ step ] )
      flagUp = FALSE;
    if( NULL == cur_down || ch != cur_down[ step ] )
      flagDown = FALSE;
    if( NULL == cur_ppage || ch != cur_ppage[ step ] )
      flagPpage = FALSE;
    if( NULL == cur_npage || ch != cur_npage[ step ] )
      flagNpage = FALSE;
    if( FALSE == flagLeft && FALSE == flagRight
	&& FALSE == flagUp && FALSE == flagDown
	&& FALSE == flagPpage && FALSE == flagNpage ){
      break;
    }
    step++;
    if( TRUE == flagLeft && 0x00 == cur_left[ step ] ){
      CommandPrevHalf( arg );
      break;
    }
    if( TRUE == flagRight && 0x00 == cur_right[ step ] ){
      CommandNextHalf( arg );
      break;
    }
    if( TRUE == flagUp && 0x00 == cur_up[ step ] ){
      CommandPrevLine( arg );
      break;
    }
    if( TRUE == flagDown && 0x00 == cur_down[ step ] ){
      CommandNextLine( arg );
      break;
    }
    if( TRUE == flagPpage && 0x00 == cur_ppage[ step ] ){
      CommandPrevPage( arg );
      break;
    }
    if( TRUE == flagNpage && 0x00 == cur_npage[ step ] ){
      CommandNextPage( arg );
      break;
    }
  }
}

private void CommandColon( unsigned int arg )
{
  int ch;
  stream_t *st;
  file_list_t *next_target;

  ch = ConsoleGetChar();

  if( COM_FILE_PREV == ch || COM_FILE_NEXT == ch ){
    if( 0 < arg ){
      if( LV_FILES_UNLIMITED != LV_FILES_MAX && LV_FILES_MAX < arg )
	arg = LV_FILES_MAX;
    } else {
      arg = 1;
    }
  }

  if( COM_FILE_PREV == ch ){
    while( arg ){
      files = files->prev;
      arg--;
    }
  } else if( COM_FILE_NEXT == ch ){
    while( arg ){
      files = files->next;
      arg--;
    }
  }

  if( COM_FILE_PREV == ch || COM_FILE_NEXT == ch ){
    CommandSavePattern();
    FileRefresh( f );
    IstrFreeAll();

    while( NULL == files->file ){
      if( NULL == (st = StreamOpen( files->name )) ){
	if( COM_FILE_PREV == ch )
	  next_target = files->prev;
	else
	  next_target = files->next;

	files->prev->next = files->next;
	files->next->prev = files->prev;
	free( files );

	files = next_target;

	continue;
      }

      files->file = FileAttach( files->name, st,
			        f->width, f->height,
			        f->inputCodingSystem,
			        f->outputCodingSystem,
			        f->keyboardCodingSystem,
			        f->pathnameCodingSystem,
			        f->defaultCodingSystem );
      FilePreload( files->file );
      ScreenTop( files->file, 0 );
      FileRefresh( files->file );
      IstrFreeAll();
    }

    f = files->file;

    f->find.displayed = FALSE;
    CommandRestorePattern();
    ScreenRefresh( f );
    DisplayFull( f );

    return;
  }
}

private void CommandNull( unsigned int arg )
{
}

#ifdef REGEXP_TEST
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
#endif /* REGEXP_TEST */

#include <keybind.h>

private void CreateFileList( file_t *file, byte **argv )
{
  file_list_t *list, *tail;
  int count = 0;

  tail = files;

  while( *(++argv) ){
    ++count;
    if( LV_FILES_UNLIMITED != LV_FILES_MAX && LV_FILES_MAX < count ){
      fprintf( stderr, "lv: too many files\n" );
      break;
    }

    list = (file_list_t *)Malloc( sizeof( file_list_t ) );

    if( access( *argv, 0 ) ){
      perror( *argv );
      count--;
      continue;
    }

    list->name = *argv;
    list->file = NULL;

    list->next = files;
    files->prev = list;
    list->prev = tail;
    tail->next = list;
    tail = list;
  }
}

public void Command( file_t *file, byte **optional )
{
  int com;
  unsigned int arg;

  /*
   * initialize file list
   */
  files = (file_list_t *)Malloc( sizeof( file_list_t ) );
  files->file = file;
  files->prev = files;
  files->next = files;

  if( NULL != optional )
    CreateFileList( file, optional );

  /*
   * startup
   */
  currentPattern.used = FALSE;

  flagQuit = FALSE;
  message = NULL;
  label = NULL;
  line_truncated = FALSE;

  f = file;

  f->find.displayed = FALSE;
  FilePreload( f );

  ConsoleSetUp();

  ScreenTop( f, 0 );
  DisplayFull( f );

  /*
   * let's start
   */
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
	label = FileName( f );
    }

    if( NULL != label ){
      ConsoleSetAttribute( ATTR_STANDOUT );
      ConsolePrints( label );
      ConsoleSetAttribute( 0 );
      label = NULL;
    }

    ConsolePrint( ':' );

    if( NULL != message ){
      ConsoleSetAttribute( ATTR_STANDOUT );
      ConsolePrints( message );
      ConsoleSetAttribute( 0 );
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

    if( TRUE == flagQuit )
      break;
  }
}
