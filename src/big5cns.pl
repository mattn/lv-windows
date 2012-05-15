#! /usr/local/bin/perl

# All rights reserved. Copyright (C) 1999 by NARITA Tomio.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# Making conversion tables between big five and CNS 11643-1992
# from CJK.INF Version 2.1 (July 12, 1996) written by Ken Lunde.

# This tables are to be imported into big5.c
#
# Each tupple, "{ big5, CNS }" or "{ CNS, big5 }", represents
# correspondence between the two character sets. The left side
# of tupples are used as binary search index, so they must be sorted
# in ascending order. Successive indexes construct a range.
# The range spans from one index to (the next index - 1).
# For example, the following indexes:
#  { 0xF9C7, 0x7235 },
#  { 0xF9D2, 0x7241 },
#  { 0xf9d6, 0x0000 }
# means 0xF9C7 to 0xF9D1 (0xF9D2-1) of Big5 to be mapped into CNS 11643
# code space starting from 0x7235. If the code-point of CNS side is 0x0000,
# it is a sign that there is no mapping character. This void mapping
# is always placed at the last to terminate each mapping table.

$file = "zcat cjk.inf.gz |";

for $i ( 0 .. $#ARGV ){
  if( $ARGV[ $i ] eq "-i" ){
    $file = $ARGV[ ++$i ];
  }
}

open( FILE, $file ) || die( "cannot open $file" );

MakeTable( "Big Five Level 1 Correspondence to CNS 11643-1992 Plane 1",
	  "big5Level1ToCnsPlane1",
	  "cnsPlane1ToBig5Level1" );

MakeTable( "Big Five Level 2 Correspondences to CNS 11643-1992 Plane 2",
	  "big5Level2ToCnsPlane2",
	  "cnsPlane2ToBig5Level2" );

exit 0;

sub MakeTable {
  local( $tag, $arrayNameBig5, $arrayNameCNS ) = @_;

  $found = 0;
  %Big5toCNS = ( );
  %CNStoBig5 = ( );

  while( <FILE> ){
    if( /$tag/ ){
      $found = 1;
      last;
    }
  }

  if( 1 == $found ){
    while( <FILE> ){
# Lunde says:
#        It turns out that corrections were made to Big Five (at least
# in the ETen and Microsoft implementations thereof) which made it a bit
# closer to CNS 11643-1992 as far as character ordering is concerned.
# The following six lines of code correspondences:
      if( /0xCAF8-0xD6CB <-> 0x2439-0x376E/ ){
	$_ = "  0xCAF8-0xD779 <-> 0x2439-0x387D";
      } elsif( /0xD77B-0xDADE <-> 0x387E-0x3E62/ ){
	$_ = "  0xD77B-0xDBA6 <-> 0x387E-0x3F69";
      } elsif( /0xD6CC <-> 0x3E63/
	      || /0xD6CD-0xD779 <-> 0x3770-0x387D/
	      || /0xDADF <-> 0x376F/
	      || /0xDAE0-0xDBA6 <-> 0x3E64-0x3F69/ ){
	next;
      }

      $big5from = substr( $_, 2, 6 );
      $big5to   = substr( $_, 9, 6 );
      $cnsfrom  = substr( $_, 20, 6 );
      if( '-' eq substr( $_, 26, 1 ) ){
	$cnsto    = substr( $_, 27, 6 );
	$range = 1;
      } else {
	$cnsto    = substr( $_, 28, 6 );
	$range = 0;
      }
      next if( '' eq $big5to || '' eq $cnsfrom );

      if( 0 == $range ){
	if( "      " ne $big5from ){
	  $Big5toCNS{ $big5from } = "$big5from $big5from $cnsfrom $cnsfrom";
	  $CNStoBig5{ $cnsfrom } = "$cnsfrom $cnsfrom $big5from $big5from";

	  $big5from = $big5to;
	  $cnsfrom = $cnsto;
	}
      }

      $big5from = $big5to if "" eq $big5from || "      " eq $big5from;
      $cnsto = $cnsfrom if "" eq $cnsto || "      " eq $cnsto;

      $Big5toCNS{ $big5from } = "$big5from $big5to $cnsfrom $cnsto";
      $CNStoBig5{ $cnsfrom } = "$cnsfrom $cnsto $big5from $big5to";

      last if /Level (1|2) Hanzi END/;
    }
  }

# Reading done.

# Make Big5 to CNS table

  $count = 0;
  $lastCode = "";

  foreach $index ( sort keys( %Big5toCNS ) ){
    $code = $Big5toCNS{ $index };
    ( $big5from, $big5to, $cnsfrom, $cnsto ) = split( / /, $code );

    if( "" ne $lastCode ){
      if( hex( $big5from ) != hex( $lastCode ) + 1 ){
	$newfrom = sprintf( "0x%04x", hex( $lastCode ) + 1 );
	$count++;
      }
    }

    $count++;

    if( $big5to eq "*" ){
      $lastCode = $big5from;
    } else {
      $lastCode = $big5to;
    }
  }

  $count++;

  print "private codes_t ${arrayNameBig5}[ $count ] = {	/* range */\n";

  $lastCode = "";

  foreach $index ( sort keys( %Big5toCNS ) ){
    $code = $Big5toCNS{ $index };
    ( $big5from, $big5to, $cnsfrom, $cnsto ) = split( / /, $code );

    if( "" ne $lastCode ){
      if( hex( $big5from ) != hex( $lastCode ) + 1 ){
	$newfrom = sprintf( "0x%04x", hex( $lastCode ) + 1 );
	print "  { $newfrom, 0x0000 },\n";
      }
    }

    print "  { $big5from, $cnsfrom },\n";

    if( $big5to eq "*" ){
      $lastCode = $big5from;
    } else {
      $lastCode = $big5to;
    }
  }

  $newfrom = sprintf( "0x%04x", hex( $lastCode ) + 1 );
  print "  { $newfrom, 0x0000 }\n";

  print "};\n\n";

# Make CNS to Big5 table

  $count = 0;
  $lastCode = "";

  foreach $index ( sort keys( %CNStoBig5 ) ){
    $code = $CNStoBig5{ $index };
    ( $cnsfrom, $cnsto, $big5from, $big5to ) = split( / /, $code );

    if( "" ne $lastCode ){
      if( hex( $cnsfrom ) != hex( $lastCode ) + 1 ){
	$newfrom = sprintf( "0x%04x", hex( $lastCode ) + 1 );
	$count++;
      }
    }

    $count++;

    if( $cnsto eq "*" ){
      $lastCode = $cnsfrom;
    } else {
      $lastCode = $cnsto;
    }
  }

  $count++;

  print "private codes_t ${arrayNameCNS}[ $count ] = {	/* range */\n";

  $lastCode = "";

  foreach $index ( sort keys( %CNStoBig5 ) ){
    $code = $CNStoBig5{ $index };
    ( $cnsfrom, $cnsto, $big5from, $big5to ) = split( / /, $code );

    if( "" ne $lastCode ){
      if( hex( $cnsfrom ) != hex( $lastCode ) + 1 ){
	$newfrom = sprintf( "0x%04x", hex( $lastCode ) + 1 );
	print "  { $newfrom, 0x0000 },\n";
      }
    }

    print "  { $cnsfrom, $big5from },\n";

    if( $cnsto eq "*" ){
      $lastCode = $cnsfrom;
    } else {
      $lastCode = $cnsto;
    }
  }

  $newfrom = sprintf( "0x%04x", hex( $lastCode ) + 1 );
  print "  { $newfrom, 0x0000 }\n";

  print "};\n\n";
}
