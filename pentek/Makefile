#
# Name		Makefile
# Description	Makefile for pentek
# Author	Akos Polster
# Version	2.0.0
# Revision	$Id: Makefile,v 1.9 1997/07/18 15:32:30 akos Exp $
#

VERSION=2.0.0
DST=/opt/pentek

BINS=pentek
LIBS=pentek.tcl build.tcl darken.tcl file.tcl
PICS=maximize.xpm minimize.xpm pinup.xpm pindown.xpm

test: tclIndex
	sh -c "PENTEKLIB=`pwd` export PENTEKLIB; exec ./pentek"

install: tclIndex
	mkdir -p $(DST)/bin $(DST)/lib $(DST)/doc
	for i in $(BINS); do \
	  ./instver $(VERSION) a+rx,og-w root sys $$i $(DST)/bin/$$i; \
	done
	for i in $(LIBS) $(PICS); do \
	  ./instver $(VERSION) a+r,og-w root sys $$i $(DST)/lib/$$i; \
	done

tclIndex: $(LIBS)
	echo 'auto_mkindex . *.tcl; exit 0' | tclsh8.0

#
# End		Makefile
#
