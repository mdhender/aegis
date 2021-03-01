#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1997, 1999-2004 Peter Miller;
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
echo '#'
echo '# This file is GENERATED.  Please DO NOT send the maintainer'
echo '# patches to this file.  If there is a problemn with this file,'
echo '# fix etc/spec.sh, and set the etc/spec.sh patch to the maintainer.'
echo '#'
echo 'Summary: project change supervisor'
echo 'Name: aegis'
echo "Version: ${version}"
echo 'Release: 1'
echo 'Copyright: GPL'
echo 'Group: Development/Version Control'
echo "Source: http://www.canb.auug.org.au/~millerp/aegis-${version}.tar.gz"
echo 'URL: http://www.canb.auug.org.au/~millerp/aegis.html'
echo 'BuildRoot: /tmp/aegis-build-root'
echo 'Icon: aegis.xpm'

#
# Build-time prerequisites, things you have to have installed so that
# this package will build successfully, correctly and completely.
#
# We want -lmagic, too, but it isn't an RPM package yet.  It will be
# file-devel or magic-devel, eventually.
#
echo 'BuildPrereq: bison, curl-devel, diffutils, gawk, gettext >= 0.11.4'
echo 'BuildPrereq: groff >= 1.15, perl, tk, zlib-devel'

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
Group: Development/Version Control

%description txtdocs
Aegis documentation in dumb ascii text format.

%package psdocs
Summary: Aegis documentation, PostScript format
Group: Development/Version Control

%description psdocs
Aegis documentation in PostScript format.

%prep
fubar

echo '%setup'
echo ''
echo '%build'
echo "./configure --sysconfdir=/etc --prefix=$prefix"
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
dirs=$prefix/com/aegis
name=`echo dir_${prefix} | sed 's/[^a-zA-Z0-9]/_/g'`
eval "${name}=yes"
eval "${name}_share=yes"
eval "${name}_lib=yes"
eval "${name}_com=yes"
eval "${name}_man=yes"
eval "${name}_man_man1=yes"
eval "${name}_man_man5=yes"

remember_prog()
{
	name=`echo ${1} | sed 's/[^a-zA-Z0-9]/_/g'`
	if eval "test \"\${${name}-no}\" != yes"
	then
		eval "${name}=yes"
		files_rx="$files_rx $prefix/bin/${1}"
	fi
}
remember_dir()
{
	case "${1}" in
	/)
		;;
	*)
		remember_dir `dirname ${1}`

		name=`echo dir_${1} | sed 's/[^a-zA-Z0-9]/_/g'`
		if eval "test \"\${${name}-no}\" != yes"
		then
			eval "${name}=yes"
			dirs="$dirs ${1}"
		fi
		;;
	esac
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
	fstrcmp/* | libaegis/* | test/* | test_* | cklinlen/* | \
	aemanifest/* | aemakefile/* )
		;;

	*/main.cc)
		dir=`echo $file | sed 's|/.*||'`
		remember_prog $dir
		;;

	script/aegis.synpic | script/ae-symlinks)
		;;

	script/*)
		prog=`echo $file | sed 's|.*/||'`
		remember_prog $prog
		;;

	lib/*/LC_MESSAGES/libaegis.po)
		;;

	lib/*.po)
		stem=`echo $file | sed 's|^lib/\(.*\)\.po$|\1|'`
		dst="$prefix/lib/aegis/$stem.mo"
		files_ro="$files_ro $dst"
		remember_dir `dirname $dst`
		;;

	lib/*/*/*.so | lib/*/*/*.bib | lib/*/*/*.pic)
		;;

	lib/*.uue )
		;;

	lib/*/man?/*)
		# Some versions of RPM gzip man pages for free.  This is
		# a pain in the behind.  Use a pattern to find them.  Sigh.
		stem=`echo $file | sed 's|^lib/||'`
		dst="$prefix/share/aegis/${stem}*"
		files_ro="$files_ro $dst"
		remember_dir `dirname $dst`

		case $file in
		lib/en/*)
			stem2=`echo $file | sed 's|^lib/en/||'`
			dst="$prefix/man/${stem2}*"
			files_ro="$files_ro $dst"
			remember_dir `dirname $dst`
			;;
		esac
		;;

	lib/*/*/main.*)
		stem=`echo $file | sed 's|^lib/\(.*\)/main.*$|\1|'`
		psdocs="$psdocs $prefix/share/aegis/$stem.ps"
		txtdocs="$txtdocs $prefix/share/aegis/$stem.txt"
		;;

	lib/*.sh)
		rest=`echo $file | sed 's|^lib/||'`
		dst="$prefix/share/aegis/$rest"
		files_rx="$files_rx $dst"
		remember_dir `dirname $dst`
		;;

	lib/cshrc.in | lib/profile.in | lib/cshrc | lib/profile )
		rest=`echo $file | sed -e 's|^lib/||' -e 's|\.in$||'`
		dst="$prefix/share/aegis/$rest"
		files_rx="$files_rx $dst"
		remember_dir `dirname $dst`
		;;

	lib/*)
		rest=`echo $file | sed 's|^lib/||'`
		dst="$prefix/share/aegis/$rest"
		files_ro="$files_ro $dst"
		remember_dir `dirname $dst`
		;;

	*)
		;;
	esac
done

grumble()
{
	echo '#'
	echo "# See the comment at the top of this file.  If you don't like"
	echo '# the file attributes, or there is a file missing, DO NOT send'
	echo '# the maintainer a patch to this file.  This file is GENERATED.'
	echo '# If you want different attributes, fix the etc/spec.sh file,'
	echo '# and send THAT patch to the maintainer.'
	echo '#'
}

echo ''
grumble
echo '%files'
for file in $dirs
do
	echo "%attr(0755,root,bin) %dir $file"
done
for file in $files_rx
do
	case $file in
	*/bin/aegis | */bin/aeimport)
		echo "%attr(4755,root,bin) $file"
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
echo "%attr(0755,root,bin) /etc/profile.d/aegis.sh"
echo "%attr(0755,root,bin) /etc/profile.d/aegis.csh"

echo ''
grumble
echo '%files txtdocs'
echo "%attr(0755,root,bin) %dir $prefix/share/aegis"
for file in $txtdocs
do
	echo "%attr(0644,root,bin) $file"
done

echo ''
grumble
echo '%files psdocs'
echo "%attr(0755,root,bin) %dir $prefix/share/aegis"
for file in $psdocs
do
	echo "%attr(0644,root,bin) $file"
done

echo ''
echo '#'
echo '# This next bit is done because when using Aegis with NFS, these'
echo '# files must have EXACTLY the same uid and gid on all systems.'
echo "# Unfortunately, RPM won't let you give exact numeric uids and gids,"
echo '# and the names for low-numbered uids and gids are essentially'
echo '# random across the various Unix implementations.  Sigh.'
echo '#'
echo '%post'
echo "chown -R 3 $prefix/com/aegis $prefix/lib/aegis $prefix/share/aegis"
echo "chgrp -R 3 $prefix/com/aegis $prefix/lib/aegis $prefix/share/aegis"

echo ''
echo '%clean'
echo 'rm -rf $RPM_BUILD_ROOT'
