#! /usr/local/bin/perl

# Big five to Unicode 

$file = "zcat BIG5.TXT.gz |";

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
  ( $big5, $uni, $rest ) = split;
  if( $uni ne "0xFFFD" ){
    $count++;
    $array{ $big5 } = $uni;
  }
}

print "private codes_t revBIG5[ $count ] = {\n";

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
