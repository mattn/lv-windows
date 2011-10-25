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

  $array{ $cnsfrom } = "$big5from $big5to $cnsfrom $cnsto";
}

for $index ( sort keys( %array ) ){
  $code = $array{ $index };
  ( $big5from, $big5to, $cnsfrom, $cnsto ) = split( / /, $code );

#  print "$code\n";

  if( $lastcns ne "" ){
    if( hex( $cnsfrom ) != hex( $lastcns ) + 1 ){
      $newfrom = sprintf( "0x%04x", hex( $lastcns ) + 1 );
      print "  { $newfrom, 0x0000 },\n";
    }
  }

  print "  { $cnsfrom, $big5from },\n";

  if( $cnsto eq "*" ){
    $lastcns = $cnsfrom;
  } else {
    $lastcns = $cnsto;
  }
}

$newfrom = sprintf( "0x%04x", hex( $lastcns ) + 1 );
print "  { $newfrom, 0x0000 }\n";
