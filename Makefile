########################################################################
# Makefile for lv
#   target: all, dos, clean, dosclean, install, uninstall, dist, dosdist
########################################################################

VERSION = 421

# If your ``make'' command does not support ``include'' directive,
# please try to change ``MAKE'' to another ``make'', for example,
# gmake or /usr/local/bin/make, etc.
MAKE = make

bindir = /usr/local/bin
libdir = /usr/local/lib/lv

distdir = lv$(VERSION)
SHELL = /bin/sh

#
# Building lv
#

all:
	cd src ; $(MAKE)
	if [ ! -h lv ]; then \
		if [ -f lv ]; then rm lv ; fi ; \
		ln -s src/lv lv ; \
	fi

dos:
	cd src
	$(MAKE) -f Makefile.dos dos
	mv lv.exe ..
	cd ..

#
# Cleaning
#

clean:
	if [ -f lv ]; then rm lv ; fi
	cd src ; $(MAKE) clean

dosclean:
	cd src
	$(MAKE) -f Makefile.dos dosclean
	cd ..
	rm lv.exe

#
# Installation
#

install: all
	cp lv $(bindir)
	if [ ! -d $(libdir) ]; then mkdir $(libdir) ; fi
	cp lv.hlp $(libdir)

uninstall:
	if [ -f $(bindir)/lv ]; then rm $(bindir)/lv ; fi
	if [ -f $(libdir)/lv.hlp ]; then rm $(libdir)/lv.hlp ; fi


#
# You SHOULD NOT distribute the following:
# (src archive for MSDOS make)
#

lvsrc:
	if [ -f lvsrc.lzh ]; then rm lvsrc.lzh ; fi
	lha a lvsrc.lzh \
		Makefile CONFIG \
		README README.ja \
		lv.hlp help.ja \
		src/Makefile.dos \
		src/*.h src/*.c

#
# Distribution file
#

dist:
	if [ -f $(distdir).tar.gz ]; then rm $(distdir).tar.gz ; fi
	cd .. ; \
	tar cvf $(distdir)/$(distdir).tar \
		$(distdir)/Makefile $(distdir)/CONFIG \
		$(distdir)/README $(distdir)/README.ja \
		$(distdir)/lv.hlp $(distdir)/help.ja \
		$(distdir)/lv.1 \
		$(distdir)/src/Makefile $(distdir)/src/Makefile.dos \
		$(distdir)/src/*.h $(distdir)/src/*.c \
		$(distdir)/src/*.map $(distdir)/src/*.rev \
		$(distdir)/src/*.pl $(distdir)/src/*.sh
	gzip $(distdir).tar

dosdist:
	if [ -f $(distdir).lzh ]; then rm $(distdir).lzh ; fi
	cd .. ; \
	lha a $(distdir)/$(distdir).lzh \
		$(distdir)/README $(distdir)/README.ja \
		$(distdir)/lv.hlp $(distdir)/help.ja \
		$(distdir)/lv.exe
