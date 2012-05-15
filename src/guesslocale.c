/*
 * guesslocale.c
 *
 * All rights reserved. Copyright (C) 2003 by Tomohiro KUBOTA.
 * $Id: guesslocale.c,v 1.7 2004/01/05 08:41:22 nrt Exp $
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

#ifdef HAVE_SETLOCALE

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#if defined(HAVE_LANGINFO_CODESET)
#include <langinfo.h>
#endif

#include <import.h>
#include <ctable.h>
#include <begin.h>
#include <guesslocale.h>

static int strcmp2( char *str1, char *str2 )
{
  while(1) {
    if( *str1 == '_' || *str1 == '-' ) {str1++; continue;}
    if( *str2 == '_' || *str2 == '-' ) {str2++; continue;}
    if( toupper(*str1) != toupper(*str2) ) return -1;
    if( *str1 == 0 ) return 0;
    str1++; str2++;
  }
}

public byte LocaleCodingSystem( char *language )
{
#ifdef MSDOS
  strcpy( language, "ja_JP" );
  return SHIFT_JIS;
#elif !defined(HAVE_LANGINFO_CODESET)
#warning "XXX There isn't nl_langinfo(CODESET) functionality."
#warning "XXX Using fixed value ``ja_JP'' and EUC_JAPAN..."
  strcpy( language, "ja_JP" );
  return EUC_JAPAN;
#else
  static char lang[6];
  char *nl;
  static byte c = 0;

  if( c ) {
    strcpy( language, lang );
    return c;
  }

  nl = setlocale( LC_CTYPE, "" );
  if (nl != NULL) {
    strncpy( lang, nl, 5 ); lang[5] = 0;
  } else
    lang[0] = 0;
  nl = nl_langinfo( CODESET );

  if ( !strcmp2(nl, "UTF-8") ) c = UTF_8;
  else if ( !strcmp2(nl, "ISO-8859-1") ) c = ISO_8859_1;
  else if ( !strcmp2(nl, "ISO-8859-2") ) c = ISO_8859_2;
  else if ( !strcmp2(nl, "ISO-8859-3") ) c = ISO_8859_3;
  else if ( !strcmp2(nl, "ISO-8859-4") ) c = ISO_8859_4;
  else if ( !strcmp2(nl, "ISO-8859-5") ) c = ISO_8859_5;
  else if ( !strcmp2(nl, "ISO-8859-6") ) c = ISO_8859_6;
  else if ( !strcmp2(nl, "ISO-8859-7") ) c = ISO_8859_7;
  else if ( !strcmp2(nl, "ISO-8859-8") ) c = ISO_8859_8;
  else if ( !strcmp2(nl, "ISO-8859-9") ) c = ISO_8859_9;
  else if ( !strcmp2(nl, "ISO-8859-10") ) c = ISO_8859_10;
  else if ( !strcmp2(nl, "ISO-8859-11") ) c = ISO_8859_11;
  else if ( !strcmp2(nl, "TIS-620") ) c = ISO_8859_11;
  else if ( !strcmp2(nl, "ISO-8859-13") ) c = ISO_8859_13;
  else if ( !strcmp2(nl, "ISO-8859-14") ) c = ISO_8859_14;
  else if ( !strcmp2(nl, "ISO-8859-15") ) c = ISO_8859_15;
  else if ( !strcmp2(nl, "EUC-JP") ) c = EUC_JAPAN;
  else if ( !strcmp2(nl, "GB2312") ) c = EUC_CHINA;
  else if ( !strcmp2(nl, "EUC-CN") ) c = EUC_CHINA;
  else if ( !strcmp2(nl, "EUC-KR") ) c = EUC_KOREA;
  else if ( !strcmp2(nl, "EUC-TW") ) c = EUC_TAIWAN;
  else if ( !strcmp2(nl, "BIG5") ) c = BIG_FIVE;
  else if ( !strcmp2(nl, "SHIFT_JIS") ) c = SHIFT_JIS;
  else if ( !strcmp2(nl, "SJIS") ) c = SHIFT_JIS;
  else c = ISO_8859_1;

  strcpy( language, lang );
  return c;
#endif
}

public byte DetermineEUC( char *language, char defaultEuc )
{
  if( EUC_JAPAN == defaultEuc || EUC_KOREA == defaultEuc ||
      EUC_CHINA == defaultEuc || EUC_TAIWAN == defaultEuc )
    return defaultEuc;
  if( !strncmp(language, "ja", 2) ) return EUC_JAPAN;
  if( !strncmp(language, "ko", 2) ) return EUC_KOREA;
  if( !strcmp(language, "zh_CN") ) return EUC_CHINA;
  if( !strcmp(language, "zh_TW") ) return EUC_TAIWAN;
  if( !strcmp(language, "zh_HK") ) return EUC_TAIWAN;
  if( !strncmp(language, "zh", 2) ) return EUC_TAIWAN;
  return EUC_JAPAN;
}

public byte Determine8bit( char *language )
{
  char country[3], language2[3], *cp;

  cp = strchr(language, '_');
  if (cp != NULL) {
    strncpy(country, cp + 1, 2);
    country[2] = 0;
  } else
    country[0] = 0;
  strncpy( language2, language, 2 );
  language2[2] = 0;

  /* Euro countries, maybe increased in future */
  if( !strcmp(country, "AT") ) return ISO_8859_15;
  if( !strcmp(country, "BE") ) return ISO_8859_15;
  if( !strcmp(country, "DE") ) return ISO_8859_15;
  if( !strcmp(country, "ES") ) return ISO_8859_15;
  if( !strcmp(country, "FI") ) return ISO_8859_15;
  if( !strcmp(country, "FR") ) return ISO_8859_15;
  if( !strcmp(country, "GR") ) return ISO_8859_7;
  if( !strcmp(country, "IE") ) return ISO_8859_15;
  if( !strcmp(country, "IT") ) return ISO_8859_15;
  if( !strcmp(country, "LU") ) return ISO_8859_15;
  if( !strcmp(country, "NL") ) return ISO_8859_15;
  if( !strcmp(country, "PT") ) return ISO_8859_15;

  if( !strcmp(language2, "ar") ) return ISO_8859_6;
  if( !strcmp(language2, "bs") ) return ISO_8859_2;
  if( !strcmp(language2, "cs") ) return ISO_8859_2;
  if( !strcmp(language2, "cy") ) return ISO_8859_14;
  if( !strcmp(language2, "fa") ) return ISO_8859_8;
  if( !strcmp(language2, "hr") ) return ISO_8859_2;
  if( !strcmp(language2, "hu") ) return ISO_8859_2;
  if( !strcmp(language2, "iw") ) return ISO_8859_8;
  if( !strcmp(language2, "lt") ) return ISO_8859_13;
  if( !strcmp(language2, "lv") ) return ISO_8859_13;
  if( !strcmp(language2, "mi") ) return ISO_8859_13;
  if( !strcmp(language2, "mk") ) return ISO_8859_5;
  if( !strcmp(language2, "mt") ) return ISO_8859_3;
  if( !strcmp(language2, "pl") ) return ISO_8859_2;
  if( !strcmp(language2, "pl") ) return ISO_8859_2;
  if( !strcmp(language2, "ro") ) return ISO_8859_2;
  if( !strcmp(language2, "ru") ) return ISO_8859_5; /* should be KOI8-R */
  if( !strcmp(language2, "sk") ) return ISO_8859_2;
  if( !strcmp(language2, "sl") ) return ISO_8859_2;
  if( !strcmp(language2, "sr") ) return ISO_8859_2; /* ISO-8859-5? */
  if( !strcmp(language2, "th") ) return ISO_8859_11;
  if( !strcmp(language2, "tr") ) return ISO_8859_9;

  return ISO_8859_1;
}

#endif /* HAVE_SETLOCALE */
