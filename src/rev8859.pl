#! /usr/local/bin/perl

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

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $code, $uni, $rest ) = split;
  if( hex( $code ) >= 0x80 ){
    $count++;
    $array{ sprintf( "0x%04x", hex( $code ) & 0x7f ) } = $uni;
  }
}

print "private codes_t revISO8859_$set[ $count ] = {\n";

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
