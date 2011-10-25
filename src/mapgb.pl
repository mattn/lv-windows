#! /usr/local/bin/perl

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

  if( $array{ $uni } ne "" ){
    print STDERR "Warning: duplicate character, uni:$uni\n";
    next;
  }

  $count++;
  $array{ $uni } = $gb;
}

print "private codes_t mapGB2312[ $count ] = {\n";

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
