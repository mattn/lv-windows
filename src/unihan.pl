#! /usr/local/bin/perl

# Unicode to others

$file = "zcat UNIHAN.TXT.gz |";

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

  ( $unistr, $class, $rest ) = split;

  $uni = substr( $unistr, 2, 4 );

#  if( $class eq "kBigFive" ){
#    $code = $rest;
#    $big5map{ $uni } = $code;
#    $big5rev{ $code } = $uni;
#    $big5Count++;
#  } elsif( $class eq "kGB0" ){
#    $ku  = substr( $rest, 0, 2 );
#    $ten = substr( $rest, 2, 2 );
#    $code = sprintf( "%04x", ( ( $ku + 0x20 ) << 8 ) | ( $ten + 0x20 ) );
#    $gbmap{ $uni } = $code;
#    $gbrev{ $code } = $uni;
#    $gbCount++;
#  } elsif( $class eq "kJis0" ){
  if( $class eq "kJis0" ){
    $ku  = substr( $rest, 0, 2 );
    $ten = substr( $rest, 2, 2 );
    $code = sprintf( "%04x", ( ( $ku + 0x20 ) << 8 ) | ( $ten + 0x20 ) );
    $jismap{ $uni } = "$code, X0208";
    $jismapCount++;
    $jis0208rev{ $code } = $uni;
    $jis0208revCount++;
  } elsif( $class eq "kJis1" ){
    $ku  = substr( $rest, 0, 2 );
    $ten = substr( $rest, 2, 2 );
    $code = sprintf( "%04x", ( ( $ku + 0x20 ) << 8 ) | ( $ten + 0x20 ) );
    $jismap{ $uni } = "$code, X0212";
    $jis0212rev{ $code } = $uni;
    $jis0212revCount++;
  }
#  } elsif( $class eq "kKSC0" ){
#    $ku  = substr( $rest, 0, 2 );
#    $ten = substr( $rest, 2, 2 );
#    $code = sprintf( "%04x", ( ( $ku + 0x20 ) << 8 ) | ( $ten + 0x20 ) );
#    $kscmap{ $uni } = $code;
#    $kscrev{ $code } = $uni;
#    $kscCount++;
#  }
}

# jis.map

print "private codes_set_t mapJIS[ $jisCount ] = {\n";

$count = $jisCount;
for $index ( sort keys( %jismap ) ){
  $code = $jismap{ $index };
  $count--;
  if( $count == 0 ){
    print "  { 0x$index, 0x$code }\n";
  } else {
    print "  { 0x$index, 0x$code },\n";
  }
}

print "};\n";

# jis0208.rev

print "private codes_t revJIS0208[ $jis0208revCount ] = {\n";

$count = $jis0208revCount;
for $index ( sort keys( %jis0208rev ) ){
  $code = $jis0208rev{ $index };
  $count--;
  if( $count == 0 ){
    print "  { 0x$index, 0x$code }\n";
  } else {
    print "  { 0x$index, 0x$code },\n";
  }
}

print "};\n";

# jis0212.rev

print "private codes_t revJIS0212[ $jis0212revCount ] = {\n";

$count = $jis0212revCount;
for $index ( sort keys( %jis0212rev ) ){
  $code = $jis0212rev{ $index };
  $count--;
  if( $count == 0 ){
    print "  { 0x$index, 0x$code }\n";
  } else {
    print "  { 0x$index, 0x$code },\n";
  }
}

print "};\n";
