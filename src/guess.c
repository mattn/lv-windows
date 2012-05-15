/*
 * guess.c
 *
 * All rights reserved. Copyright (C) 1996 by NARITA Tomio.
 * $Id: guess.c,v 1.8 2003/11/13 03:08:19 nrt Exp $
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

#include <import.h>
#include <decode.h>
#include <big5.h>
#include <utf.h>
#ifndef MSDOS /* IF NOT DEFINED */
#include <unimap.h>
#include <unirev.h>
#endif /* MSDOS */
#ifdef HAVE_SETLOCALE
#include <locale.h>
#include <guesslocale.h>
#endif
#include <begin.h>
#include <guess.h>

#ifndef MSDOS
static int isUTF8( byte *str, int length )
{
  int i, mode = 0, m2;
  unsigned long ucs;
  byte ch;

  for( i = 0 ; i < length ; i++ ){
    ch = str[ i ];
    if( mode == 0 ){
      if( (ch&0x80) == 0 ) continue;
      if( (ch&0xe0) == 0xc0 ){ mode = m2 = 1; ucs = (ch&0x1f); continue; }
      if( (ch&0xf0) == 0xe0 ){ mode = m2 = 2; ucs = (ch&0x0f); continue; }
      if( (ch&0xf8) == 0xf0 ){ mode = m2 = 3; ucs = (ch&0x07); continue; }
      if( (ch&0xfc) == 0xf8 ){ mode = m2 = 4; ucs = (ch&0x03); continue; }
      if( (ch&0xfe) == 0xfc ){ mode = m2 = 5; ucs = (ch&0x01); continue; }
      return 0;
    } else {
      if( (ch&0xc0) != 0x80 ) return 0;
      ucs <<= 6; ucs += (ch&0x3f);
      mode--;
      if( !mode ){
	if( m2 == 1 && ucs < 0x0000080 ) return 0;
	if( m2 == 2 && ucs < 0x0000800 ) return 0;
	if( m2 == 3 && ucs < 0x0010000 ) return 0;
	if( m2 == 4 && ucs < 0x0200000 ) return 0;
	if( m2 == 5 && ucs < 0x4000000 ) return 0;
      }
    }
  }
  return 1;
}
#endif /* MSDOS */

private byte GuessCodingSystem_EastAsia( byte *str, int length,
					 byte defaultEuc, char *language )
{
  int i;
  byte ch;
  int big5Penalty = 0;
  int sjisPenalty = 0;
  int eucjapanPenalty = 0;
  int eucPenalty = 0;

  for( i = 0 ; i < length ; i++ ){
    /*
     * check for simple euc
     */
    ch = str[ i ];
    if( 0x80 & ch ){
      if( SS2 == ch || SS3 == ch ){
	eucPenalty++;
	break;
      }
      if( !IsEucByte( ch ) ) {
	eucPenalty++;
	break;
      }
      if( ++i >= length )
	break;	
      ch = str[ i ];
      if( !IsEucByte( ch ) ) {
	eucPenalty++;
	break;
      }
    }
  }
  if( 0 == eucPenalty ) {
    if (language)
      return DetermineEUC(language, defaultEuc);
    else if ( AUTOSELECT != defaultEuc )
      return defaultEuc;
  }

  for( i = 0 ; i < length ; i++ ){
    /*
     * check for euc-japan or euc-taiwan
     */
    ch = str[ i ];
    if( 0x80 & ch ){
      if( SS2 == ch ){
	if( ++i >= length )
	  break;
	ch = str[ i ];
	if( !IsKatakanaByte( ch ) ) {
	  eucjapanPenalty++;
	  break;
	}
	continue;
      }
      if( SS3 == ch ){
	if( ++i >= length )
	  break;
	ch = str[ i ];
      }
      if( !IsEucByte( ch ) ) {
	eucjapanPenalty++;
	break;
      }
      if( ++i >= length )
	break;
      ch = str[ i ];
      if( !IsEucByte( ch ) ) {
	eucjapanPenalty++;
	break;
      }
    }
  }
  if( 0 == eucjapanPenalty ) {
    if (language) {
      if( !strncmp(language, "ja", 2) )
	return EUC_JAPAN;
      else
	return EUC_TAIWAN;
    } else {
      return EUC_JAPAN; /* XXX */
    }
  }

  for( i = 0 ; i < length ; i++ ){
    /*
     * check for big5
     */
    ch = str[ i ];
    if( 0x80 & ch ){
      if( !IsBig5Byte1( ch ) ) {
	big5Penalty++;
	break;
      }
      ch = str[ ++i ];
      if( i >= length )
	break;
      if( !IsBig5Byte2( ch ) ) {
	big5Penalty++;
	break;
      }
    }
  }
  if( 0 == big5Penalty )
    return BIG_FIVE;

  for( i = 0 ; i < length ; i++ ){
    /*
     * check for shift-jis
     */
    ch = str[ i ];
    if( 0x80 & ch ){
      if( IsKatakanaByte( ch ) )
	continue;
      if( !IsShiftJisByte1( ch ) ) {
	sjisPenalty++;
	break;
      }
      ch = str[ ++i ];
      if( i >= length )
	break;
      if( !IsShiftJisByte2( ch ) ) {
	sjisPenalty++;
	break;
      }
    }
  }
  if( 0 == sjisPenalty )
    return SHIFT_JIS;

  return ISO_8859_1;
}

public byte GuessCodingSystem( byte *str, int length, byte defaultEuc )
{
  int i;
  byte ch;
#ifdef HAVE_SETLOCALE
  byte c;
  char *nl, language[6];
  int use_locale = 1;

  nl = setlocale( LC_CTYPE, "" );
  if (NULL == nl || !strcmp("C", nl))
    use_locale = 0;

  if (use_locale) {
#ifndef MSDOS /* IF NOT DEFINED */
    /*
     * Since UTF-8 is a strict coding system, it is unlikely that
     * a non-UTF-8 file is accidently recognized as a UTF-8 file.
     * Thus, UTF-8 test is performed first regardless of locale.
     */
    if( 1 == isUTF8( str, length ) )
      return UTF_8;
#endif /* MSDOS */
    /*
     * Now, the file is not UTF-8.  In this case, separate algorithms
     * are used for east Asian locales (where multibyte coding systems
     * are expected to be used and further guessing may be possible) and
     * other locales (where 8bit coding systems are used and further
     * guessing is almost impossible).
     */
    c = LocaleCodingSystem(language);

    if( !strncmp(language, "ja", 2) || !strncmp(language, "ko", 2) ||
	!strncmp(language, "zh", 2) ) {
      /*
       * In case of east Asian locales.  If the user has a certain
       * preference on EUC coding system, the preference is used.
       * If the file is EUC-compliant, coding system is determined
       * by locale.
       */
      return GuessCodingSystem_EastAsia(str, length, defaultEuc, language);
    }

    /*
     * In case of other than east Asian locales. (not multibyte)
     */
    if( UTF_8 != defaultEuc && AUTOSELECT != defaultEuc )
      /*
       * If the user has a certain preference on coding system,
       * the will is repected.  However, it is already known that
       * the file is not UTF-8.
       */
      return defaultEuc;

    if( UTF_8 == c ) {
      /*
       * When the environment is UTF-8 locale while the file is
       * not UTF-8, coding system is guessed from language/country
       * part of the current locale.
       */
      return Determine8bit( language );
    }

    /*
     * When the environment is not UTF-8 locale, coding system is
     * determined by the current locale.
     */
    return c;
  } else
#endif /* HAVE_SETLOCALE */
  {
#ifndef MSDOS /* IF NOT DEFINED */
    /*
     * Since UTF-8 is a strict coding system, it is unlikely that
     * a non-UTF-8 file is accidently recognized as a UTF-8 file.
     * Thus, UTF-8 test is performed first regardless of locale.
     */
    if( 1 == isUTF8( str, length ) )
      return UTF_8;
#endif /* MSDOS */

    return GuessCodingSystem_EastAsia(str, length, defaultEuc, NULL);
  }
}

public byte GuessHz( byte *str, int length )
{
  boolean_t gb = FALSE;
  int i, hzPenalty = 0;
  byte ch;

  for( i = 0 ; i < length ; i++ ){
    /*
     * check for HZ
     */
    ch = str[ i ];
    if( '~' == ch ){
      if( ++i >= length )
	break;
      ch = str[ i ];
      if( FALSE == gb ){
	if( '{' == ch ){
	  gb = TRUE;
	} else if( '~' == ch || '}' == ch ){
	  /* do nothing */
	} else
	  hzPenalty++;
      } else {
	if( '}' == ch )
	  gb = FALSE;
      }
    }
  }

  return 0 == hzPenalty;
}


public void AdjustPatternCharset( byte inputCodingSystem,
				 byte keyboardCodingSystem,
				 byte defaultCodingSystem,
				 i_str_t *istr )
{
  if( FALSE == adjust_charset )
    return;

#ifndef MSDOS /* IF NOT DEFINED */
  if( IsUtfEncoding( inputCodingSystem ) || IsUtfEncoding( keyboardCodingSystem ) ){
    if( IsUtfEncoding( inputCodingSystem ) && !IsUtfEncoding( keyboardCodingSystem ) )
      ConvertToUNI( istr );
    else if( !IsUtfEncoding( inputCodingSystem ) && IsUtfEncoding( keyboardCodingSystem ) ){
      if( AUTOSELECT == inputCodingSystem )
	inputCodingSystem = defaultCodingSystem;
      ConvertFromUNI( istr, inputCodingSystem );
    }
  } else
#endif /* MSDOS */
    if( BIG_FIVE == inputCodingSystem && BIG_FIVE != keyboardCodingSystem ){
      ConvertCNStoBIG5( istr );
    } else if( BIG_FIVE != inputCodingSystem && BIG_FIVE == keyboardCodingSystem ){
      ConvertBIG5toCNS( istr );
    }
}
