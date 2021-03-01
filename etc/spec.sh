#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1997, 1999 Peter Miller;
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
# MANIFEST: shell script to generate RedHat spec file
#
version=${version-0.0.0}
echo 'Summary: project change supervisor'
echo 'Name: aegis'
echo "Version: ${version}"
echo 'Release: 1'
echo 'Copyright: GPL'
echo 'Group: Development/Version Control'
echo "Source: http://www.canb.auug.org.au/~millerp/aegis-${version}.tar.gz"
echo 'URL: http://www.canb.auug.org.au/~millerp/aegis.html'
echo 'BuildRoot: /tmp/aegis-build-root'
echo 'Icon: aegis.gif'

prefix=/usr
#
# RPM only has install-time relocatable packages.  It has no support for
# build-time relocatable packages.  Therefore, we must NOT specify a Spec
# prefix, or the installed locations will not match the built locations.
#
#echo "Prefix: $prefix"

echo ''

cat << 'fubar'
%description
Aegis is a transaction-based software configuration management system.
It provides a framework within which a team of developers may work
on many changes to a program independently, and Aegis coordinates
integrating these changes back into the master source of the program,
with as little disruption as possible.

%package txtdocs
Summary: Aegis documentation, dumb ascii text
Group: Development/Building

%description txtdocs
Aegis documentation in dumb ascii text format.

%package psdocs
Summary: Aegis documentation, PostScript format
Group: Development/Building

%description psdocs
Aegis documentation in PostScript format.

%package dvidocs
Summary: aegis documentation, DVI format
Group: Development/Building

%description dvidocs
Aegis documentation in DVI format.

%prep
fubar

echo '%setup'
echo ''
echo '%build'
echo "./configure --prefix=$prefix"
echo 'make'
echo ''
echo '%install'
echo 'make RPM_BUILD_ROOT=$RPM_BUILD_ROOT install'

#
# remember things for the %files section
#
files_ro=
files_rx=
txtdocs=
psdocs=
dvidocs=

remember_prog()
{
	if eval "test \"\${prog_${1}-no}\" != yes"
	then
		eval "prog_${1}=yes"
		files_rx="$files_rx $prefix/bin/${1}"
	fi
}

for file in $*
do
	case $file in
	*.in)
		file=`echo $file | sed 's|[.]in$||'`
		;;
	esac

	case $file in

	aefp/* | etc/* | common/* | find_sizes/* | fmtgen/* | \
	fstrcmp/* | libaegis/* | test/* | test_*)
		;;

	*/main.c)
		dir=`echo $file | sed 's|/.*||'`
		remember_prog $dir
		;;

	lib/*/building/* | lib/*/lsm/* | lib/*/readme/* | \
	lib/*/release/* | lib/*/LC_MESSAGES/libaegis.po)
		;;

	lib/*.po)
		stem=`echo $file | sed 's|^lib/\(.*\)\.po$|\1|'`
		dst="$prefix/lib/aegis/$stem.mo"
		files_ro="$files_ro $dst"
		;;

	lib/*/*/*.so | lib/*/*/*.bib | lib/*/*/*.pic)
		;;

	lib/*.uue )
		;;

	lib/*/man?/*)
		stem=`echo $file | sed 's|^lib/||'`
		files_ro="$files_ro $prefix/share/aegis/$stem"

		case $file in
		lib/en/*)
			stem2=`echo $file | sed 's|^lib/en/||'`
			files_ro="$files_ro $prefix/man/$stem2"
			;;
		esac
		;;

	lib/*.cgi)
		stem=`echo $file | sed 's|^lib/||'`
		files_rx="$files_rx /home/httpd/cgi-bin/$stem"
		;;

	lib/*/*/main.*)
		stem=`echo $file | sed 's|^lib/\(.*\)/main.*$|\1|'`
		psdocs="$psdocs $prefix/share/aegis/$stem.ps"
		dvidocs="$dvidocs $prefix/share/aegis/$stem.dvi"
		txtdocs="$txtdocs $prefix/share/aegis/$stem.txt"
		;;

	lib/*.sh)
		rest=`echo $file | sed 's|^lib/||'`
		dst="$prefix/share/aegis/$rest"
		files_rx="$files_rx $dst"
		;;

	lib/*)
		rest=`echo $file | sed 's|^lib/||'`
		dst="$prefix/share/aegis/$rest"
		files_ro="$files_ro $dst"
		;;

	*)
		;;
	esac
done

echo ''
echo '%files'
echo "%attr(0755,root,bin) %dir $prefix/com/aegis"
echo "%attr(0755,root,bin) %dir $prefix/lib/aegis"
echo "%attr(0755,root,bin) %dir $prefix/share/aegis"
for file in $files_rx
do
	case $file in
	*/bin/aegis)
		echo "%attr(4755,root,bin) $file"
		;;
	*/bin/*)
		echo "%attr(0755,root,bin) $file"
		;;
	*)
		echo "%attr(0755,root,bin) $file"
		;;
	esac
done
for file in $files_ro
do
	echo "%attr(0644,root,bin) $file"
done

echo ''
echo '%files txtdocs'
for file in $txtdocs
do
	echo "%attr(0644,root,bin) $file"
done

echo ''
echo '%files psdocs'
for file in $psdocs
do
	echo "%attr(0644,root,bin) $file"
done

echo ''
echo '%files dvidocs'
for file in $dvidocs
do
	echo "%attr(0644,root,bin) $file"
done

echo ''
echo '%post'
echo "chown -R 3 $prefix/com/aegis $prefix/lib/aegis $prefix/share/aegis"
echo "chgrp -R 3 $prefix/com/aegis $prefix/lib/aegis $prefix/share/aegis"

echo ''
echo '%clean'
echo 'rm -rf $RPM_BUILD_ROOT'
