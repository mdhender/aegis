#!/bin/sh
#
#	aegis - a project change supervisor
#	Copyright (C) 1990-2004 Peter Miller;
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
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: shell script to generate Makefile fragment for each source file
#
depfile=no-such-file-or-directory
case $# in
2)
	;;
3)
	depfile=$3
	;;
*)
	echo "usage: $0 filename resolved-filename [ resolved-depfile ]" 1>&2
	exit 1
	;;
esac
file="$1"
rfn="$2"

case $file in
*.in)
	file=`echo $file | sed 's/[.]in$//'`
	rfn=`echo $rfn | sed 's/[.]in$//'`
	;;
*)
	;;
esac

case $file in

*/*.y)
	root=`basename $file .y`
	stem=`echo $file | sed -e 's/\.y$//'`
	dir=`echo $file | sed -e 's|/.*||'`
	yy=`echo $stem | sed -e 's|^[^/]*/||' -e 's|[^a-zA-Z0-9]|_|g'`

	case $file in

	common/gettime.y)
		numconf="8 shift/reduce"
		;;

	libaegis/aer/report.y)
		numconf="2 reduce/reduce"
		;;

	aefind/cmdline.y)
		numconf="5 shift/reduce, 23 reduce/reduce"
		;;

	common/gettime.y)
		numconf="8 shift/reduce"
		;;

	*)
		numconf="no"
		;;
	esac

	echo ""
	echo "${stem}.gen.cc ${stem}.gen.h: $file"
	echo "	@echo Expect $numconf conflicts."
	echo "	\$(YACC) -d $file"
	echo "	sed -e 's/[yY][yY]/${yy}_/g'" \
		"-e '/<stdio.h>/d'" \
		"-e '/<stdlib.h>/d'" \
		"-e '/<stddef.h>/d'" \
		"y.tab.c > ${stem}.gen.cc"
	echo "	sed -e 's/[yY][yY]/${yy}_/g' -e 's/Y_TAB_H/${yy}_TAB_H/g'" \
		"y.tab.h > ${stem}.gen.h"
	echo "	rm -f y.tab.c y.tab.h"
	;;

*/*.cc)
	root=`basename $file .cc`
	stem=`echo $file | sed -e 's/\.cc$//'`
	dir=`echo $file | sed -e 's|/.*||'`

	dep=
	if [ -r $depfile ]; then
		dep=`sed -e 's_.arch]/__' $depfile`
	fi

	echo ""
	echo "${stem}.\$(OBJEXT): $file" $dep
	echo "	\$(CXX) -I$dir -Ilibaegis -Icommon \$(CXXFLAGS) -c $file"
	echo "	mv ${root}.\$(OBJEXT) \$@"
	;;

lib/*.gif.uue)
	rest=`echo $file | sed -e 's|^lib/\(.*\).uue$|\1|'`
	dir=`dirname $file`
	echo ""
	echo "lib/$rest: $file"
	echo "	uudecode $file"
	echo ""
	echo "\$(RPM_BUILD_ROOT)\$(IconRoot)/$rest: lib/$rest" \
		"$dir/.mkdir.script"
	echo "	\$(INSTALL_DATA) $file \$@"
	;;

lib/cshrc | lib/profile)
	# should only be setting ${sharedstatedir}
	# so be inconsistent and install them in share
	rest=`echo $file | sed -e 's|^lib/||'`
	dir=`dirname $file`
	echo ""
	echo "\$(RPM_BUILD_ROOT)\$(datadir)/$rest: $file $dir/.mkdir.datadir"
	echo "	\$(INSTALL_SCRIPT) $file \$@"
	case $file in
	lib/profile)
		echo
		echo "\$(RPM_BUILD_ROOT)\$(sysconfdir)/profile.d/aegis.sh:" \
			"\$(RPM_BUILD_ROOT)\$(datadir)/$rest"
		echo '	-@mkdir -p $(RPM_BUILD_ROOT)\$(sysconfdir)/profile.d'
		echo "	-ln -s \$(datadir)/$rest \$@"
		;;
	lib/cshrc)
		echo
		echo "\$(RPM_BUILD_ROOT)\$(sysconfdir)/profile.d/aegis.csh:" \
			"\$(RPM_BUILD_ROOT)\$(datadir)/$rest"
		echo '	-@mkdir -p $(RPM_BUILD_ROOT)\$(sysconfdir)/profile.d'
		echo "	-ln -s \$(datadir)/$rest \$@"
		;;
	*)
		;;
	esac
	;;

lib/*.sh)
	rest=`echo $file | sed -e 's|^lib/||'`
	dir=`dirname $file`
	echo ""
	echo "\$(RPM_BUILD_ROOT)\$(datadir)/$rest: $file $dir/.mkdir.datadir"
	echo "	\$(INSTALL_SCRIPT) $file \$@"
	;;

lib/*/libaegis.po)
	# obsolete
	;;

lib/*.po)
	#
	# Assume that we are using the GNU Gettext program.  All others
	# will fail, because they do not have the -o option.
	#
	stem=`echo $file | sed -e 's|^lib/\(.*\)\.po$|\1|'`
	dir=`dirname $file`
	echo ""
	echo "lib/$stem.mo: etc/msgfmt.sh $file"
	echo "	\$(SH) etc/msgfmt.sh --msgfmt=\$(MSGFMT) --msgcat=\$(MSGCAT)" \
	    "--output=\$@ $file"
	echo ""
	echo "\$(RPM_BUILD_ROOT)\$(NLSDIR)/$stem.mo: lib/$stem.mo \
$dir/.mkdir.libdir"
	echo "	\$(INSTALL_DATA) lib/$stem.mo \$@"
	echo "	-chown \$(AEGIS_UID) \$@ && chgrp \$(AEGIS_GID) \$@"
	;;

lib/*/man[[1-9]/*.[1-9])
	dir=`echo $file | sed 's|^\(lib/.*/man[1-9]\)/.*|\1|'`
	base=`echo $file | sed 's|^lib/.*/man[1-9]/\(.*\)|\1|'`
	stem=`echo $file | sed 's|^lib/\(.*\)|\1|'`
	part=`echo $file | sed 's|^lib/.*/\(man[1-9]/.*\)|\1|'`
	ugly=`echo $file | sed 's|^lib/.*/\(man[1-9]\)/.*|\1|'`

	dep=
	if [ -r $depfile ]; then
		dep=`sed -e 's_.arch]/__' $depfile`
	fi

	echo ""
	echo "\$(RPM_BUILD_ROOT)\$(datadir)/$stem: $file $dir/.mkdir.datadir" \
		$dep
	echo "	\$(SOELIM) -I$dir -Ietc $file | sed '/^\.lf/d'" \
		"> \$\${TMPDIR-/tmp}/aegis.tmp"
	echo "	\$(INSTALL_DATA) \$\${TMPDIR-/tmp}/aegis.tmp \$@"
	echo "	-chown \$(AEGIS_UID) \$@ && chgrp \$(AEGIS_GID) \$@"
	echo "	@rm -f \$\${TMPDIR-/tmp}/aegis.tmp"
	echo ""
	echo "\$(RPM_BUILD_ROOT)\$(mandir)/$part: $file" $dep .${ugly}dir
	echo "	\$(SOELIM) -I$dir -Ietc $file | sed '/^\.lf/d'" \
		"> \$\${TMPDIR-/tmp}/aegis.tmp"
	echo "	\$(INSTALL_DATA) \$\${TMPDIR-/tmp}/aegis.tmp \$@"
	echo "	-chown \$(AEGIS_UID) \$@ && chgrp \$(AEGIS_GID) \$@"
	echo "	@rm -f \$\${TMPDIR-/tmp}/aegis.tmp"
	;;

lib/*/man?/*)
	;;

lib/*/*/*.so)
	;;

lib/*/*/*.bib)
	;;

lib/*/*/main.*)
	macros=`echo $file | sed 's|^lib/.*/.*/main.\(.*\)$|\1|'`
	stem=`echo $file | sed 's|^lib/\(.*/.*/main\).*$|\1|'`
	dir=`dirname $file`
	dirdir=`dirname $dir`

	dep=
	if [ -r $depfile ]; then
		dep=`sed -e 's_.arch]/__' $depfile`
	fi

	case $macros in
	roff)
		macros=""
		;;
	mm)
		macros='$(MM)'
		;;
	ms)
		macros='$(MS)'
		;;
	*)
		macros="-$macros"
		;;
	esac
	stem2=`dirname $stem`
	stem3=`dirname $stem2`

	echo ""
	echo "lib/$stem2.ps: $file" $dep
	echo "	\$(SOELIM) -I$dir -Ietc -I$dirdir/man1 -I$dirdir/man5" \
		"-I$dirdir/readme $file | \$(GROFF) -R -t -p $macros -mpic" \
		"-mpspic > \$@"

	echo ""
	echo "\$(RPM_BUILD_ROOT)\$(datadir)/$stem2.ps: lib/$stem2.ps" \
		"lib/$stem3/.mkdir.datadir"
	echo "	\$(INSTALL_DATA) lib/$stem2.ps \$@"

	echo ""
	echo "lib/$stem2.dvi: $file" $dep
	echo "	\$(SOELIM) -I$dir -Ietc -I$dirdir/man1 -I$dirdir/man5" \
		"-I$dirdir/readme $file | \$(GROFF) -Tdvi -R -t -p $macros" \
		"-mpic > \$@"

	echo ""
	echo "\$(RPM_BUILD_ROOT)\$(datadir)/$stem2.dvi: lib/$stem2.dvi" \
		"lib/$stem3/.mkdir.datadir"
	echo "	\$(INSTALL_DATA) lib/$stem2.dvi \$@"

	echo ""
	echo "lib/$stem2.txt: $file " $dep
	echo "	-\$(SOELIM) -I$dir -Ietc -I$dirdir/man1 -I$dirdir/man5" \
		"-I$dirdir/readme $file | \$(GROFF) -Tascii -R -t -p $macros" \
		"-mpic > \$@"

	echo ""
	echo "\$(RPM_BUILD_ROOT)\$(datadir)/$stem2.txt: lib/$stem2.txt" \
		"lib/$stem3/.mkdir.datadir"
	echo "	\$(INSTALL_DATA) lib/$stem2.txt \$@"
	;;

lib/*)
	stem=`echo $file | sed -e 's|^lib/||'`
	dir=`dirname $file`
	echo ""
	echo "\$(RPM_BUILD_ROOT)\$(datadir)/$stem: $file $dir/.mkdir.datadir"
	echo "	\$(INSTALL_DATA) $file \$@"
	echo "	-chown \$(AEGIS_UID) \$@ && chgrp \$(AEGIS_GID) \$@"
	;;

*/*.def)
	root=`basename $file .def`
	stem=`echo $file | sed -e 's/\.def$//'`
	dir=`echo $file | sed -e 's|/.*||'`

	dep=
	if [ -r $depfile ]; then
		dep=`sed -e 's_.arch]/__' $depfile`
	fi

	echo ""
	echo "$stem.cc $stem.h: $file bin/fmtgen\$(EXEEXT)" $dep
	echo "	bin/fmtgen\$(EXEEXT) -I$dir $file $stem.cc $stem.h"
	;;

test/*/*.sh)
	stem=`echo $file | sed -e 's/\.sh$//'`
	echo ""
	echo "$stem.ES: $file all-bin etc/test.sh"
	echo "	CXX=\"\$(CXX)\" \$(SH) etc/test.sh -shell \$(SH) -run $file" \
		"$stem.ES"
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
