/*
 * lv.c
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef UNIX
#include <unistd.h>
#include <fcntl.h>
#endif /* UNIX */

#ifdef MSDOS
#include <dos.h>
#endif /* MSDOS */

#include <import.h>
#include <conv.h>
#include <itable.h>
#include <ctable.h>
#include <istr.h>
#include <file.h>
#include <decode.h>
#include <encode.h>
#include <console.h>
#include <command.h>
#include <re.h>
#include <find.h>
#include <unimap.h>
#include <uty.h>
#include <display.h>
#include <version.h>
#include <begin.h>

/*#define DEBUG*/

#define DEFAULT_INPUT_CODING_SYSTEM	AUTOSELECT

#define LV_HELP		"lv.hlp"
private char *lvHelpFile;

#ifdef UNIX
#define DEFAULT_OUTPUT_CODING_SYSTEM	ISO_2022_JP
#define DEFAULT_KEYBOARD_CODING_SYSTEM	ISO_2022_JP

#define LV_CONF		".lv"
#define LV_HELP_PATH	"/usr/local/lib/lv/"
#endif /* UNIX */

#ifdef MSDOS
#define DEFAULT_OUTPUT_CODING_SYSTEM	SHIFT_JIS
#define DEFAULT_KEYBOARD_CODING_SYSTEM	SHIFT_JIS

#define HEAP_SIZE	16384U

#define LV_CONF 	"_lv"
#endif /* MSDOS */

private char *filter = "zcat";

#define BUF_SIZE 	128

private void LvCopyright()
{
  fprintf( stderr,
	  "# lv " VERSION "\n"
	  "# All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio\n"
	  );
}

private void LvInit( char **argv )
{
  ItableInit();
  CtableInit();
  IstrInit();
  CommandInit();
  ConsoleInit();

#ifdef MSDOS
  {
    int i;

    lvHelpFile = Malloc( strlen( argv[ 0 ] ) + strlen( LV_HELP ) + 1 );
    strcpy( lvHelpFile, argv[ 0 ] );
    for( i = strlen( lvHelpFile ) - 1 ; i >= 0 ; i-- ){
      if( '/' == lvHelpFile[ i ] || '\\' == lvHelpFile[ i ] ){
	i++;
	break;
      }
    }
    if( i < 0 ) i = 0;
    lvHelpFile[ i ] = NULL;
    strcat( lvHelpFile, LV_HELP );
  }
#else
  lvHelpFile = Malloc( strlen( LV_HELP_PATH LV_HELP ) + 1 );
  strcpy( lvHelpFile, LV_HELP_PATH LV_HELP );
#endif /* MSDOS */
}

private void UnknownOption( char *s, char *location )
{
  fprintf( stderr, "lv: unknown option %s in %s\n", s, location );
  exit( -1 );
}

private void SetCodingSystem( char *s, char *codingSystem, char *location )
{
  switch( *( s + 1 ) ){
  case 'e':
    /*
     * extended unix code
     */
    switch( *( s + 2 ) ){
    case 'c': *codingSystem = EUC_CHINA;	break;
    case 'j': *codingSystem = EUC_JAPAN;	break;
    case 'k': *codingSystem = EUC_KOREA;	break;
    case 't': *codingSystem = EUC_TAIWAN;	break;
    default:
      *codingSystem = EUC_JAPAN;
    }
    break;
  case 'l':
    /*
     * iso-8859
     */
    switch( *( s + 2 ) ){
    case '1': *codingSystem = ISO_8859_1;	break;
    case '2': *codingSystem = ISO_8859_2;	break;
    case '3': *codingSystem = ISO_8859_3;	break;
    case '4': *codingSystem = ISO_8859_4;	break;
    case '5': *codingSystem = ISO_8859_5;	break;
    case '6': *codingSystem = ISO_8859_6;	break;
    case '7': *codingSystem = ISO_8859_7;	break;
    case '8': *codingSystem = ISO_8859_8;	break;
    case '9': *codingSystem = ISO_8859_9;	break;
    default:
      *codingSystem = ISO_8859_1;
    }
    break;
  case 'c': *codingSystem = ISO_2022_CN;	break;
  case 'j': *codingSystem = ISO_2022_JP;	break;
  case 'k': *codingSystem = ISO_2022_KR;	break;
  case 'm':
  case 's': *codingSystem = SHIFT_JIS;		break;
  case 'b': *codingSystem = BIG_FIVE;		break;
#ifndef MSDOS /* IF NOT DEFINED */
  case 'u':
    switch( *( s + 2 ) ){
    case '7': *codingSystem = UTF_7;		break;
    case '8': *codingSystem = UTF_8;		break;
    default:
      *codingSystem = UTF_8;
    }
    break;
#endif /* MSDOS */
  case 'r': *codingSystem = RAW;		break;
  default:
    UnknownOption( s, location );
  }
}

private void SetAnsiSequence( char *s, char *location )
{
  switch( *( s + 1 ) ){
  case 's': ansi_standout = TokenAlloc( s + 2 ); 	break;
  case 'r': ansi_reverse = TokenAlloc( s + 2 );		break;
  case 'b': ansi_blink = TokenAlloc( s + 2 );		break;
  case 'u': ansi_underline = TokenAlloc( s + 2 );	break;
  case 'h': ansi_hilight = TokenAlloc( s + 2 );		break;
  default:
    UnknownOption( s, location );
  }
}

typedef struct {
  char		*file;
  FILE		*fp;
  FILE		*sp;
  int		width;
  int		height;
  boolean_t	options;
  char		inputCodingSystem;
  char		outputCodingSystem;
  char		keyboardCodingSystem;
} conf_t;

private void LvConfInit( conf_t *conf )
{
  conf->file	= NULL;
  conf->fp	= NULL;
  conf->sp	= NULL;
  conf->width	= -1;
  conf->height	= -1;
  conf->options	= TRUE;
  conf->inputCodingSystem	= DEFAULT_INPUT_CODING_SYSTEM;
  conf->outputCodingSystem	= DEFAULT_OUTPUT_CODING_SYSTEM;
  conf->keyboardCodingSystem	= DEFAULT_KEYBOARD_CODING_SYSTEM;

#ifndef MSDOS /* IF NOT DEFINED */
  unicode_width_threshold = 0x3000;
  unimap_iso8859 = FALSE;
#endif /* MSDOS */

  allow_unify		= FALSE;
  immediate_print	= FALSE;
  regexp_search		= TRUE;
  casefold_search	= FALSE;
  allow_ansi_esc	= FALSE;
  no_scroll		= FALSE;
  smooth_paging		= FALSE;

  ConsoleResetAnsiSequence();
}

private void LvConfArg( conf_t *conf, char **argv, char *location )
{
  char *s;

  if( TRUE == conf->options && '-' == **argv ){
    s = *argv + 1;
    if( NULL == *s )
      conf->options = FALSE;
    while( *s ){
      switch( *s ){
      case 'A':
	SetCodingSystem( s, &conf->inputCodingSystem, location );
	conf->keyboardCodingSystem = conf->outputCodingSystem
	  = conf->inputCodingSystem;
	break;
      case 'I':
	if( 'a' == *( s + 1 ) )
	  conf->inputCodingSystem = AUTOSELECT;
	else
	  SetCodingSystem( s, &conf->inputCodingSystem, location );
	break;
      case 'K':
	SetCodingSystem( s, &conf->keyboardCodingSystem, location );
	break;
      case 'O':
	SetCodingSystem( s, &conf->outputCodingSystem, location );
	break;
      case 'S': SetAnsiSequence( s, location ); break;
      case 'W': conf->width = atoi( s + 1 ); break;
      case 'H': conf->height = atoi( s + 1 ); break;
#ifndef MSDOS /* IF NOT DEFINED */
      case 'T': unicode_width_threshold = (ic_t)atoi( s + 1 ); break;
      case 'm': unimap_iso8859 = TRUE; s++; continue;
#endif /* MSDOS */
      case 'c': allow_ansi_esc = TRUE; s++; continue;
      case 'd': casefold_search = TRUE; s++; continue;
      case 'f': regexp_search = FALSE; s++; continue;
      case 'p': immediate_print = TRUE; s++; continue;
      case 's': smooth_paging = TRUE; s++; continue;
      case 'u': allow_unify = TRUE; s++; continue;
      case 'z': no_scroll = TRUE; s++; continue;
      case '@': LvConfInit( conf ); s++; continue;
      case 'h': conf->file = lvHelpFile; break;
      case 'v': LvCopyright(); exit( 0 );
      case SP:
      case HT:
	break;
      default:
	UnknownOption( s, location );
      }
      do {
	s++;
	if( '-' == *s || '+' == *s ){
	  s++;
	  break;
	}
      } while( *s );
    }
  } else if( TRUE == conf->options && '+' == **argv ){
    s = *argv + 1;
    while( *s ){
      switch( *s ){
#ifndef MSDOS /* IF NOT DEFINED */
      case 'm': unimap_iso8859 = FALSE; s++; continue;
#endif /* MSDOS */
      case 'c': allow_ansi_esc = FALSE; s++; continue;
      case 'd': casefold_search = FALSE; s++; continue;
      case 'f': regexp_search = TRUE; s++; continue;
      case 'p': immediate_print = FALSE; s++; continue;
      case 's': smooth_paging = FALSE; s++; continue;
      case 'u': allow_unify = FALSE; s++; continue;
      case 'z': no_scroll = FALSE; s++; continue;
      case SP:
      case HT:
	break;
      default:
	UnknownOption( s, location );
      }
      do {
	s++;
	if( '-' == *s || '+' == *s ){
	  s++;
	  break;
	}
      } while( *s );
    }
  } else {
    conf->file = *argv;
  }
}

private void LvConfFile( conf_t *conf, char *file )
{
  FILE *fp;
  int i;
  char *argv[ 1 ], buf[ BUF_SIZE ];

  if( NULL == (fp = fopen( file, "r" )) )
    return;

  while( NULL != fgets( buf, BUF_SIZE, fp ) ){
    if( '#' == *buf )
      continue;
    for( i = strlen( buf ) - 1 ; i >= 0 ; i-- ){
      if( buf[ i ] < SP )
	buf[ i ] = NULL;
      else
	break;
    }
    *argv = buf;
    LvConfArg( conf, argv, file );
  }

  fclose( fp );
}

private void LvConf( conf_t *conf, char **argv )
{
  char *ptr, buf[ BUF_SIZE ];

  LvConfInit( conf );

  if( NULL != (ptr = getenv( "HOME" )) ){
    strcpy( buf, ptr );
    strcat( buf, "/" LV_CONF );
    LvConfFile( conf, buf );
  }

  LvConfFile( conf, LV_CONF );

  if( NULL != (*argv = getenv( "LV" )) )
    LvConfArg( conf, argv, "environment" );

  conf->file = NULL;

  while( *++argv )
    LvConfArg( conf, argv, "argument" );
}

private void StdinDuplicationFailed()
{
  fprintf( stderr, "lv: stdin duplication failed\n" );
  exit( -1 );
}

private boolean_t LvOpen( conf_t *conf )
{
  char *exts;

  if( NULL != (exts = Exts( conf->file ))
     && ( !strcmp( "gz", exts )
	 || !strcmp( "z", exts ) || !strcmp( "Z", exts ) ) ){
    /*
     * zcat
     */
    if( NULL == (conf->fp = (FILE *)tmpfile()) )
      perror( "temporary file" ), exit( -1 );

#ifdef MSDOS
    { int sout;

      sout = dup( 1 );
      close( 1 );
      dup( fileno( conf->fp ) );
      if( 0 > spawnlp( 0, filter, filter, conf->file, NULL ) )
	FatalErrorOccurred();
      close( 1 );
      dup( sout );
      rewind( conf->fp );

      return TRUE;
    }
#endif /* MSDOS */

#ifdef UNIX
    { int fds[ 2 ], pid;

      if( 0 > pipe( fds ) )
	perror( "pipe" ), exit( -1 );

      switch( pid = fork() ){
      case -1:
	perror( "fork" ), exit( -1 );
      case 0:
	/*
	 * child process
	 */
	close( fds[ 0 ] );
	close( 1 );
	dup( fds[ 1 ] );
	if( 0 > execlp( filter, filter, conf->file, NULL ) )
	  perror( "zcat" ), exit( -1 );
	/*
	 * never reach here
	 */
      default:
	/*
	 * parent process
	 */
	close( fds[ 1 ] );
	if( NULL == (conf->sp = fdopen( fds[ 0 ], "r" )) )
	  StdinDuplicationFailed();

	return TRUE;
      }
    }
#endif /* UNIX */
  }

  if( NULL == (conf->fp = fopen( conf->file, "rb" )) )
    perror( conf->file ), exit( -1 );

  return TRUE;
}

private boolean_t LvReconnect( conf_t *conf )
{
  if( NULL != conf->file ){
    LvOpen( conf );
  } else {
#ifdef MSDOS
    conf->file = "(stdin)";
    if( NULL == (conf->fp = fdopen( dup( 0 ), "rb" )) )
      StdinDuplicationFailed();
    close( 0 );
    dup( 1 );
#endif /* MSDOS */
#ifdef UNIX
    struct stat sbuf;

    conf->file = "(stdin)";
    fstat( 0, &sbuf );
    if( S_IFREG == ( sbuf.st_mode & S_IFMT ) ){
      /* regular */
      if( NULL == (conf->fp = fdopen( dup( 0 ), "r" )) )
	StdinDuplicationFailed();
    } else {
      /* socket */
      if( NULL == (conf->fp = (FILE *)tmpfile()) )
	perror( "temporary file" ), exit( -1 );
      if( NULL == (conf->sp = fdopen( dup( 0 ), "r" )) )
	StdinDuplicationFailed();
    }
    close( 0 );
    if( 0 != open( "/dev/tty", O_RDONLY ) )
      perror( "/dev/tty" ), exit( -1 );
#endif /* UNIX */
  }

  return TRUE;
}

public int main( int argc, char **argv )
{
  file_t *f;
  conf_t *conf;

#ifdef MSDOS
  free( Malloc( HEAP_SIZE ) );
#endif /* MSDOS */

  LvInit( argv );

  conf = Malloc( sizeof( conf_t ) );
  LvConf( conf, argv );

  if( NULL == conf->file && IsAtty( 0 ) ){
    LvCopyright();
    fprintf( stderr, "You can easily try lv by typing ``lv -h'' on your terminal.\n" );
    exit( -1 );
  }

  LvReconnect( conf );

  if( IsAtty( 1 ) ){
    binary_decode = FALSE;

    ConsoleTermInit();
    FindSetup();

    if( conf->width > 0 ) WIDTH = conf->width;
    if( conf->height > 0 ) HEIGHT = conf->height;

    if( HEIGHT > PAGE_SIZE * ( BLOCK_SIZE - 1 ) )
      HEIGHT = PAGE_SIZE * ( BLOCK_SIZE - 1 );

    f = FileOpen( conf->file, conf->fp, conf->sp,
		 WIDTH, HEIGHT - 1,
		 conf->inputCodingSystem,
		 conf->outputCodingSystem,
		 conf->keyboardCodingSystem );
    ConsoleSetUp();
    Command( f );
    ConsoleSetDown();
  } else {
    binary_decode = TRUE;
    immediate_print = FALSE;

    f = FileOpen( conf->file, conf->fp, conf->sp,
		 0, 0,
		 conf->inputCodingSystem,
		 conf->outputCodingSystem,
		 conf->keyboardCodingSystem );
    Conv( f );
  }

  return 0;
}
