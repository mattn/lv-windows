#! /usr/local/bin/perl

# Unicode to Big five

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
    if( $array{ $uni } ne "" ){
      print STDERR "Warning: duplicate unicode: $uni\n";
      next;
    }

    $count++;
    $array{ $uni } = $big5;
  }
}

print "private codes_t mapBIG5[ $count ] = {\n";

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
