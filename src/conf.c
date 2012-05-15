/*
 * conf.c
 *
 * All rights reserved. Copyright (C) 1998 by NARITA Tomio.
 * $Id: conf.c,v 1.16 2004/01/05 07:23:29 nrt Exp $
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

#include <import.h>
#include <itable.h>
#include <unimap.h>
#include <find.h>
#include <display.h>
#include <guess.h>
#include <console.h>
#include <file.h>
#include <kana.h>
#include <decode.h>
#include <command.h>
#include <conv.h>
#include <uty.h>
#include <version.h>
#ifdef HAVE_SETLOCALE
#include <locale.h>
#include <guesslocale.h>
#endif
#include <begin.h>
#include <conf.h>

#define DEFAULT_INPUT_CODING_SYSTEM	AUTOSELECT

#ifdef UNIX
#define DEFAULT_OUTPUT_CODING_SYSTEM	EUC_JAPAN	/* ja specific */
#define DEFAULT_KEYBOARD_CODING_SYSTEM	EUC_JAPAN
#define DEFAULT_PATHNAME_CODING_SYSTEM	EUC_JAPAN
#define DEFAULT_DEFAULT_CODING_SYSTEM	EUC_JAPAN

#define LV_CONF		".lv"
#endif /* UNIX */

#ifdef MSDOS
#define DEFAULT_OUTPUT_CODING_SYSTEM	SHIFT_JIS
#define DEFAULT_KEYBOARD_CODING_SYSTEM	SHIFT_JIS
#define DEFAULT_PATHNAME_CODING_SYSTEM	SHIFT_JIS
#define DEFAULT_DEFAULT_CODING_SYSTEM	EUC_JAPAN

#define LV_CONF 	"_lv"
#endif /* MSDOS */

#define BUF_SIZE 	128

#define LV_HELP		"lv.hlp"
/*#define LV_HELP_PATH	"/usr/local/lib/lv/"*/ /* now defined through make */

private byte *lvHelpFile[ 2 ];

private void ConfInitArgs( conf_t *conf )
{
#ifdef HAVE_SETLOCALE
  byte localeCodingSystem;
  char dummy[6];
  char *nl;
  int use_locale = 1;

  nl = setlocale( LC_CTYPE, "" );
  if (NULL == nl || !strcmp("C", nl))
    use_locale = 0;
  else
    localeCodingSystem = LocaleCodingSystem(dummy);
#endif

  conf->file	= NULL;
  conf->st	= NULL;
  conf->width	= -1;
  conf->height	= -1;
  conf->options	= TRUE;
  conf->inputCodingSystem	= DEFAULT_INPUT_CODING_SYSTEM;
#ifdef HAVE_SETLOCALE
  if (use_locale) {
    conf->keyboardCodingSystem	= localeCodingSystem;
    conf->outputCodingSystem	= localeCodingSystem;
    conf->pathnameCodingSystem	= localeCodingSystem;
    conf->defaultCodingSystem	= localeCodingSystem;
  } else
#endif
  {
    conf->keyboardCodingSystem	= DEFAULT_KEYBOARD_CODING_SYSTEM;
    conf->outputCodingSystem	= DEFAULT_OUTPUT_CODING_SYSTEM;
    conf->pathnameCodingSystem	= DEFAULT_PATHNAME_CODING_SYSTEM;
    conf->defaultCodingSystem	= DEFAULT_DEFAULT_CODING_SYSTEM;
  }
  conf->keyCodingSystemVirgin	= TRUE;

#ifndef MSDOS /* IF NOT DEFINED */
  unicode_width_threshold = 0x3000;	/* itable_t.h */
  unimap_iso8859 = FALSE;		/* unimap.h */
#endif /* MSDOS */

  casefold_search	= TRUE;		/* find.h */
  regexp_search		= TRUE;		/* find.h */
  carefully_divide	= TRUE;		/* display.h */
  adjust_charset	= TRUE;		/* guess.h */
  no_scroll		= TRUE;		/* console.h */

  allow_unify		= FALSE;	/* itable_t.h */
  allow_ansi_esc	= FALSE;	/* console.h */
  line_number		= FALSE;	/* conv.h */
  grep_inverted		= FALSE;	/* conv.h */
  kana_conv		= FALSE;	/* kana.h */
  smooth_paging		= FALSE;	/* display.h */
  hz_detection		= FALSE;	/* decode.h */

  ConsoleResetAnsiSequence();
}

private void UnknownOption( byte *s, byte *location )
{
  fprintf( stderr, "lv: unknown option %s in %s\n", s, location );
  exit( -1 );
}

private void SetCodingSystem( byte *s, byte *codingSystem, byte *location )
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
    case '0': *codingSystem = ISO_8859_10;	break;
    case 'b': *codingSystem = ISO_8859_11;	break;
    case 'd': *codingSystem = ISO_8859_13;	break;
    case 'e': *codingSystem = ISO_8859_14;	break;
    case 'f': *codingSystem = ISO_8859_15;	break;
    case 'g': *codingSystem = ISO_8859_16;	break;
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
  case 'h':
  case 'z':
    hz_detection = TRUE;
    *codingSystem = HZ_GB;			break;
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

private void SetAnsiSequence( byte *s, byte *location )
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

private void ConfArg( conf_t *conf, byte **argv, byte *location )
{
  byte *s, quotationChar;
#ifdef HAVE_SETLOCALE
  char dummy[6];
#endif

  if( TRUE == conf->options && '-' == **argv ){
    s = *argv + 1;
    if( 0x00 == *s )
      conf->options = FALSE;
    while( *s ){
      switch( *s ){
      case 'A':
	SetCodingSystem( s, &conf->inputCodingSystem, location );
	conf->keyboardCodingSystem = conf->outputCodingSystem =
	  conf->pathnameCodingSystem = conf->defaultCodingSystem =
	    conf->inputCodingSystem;
	break;
      case 'I':
	if( 'a' == *( s + 1 ) )
	  conf->inputCodingSystem = AUTOSELECT;
	else
	  SetCodingSystem( s, &conf->inputCodingSystem, location );
	break;
      case 'O':
#ifdef HAVE_SETLOCALE
	if( 'a' == *( s + 1 ) )
	  conf->outputCodingSystem = LocaleCodingSystem(dummy);
	else
	  SetCodingSystem( s, &conf->outputCodingSystem, location );
#else
	SetCodingSystem( s, &conf->outputCodingSystem, location );
#endif
	break;
      case 'K':
	SetCodingSystem( s, &conf->keyboardCodingSystem, location );
	conf->keyCodingSystemVirgin = FALSE;
	break;
      case 'P':
	SetCodingSystem( s, &conf->pathnameCodingSystem, location );
	break;
      case 'D':
	SetCodingSystem( s, &conf->defaultCodingSystem, location );
	break;
      case 'S': SetAnsiSequence( s, location ); break;
      case 'W': conf->width = atoi( s + 1 ); break;
      case 'H': conf->height = atoi( s + 1 ); break;
      case 'E':
	if( '\'' == *( s + 1 ) ||  '"' == *( s + 1 ) ){
	  quotationChar = *( s + 1 );
	  editor_program = TokenAlloc( s + 1 );
	  s += 2;
	  while( 0x00 != *s && quotationChar != *s )
	    s++;
	} else {
	  editor_program = Malloc( strlen( s + 1 ) + 1 );
	  strcpy( editor_program, s + 1 );
	}
	break;
#ifndef MSDOS /* IF NOT DEFINED */
      case 'T': unicode_width_threshold = (ic_t)atoi( s + 1 ); break;
      case 'm': unimap_iso8859 = TRUE; s++; continue;
#endif /* MSDOS */
      case 'a': adjust_charset = TRUE; s++; continue;
      case 'c': allow_ansi_esc = TRUE; s++; continue;
      case 'd': casefold_search = TRUE; s++; continue;
      case 'i': casefold_search = TRUE; s++; continue;
      case 'f': regexp_search = FALSE; s++; continue;
      case 'g': grep_mode = TRUE; s++; continue;
      case 'k': kana_conv = TRUE; s++; continue;
      case 'l': carefully_divide = FALSE; s++; continue;
      case 'n': line_number = TRUE; s++; continue;
      case 'q': no_scroll = FALSE; s++; continue;
      case 's': smooth_paging = TRUE; s++; continue;
      case 'u': allow_unify = TRUE; s++; continue;
      case 'v': grep_inverted = TRUE; s++; continue;
      case 'z': hz_detection = TRUE; s++; continue;
      case '+': ConfInitArgs( conf ); s++; continue;
      case 'h': conf->file = lvHelpFile; break;
      case 'V': Banner(); exit( 0 );
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
      case 'a': adjust_charset = FALSE; s++; continue;
      case 'c': allow_ansi_esc = FALSE; s++; continue;
      case 'd': casefold_search = FALSE; s++; continue;
      case 'i': casefold_search = FALSE; s++; continue;
      case 'f': regexp_search = TRUE; s++; continue;
      case 'g': grep_mode = FALSE; s++; continue;
      case 'k': kana_conv = FALSE; s++; continue;
      case 'l': carefully_divide = TRUE; s++; continue;
      case 'n': line_number = FALSE; s++; continue;
      case 'q': no_scroll = TRUE; s++; continue;
      case 's': smooth_paging = FALSE; s++; continue;
      case 'u': allow_unify = FALSE; s++; continue;
      case 'v': grep_inverted = FALSE; s++; continue;
      case 'z': hz_detection = FALSE; s++; continue;
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
    if( TRUE == grep_mode && NULL == conf->pattern )
      conf->pattern = *argv;
    else if( NULL == conf->file )
      conf->file = argv;
  }
}

private void ConfFile( conf_t *conf, byte *file )
{
  FILE *fp;
  int i;
  byte *argv[ 1 ], buf[ BUF_SIZE ];

  if( NULL == (fp = fopen( file, "r" )) )
    return;

  while( NULL != fgets( buf, BUF_SIZE, fp ) ){
    if( '#' == *buf )
      continue;
    for( i = strlen( buf ) - 1 ; i >= 0 ; i-- ){
      if( buf[ i ] < SP )
	buf[ i ] = 0x00;
      else
	break;
    }
    *argv = buf;
    ConfArg( conf, argv, file );
  }

  fclose( fp );
}

public void Conf( conf_t *conf, byte **argv )
{
  byte *ptr, buf[ BUF_SIZE ];

  ConfInitArgs( conf );

  if( NULL != (ptr = getenv( "HOME" )) ){
    strcpy( buf, ptr );
    strcat( buf, "/" LV_CONF );
    ConfFile( conf, buf );
  }

#ifdef MSDOS
  ConfFile( conf, LV_CONF );
#endif

  if( NULL != (*argv = getenv( "LV" )) )
    ConfArg( conf, argv, "environment" );

  conf->file = NULL;
  conf->pattern = NULL;

  while( *++argv )
    ConfArg( conf, argv, "argument" );
}

public void ConfInit( byte **argv )
{
#define helpFile	(*lvHelpFile)
  lvHelpFile[ 1 ] = NULL;

#ifdef MSDOS
  {
    int i;

    helpFile = Malloc( strlen( argv[ 0 ] ) + strlen( LV_HELP ) + 1 );
    strcpy( helpFile, argv[ 0 ] );
    for( i = strlen( helpFile ) - 1 ; i >= 0 ; i-- ){
      if( '/' == helpFile[ i ] || '\\' == helpFile[ i ] ){
	i++;
	break;
      }
    }
    if( i < 0 ) i = 0;
    helpFile[ i ] = NULL;
    strcat( helpFile, LV_HELP );
  }
#else
  helpFile = Malloc( strlen( LV_HELP_PATH "/" LV_HELP ) + 1 );
  strcpy( helpFile, LV_HELP_PATH "/" LV_HELP );
#endif /* MSDOS */
}

public byte *ConfFilename( conf_t *conf )
{
  byte *env;

  if( NULL == conf->file ){
    if( NULL != (env = getenv( "MAN_PN" )) )
      return env;
    else
      return (byte *)"(stdin)";
  } else {
    return *( conf->file );
  }
}
