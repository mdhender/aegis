#!/bin/sh
#
#	aegis - a project change supervisor
#	Copyright (C) 1990, 1991, 1992, 1993, 1994 Peter Miller.
#	All rights reserved.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# MANIFEST: shell script to generate Makefile fragment for each source file
#
case $# in
2)
	;;
*)
	echo "usage: $0 filename resolved-filename" 1>&2
	exit 1
	;;
esac
file="$1"
rfn="$2"

case $file in

*/*.y)
	root=`basename $file .y`
	stem=`echo $file | sed 's/\.y$//'`
	dir=`echo $file | sed 's|/.*||'`

	case $file in

	aegis/aer/report.y)
		numconf="2 reduce/reduce"
		;;

	*)
		numconf="no"
		;;
	esac

	echo ""
	echo "${stem}.gen.c ${stem}.gen.h: $file"
	echo "	@echo Expect $numconf conflicts."
	echo "	$(YACC) -d $file"
	echo "	sed -e 's/[yY][yY]/${root}_/g' y.tab.c > ${stem}.gen.c"
	echo "	sed -e 's/[yY][yY]/${root}_/g' y.tab.h > ${stem}.gen.h"
	echo "	rm y.tab.c y.tab.h"

	depfile=`echo $file | sed 's/\.y$/.gen.d/'`
	if [ -r $depfile ]; then
		dep=`sed -e '1d' -e '$d' -e 's_.arch]/__' $depfile`
	else
		depfile=`echo $rfn | sed 's/\.y$/.gen.d/'`
		if [ -r $depfile ]; then
			dep=`sed -e '1d' -e '$d' -e 's_.arch]/__' $depfile`
		fi
	fi

	echo ""
	echo "${stem}.gen.o: ${stem}.gen.c" $dep
	echo "	$(CC) $(CFLAGS) -I$dir -Icommon -c ${stem}.gen.c"
	echo "	mv ${root}.gen.o ${stem}.gen.o"
	;;

*/*.c)
	root=`basename $file .c`
	stem=`echo $file | sed 's/\.c$//'`
	dir=`echo $file | sed 's|/.*||'`

	depfile=`echo $file | sed 's/\.c$/.d/'`
	if [ -r $depfile ]; then
		dep=`sed -e '1d' -e '$d' -e 's_.arch]/__' $depfile`
	else
		depfile=`echo $rfn | sed 's/\.c$/.d/'`
		if [ -r $depfile ]; then
			dep=`sed -e '1d' -e '$d' -e 's_.arch]/__' $depfile`
		fi
	fi

	extra=
	if [ ${root} = gonzo ]
	then
		extra="-D'LIB=\"$(libdir)\"'"
		extra="$extra -D'AEGIS_USER_UID=$(AEGIS_UID)'"
		extra="$extra -D'AEGIS_USER_GID=$(AEGIS_GID)'"
	fi

	echo ""
	echo "${stem}.o: $file" $dep
	echo "	$(CC) $(CFLAGS)" $extra "-I$dir -Icommon -c $file"
	echo "	mv ${root}.o ${stem}.o"
	;;

lib/*.sh)
	rest=`echo $file | sed 's|^lib/||'`
	dir=`dirname /libdir/$rest | sed s_/_._g`
	echo ""
	echo "$(libdir)/$rest: $dir $file"
	echo "	$(INSTALL_PROGRAM) $file $(libdir)/$rest"
	;;

lib/*)
	rest=`echo $file | sed 's|^lib/||'`
	dir=`dirname /libdir/$rest | sed s_/_._g`
	echo ""
	echo "$(libdir)/$rest: $dir $file"
	echo "	$(INSTALL_DATA) $file $(libdir)/$rest"
	;;


man1/*.1)
	root=`basename $file .1`
	echo ""
	echo "man1/$root.h: $file bin/txt2c"
	echo "	bin/txt2c $file man1/$root.h"
	echo ""
	echo "$(mandir)/$file: $file"
	echo "	(cd man1; soelim $root.1) > tmp"
	echo "	$(INSTALL_DATA) tmp $(mandir)/$file"
	echo "	@rm -f tmp"
	;;

man1/*.so)
	root=`basename $file .so`
	echo ""
	echo "man1/$root.h: $file bin/txt2c"
	echo "	bin/txt2c $file man1/$root.h"
	;;

man5/*.5)
	root=`basename $file .5`
	echo ""
	echo "$(mandir)/$file: $file"
	echo "	(cd man5; soelim $root.5) > tmp"
	echo "	$(INSTALL_DATA) tmp $(mandir)/$file"
	echo "	@rm -f tmp"
	;;

*/*.def)
	root=`basename $file .def`
	stem=`echo $file | sed 's/\.def$//'`
	dir=`echo $file | sed 's|/.*||'`

	dep=
	depfile=`echo $file | sed 's/\.def$/.dd/'`
	if [ -r $depfile ]; then
		dep=`sed -e '1d' -e '$d' -e 's_.arch]/__' $depfile`
	else
		depfile=`echo $rfn | sed 's/\.def$/.dd/'`
		if [ -r $depfile ]; then
			dep=`sed -e '1d' -e '$d' -e 's_.arch]/__' $depfile`
		fi
	fi

	echo ""
	echo "$stem.c $stem.h: $file bin/fmtgen" $dep
	echo "	bin/fmtgen -I$dir $file $stem.c $stem.h"

	dep=
	depfile=`echo $file | sed 's/\.def$/.d/'`
	if [ -r $depfile ]; then
		dep=`sed -e '1d' -e '$d' -e 's_.arch]/__' $depfile`
	else
		depfile=`echo $rfn | sed 's/\.def$/.d/'`
		if [ -r $depfile ]; then
			dep=`sed -e '1d' -e '$d' -e 's_.arch]/__' $depfile`
		fi
	fi

	echo ""
	echo "${stem}.o: ${stem}.c" $dep
	echo "	$(CC) $(CFLAGS) -I$dir -Icommon -c ${stem}.c"
	echo "	mv ${root}.o ${stem}.o"
	;;

test/*/*.sh)
	root=`basename $file .sh`
	echo ""
	echo "$root: $file all"
	echo "	$(SH) $file"
	;;

script/*.tcl)
	root=`basename $file .tcl`
	echo ""
	echo "bin/$root: $file"
	echo "	cp $file bin/$root"
	echo "	chmod a+rx bin/$root"
	;;

*)
	;;
esac
exit 0
