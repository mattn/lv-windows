#! /usr/local/bin/perl

# All rights reserved. Copyright (C) 1999 by NARITA Tomio.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# Unicode to GB2312

$file = "zcat GB2312.TXT.gz |";

for $i ( 0 .. $#ARGV ){
  if( $ARGV[ $i ] eq "-i" ){
    $file = $ARGV[ ++$i ];
  }
}

open( FILE, $file ) || die( "cannot open $file" );

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $gb, $uni, $rest ) = split;

  if( $array{ $gb } ne "" ){
    print STDERR "Warning: duplicate character, gb:$gb\n";
    next;
  }

  $count++;
  $array{ $gb } = $uni;
}

print "private codes_t revGB2312[ $count ] = {\n";

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
