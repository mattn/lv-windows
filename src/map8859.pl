#! /usr/local/bin/perl

# All rights reserved. Copyright (C) 1999 by NARITA Tomio.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# 8859-2

$file = "zcat 8859-2.TXT.gz |";
$set = "2";

for $i ( 0 .. $#ARGV ){
  if( $ARGV[ $i ] eq "-i" ){
    $file = $ARGV[ ++$i ];
  } elsif( $ARGV[ $i ] eq "-s" ){
    $set = $ARGV[ ++$i ];
  }
}

open( FILE, $file ) || die( "cannot open $file" );

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $code, $uni, $rest ) = split;
  if( hex( $code ) >= 0x80  ){
    $count++;
    $array{ $uni } = sprintf( "0x%02x", hex( $code ) & 0x7f );
  }
}

close( FILE );

# output

print "private codes_t mapISO8859_${set}[ $count ] = {\n";

for $index ( sort keys( %array ) ){
  $code = $array{ $index };
  $count--;
  if( $count == 0 ){
    print "  { $index, $code }\n";
  } else {
    print "  { $index, $code },\n";
  }
}

print "};\n";
