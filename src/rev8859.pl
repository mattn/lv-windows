#! /usr/local/bin/perl

# All rights reserved. Copyright (C) 1999 by NARITA Tomio.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# ISO 8859-* to Unicode

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

for( $index = 0x20 ; $index < 0x80 ; $index++ ){
  $array{$index} = "0x0000";
}

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $code, $uni, $rest ) = split;
  if( hex( $code ) >= 0x80 ){
    $count++;
#    $array{ sprintf( "0x%02x", hex( $code ) & 0x7f ) } = $uni;
    $array{ hex( $code ) & 0x7f } = $uni;
  }
}

close( FILE );

#print "private codes_t revISO8859_${set}[ $count ] = {\n";
print "private ic_t revISO8859_${set}[ 96 ] = {\n";

for( $index = 0x20 ; $index < 0x80 ; $index++ ){
  if( 0x7f == $index ){
    print "  $array{$index}\n";
  } else {
    print "  $array{$index},\n";
  }
}

#for $index ( sort keys( %array ) ){
#  $code = $array{ $index };
#  $count--;
#  if( $count == 0 ){
#    print "  { $index, $code }\n";
#  } else {
#    print "  { $index, $code },\n";
#  }
#}

print "};\n";
