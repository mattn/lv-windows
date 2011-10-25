/*
 * file.h
 *
 * All rights reserved. Copyright (C) 1994,1997 by NARITA Tomio
 */

#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>

#include <itable.h>
#include <ctable.h>
#include <str.h>

#define LINE_SIZE	64U		/* physical lines per logical line */
#define PAGE_SIZE	32U		/* lines per page */

#ifdef MSDOS
#define BLOCK_SIZE	2		/* segments on memory */
#define SLOT_SIZE	2047U		/* file location pointers */
#else
#define BLOCK_SIZE	4		/* segments on memory */
#define SLOT_SIZE	16384U		/* file location pointers */
#endif /* MSDOS */

typedef struct {
  int		ptr;
  int		width;
  boolean_t	hit;
} head_t;

typedef struct {
  i_str_t	*istr;
  int		heads;			/* physical line number */
  head_t	*head;
} line_t;

typedef struct {
  unsigned int	segment;
  int		lines;			/* logical line number */
  line_t	line[ PAGE_SIZE ];
} page_t;

typedef struct {
  unsigned int	seg;
  int		blk;
  int		off;
  int		phy;
} position_t;

typedef struct {
  position_t	top;
  position_t	bot;
  int		lines;
} screen_t;

typedef struct {
  position_t	pos;
  i_str_t	*pattern;
  boolean_t	first;
  boolean_t	displayed;
} find_t;

typedef struct {
/*
  state_t	state;
*/
  long		ptr;
} slot_t;

typedef struct {
  char		*fileName;
  FILE		*fp;
  FILE		*sp;
  int		inputCodingSystem;
  int		outputCodingSystem;
  int		keyboardCodingSystem;
  int		width;
  int		height;
  unsigned int	lastSegment;
  unsigned int	totalLines;
  long		lastPtr;
  boolean_t	done;
  boolean_t	eof;
  boolean_t	top;
  boolean_t	bottom;
  boolean_t	truncated;
  boolean_t	dirty;
  find_t	find;
  screen_t	screen;
  boolean_t	used[ BLOCK_SIZE ];
  page_t	page[ BLOCK_SIZE ];
  slot_t	slot[ SLOT_SIZE ];
} file_t;

#ifdef MSDOS
#define line_t line_t far
#define page_t page_t far
#define file_t file_t far
#else
#ifndef far
#define far
#endif /* far */
#endif /* MSDOS */

#define Segment( line )		( (line) / PAGE_SIZE )
#define Offset( line )		( (line) % PAGE_SIZE )
#define Block( segment )	( (segment) % BLOCK_SIZE )

public boolean_t immediate_print;

public char load_str[ STR_SIZE ];

public boolean_t FileLoadLine( file_t *f, int *length );

public file_t *FileOpen( char *fileName, FILE *fp, FILE *sp,
			int width, int height,
			char inputCodingSystem,
			char outputCodingSystem,
			char keyboardCodingSystem );

public boolean_t FileClose( file_t *f );
public boolean_t FileStretch( file_t *f, unsigned int target );
public boolean_t FileSeek( file_t *f, unsigned int segment );

public void FileFreeHead( file_t *f );
public void FileRefresh( file_t *f );
public char *FileStatus( file_t *f );

#endif /* __FILE_H__ */
