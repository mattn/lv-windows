#! /usr/local/bin/perl

# Big five to CNS table

$file = "zcat b1cns1.dat.gz |";

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
  ( $big5from, $big5to, $cnsfrom, $cnsto, $rest ) = split;
  if( $big5to eq "*" ){
    $big5to = $big5from;
  }
  if( $cnsto eq "*" ){
    $cnsto = $cnsfrom;
  }

  $count++;
  $array{ $big5from } = "$big5from $big5to $cnsfrom $cnsto";
}

#print "private codes_t [ $count ] = {\n";

for $index ( sort keys( %array ) ){
  $code = $array{ $index };
  ( $big5from, $big5to, $cnsfrom, $cnsto ) = split( / /, $code );

  if( $lastbig5 ne "" ){
    if( hex( $big5from ) != hex( $lastbig5 ) + 1 ){
      $newfrom = sprintf( "0x%04x", hex( $lastbig5 ) + 1 );
      print "  { $newfrom, 0x0000 },\n";
    }
  }

  $count--;
  if( $count == 0 ){
    print "  { $big5from, $cnsfrom },\n";
  } else {
    print "  { $big5from, $cnsfrom },\n";
  }

  if( $big5to eq "*" ){
    $lastbig5 = $big5from;
  } else {
    $lastbig5 = $big5to;
  }
}

$newfrom = sprintf( "0x%04x", hex( $lastbig5 ) + 1 );
print "  { $newfrom, 0x0000 }\n";

#print "};\n";
