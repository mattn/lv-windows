#! /bin/csh
foreach file ( *.c *.h lv.hlp Makefile* makefile* )
	d2u.pl $file > /tmp/lvtmp
	cat /tmp/lvtmp > $file
end
