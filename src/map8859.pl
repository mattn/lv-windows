#! /usr/local/bin/perl

# 8859-2

$file = "zcat 8859-2.TXT.gz |";

open( FILE, $file ) || die( "cannot open $file" );

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $code, $uni, $rest ) = split;
  if( hex( $code ) >= 0x80 && hex( $uni ) >= 0x100 ){
    $count++;
    $array{ $uni } = sprintf( "0x%04x, ISO8859_2", hex( $code ) & 0x7f );
  }
}

close( FILE );

# 8859-3

$file = "zcat 8859-3.TXT.gz |";

open( FILE, $file ) || die( "cannot open $file" );

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $code, $uni, $rest ) = split;
  if( hex( $code ) >= 0x80 && hex( $uni ) >= 0x100 ){
    if( $array{ $uni } ne "" ){
      print STDERR "Warning: duplicate unicode: $uni\n";
      next;
    }
    $count++;
    $array{ $uni } = sprintf( "0x%04x, ISO8859_3", hex( $code ) & 0x7f );
  }
}

close( FILE );

# 8859-4

$file = "zcat 8859-4.TXT.gz |";

open( FILE, $file ) || die( "cannot open $file" );

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $code, $uni, $rest ) = split;
  if( hex( $code ) >= 0x80 && hex( $uni ) >= 0x100 ){
    if( $array{ $uni } ne "" ){
      print STDERR "Warning: duplicate unicode: $uni\n";
      next;
    }
    $count++;
    $array{ $uni } = sprintf( "0x%04x, ISO8859_4", hex( $code ) & 0x7f );
  }
}

close( FILE );

# 8859-5

$file = "zcat 8859-5.TXT.gz |";

open( FILE, $file ) || die( "cannot open $file" );

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $code, $uni, $rest ) = split;
  if( hex( $code ) >= 0x80 && hex( $uni ) >= 0x100 ){
    if( $array{ $uni } ne "" ){
      print STDERR "Warning: duplicate unicode: $uni\n";
      next;
    }
    $count++;
    $array{ $uni } = sprintf( "0x%04x, ISO8859_5", hex( $code ) & 0x7f );
  }
}

close( FILE );

# 8859-6

$file = "zcat 8859-6.TXT.gz |";

open( FILE, $file ) || die( "cannot open $file" );

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $code, $uni, $rest ) = split;
  if( hex( $code ) >= 0x80 && hex( $uni ) >= 0x100 ){
    if( $array{ $uni } ne "" ){
      print STDERR "Warning: duplicate unicode: $uni\n";
      next;
    }
    $count++;
    $array{ $uni } = sprintf( "0x%04x, ISO8859_6", hex( $code ) & 0x7f );
  }
}

close( FILE );

# 8859-7

$file = "zcat 8859-7.TXT.gz |";

open( FILE, $file ) || die( "cannot open $file" );

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $code, $uni, $rest ) = split;
  if( hex( $code ) >= 0x80 && hex( $uni ) >= 0x100 ){
    if( $array{ $uni } ne "" ){
      print STDERR "Warning: duplicate unicode: $uni\n";
      next;
    }
    $count++;
    $array{ $uni } = sprintf( "0x%04x, ISO8859_7", hex( $code ) & 0x7f );
  }
}

close( FILE );

# 8859-8

$file = "zcat 8859-8.TXT.gz |";

open( FILE, $file ) || die( "cannot open $file" );

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $code, $uni, $rest ) = split;
  if( hex( $code ) >= 0x80 && hex( $uni ) >= 0x100 ){
    if( $array{ $uni } ne "" ){
      print STDERR "Warning: duplicate unicode: $uni\n";
      next;
    }
    $count++;
    $array{ $uni } = sprintf( "0x%04x, ISO8859_8", hex( $code ) & 0x7f );
  }
}

close( FILE );

# 8859-9

$file = "zcat 8859-9.TXT.gz |";

open( FILE, $file ) || die( "cannot open $file" );

while( <FILE> ){
  chop;
  if( /^#/ ){
     next;
   }
  ( $code, $uni, $rest ) = split;
  if( hex( $code ) >= 0x80 && hex( $uni ) >= 0x100 ){
    if( $array{ $uni } ne "" ){
      print STDERR "Warning: duplicate unicode: $uni\n";
      next;
    }
    $count++;
    $array{ $uni } = sprintf( "0x%04x, ISO8859_9", hex( $code ) & 0x7f );
  }
}

close( FILE );

# output

print "private codes_cset_t mapISO8859[ $count ] = {\n";

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
