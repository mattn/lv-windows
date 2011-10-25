#! /usr/local/bin/perl

# Unicode to JIS X 0208

$file = "zcat JIS0208.TXT.gz |";

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
  ( $sjis, $jis, $uni, $rest ) = split;

  $count++;
  $array{ $jis } = $uni;
}

close( FILE );

# Output

print "private codes_t revJIS0208[ $count ] = {\n";

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
