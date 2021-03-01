#! /bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1992, 1993 Peter Miller.
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
# MANIFEST: shell script to generate the patch file
#
case $# in
2)
	project=$1
	change=$2
	;;
*)
	echo "usage: $0 <project> <change>" 1>&2
	exit 1
	;;
esac

set -e
tmp=/tmp/$$
tmp2=/tmp/$$.2
bl=`aegis -cd -bl -p $project`

#
# These files are generated
# so we don't have an older version in history
#
weird="README BUILDING CHANGES MANIFEST Makefile doc/version.so aux/new.so"
echo "#! /bin/sh"
echo "#"
echo "# This is a self-applying patch in a shell script."
echo "#"
echo "# Change directory to the appropriate place"
echo "# before applying running this shell script."
echo "#"
echo "# Don't forget the -p0 option if you apply this patch manually."
echo "#"
echo "# ------------------------------------------------------------"
echo
echo "#"
echo "# Before you apply this patch manually,"
echo "# please delete the following files:"
echo "#"
echo "rm -f $weird"
echo
echo "#"
echo "# Now patch the files..."
echo "#	(Don't forget the -p0 option.)"
echo "#"
echo "patch -p0 << 'fubar'"

#
# the patchlevel patch can be generated accurately
#
echo "Index: common/patchlevel.h"
prev=`aegis -list version -p $project -c $change | awk -F'"' '/previous/{print $2}'`
echo "Prereq: \"$prev\""
echo "#define PATCHLEVEL \"$prev\"" > $tmp
set +e
diff -c $tmp common/patchlevel.h | sed '1,2d'
set -e

#
# get a list of files from aegis
#
files=`(aegis -l pf -p $project -c $change -ter; aegis -l cf -p $project -c $change -ter) | sort -u`

#
# get the diff listing for each file
#
for f in $files
do
	if [ "$f" != "common/patchlevel.h" ]
	then
		aegis -cp $f -delta 1 -output $tmp -p $project -c $change
		set +e
		if [ -f $f ]
		then
			fn=$f
		else
			fn=$bl/$f
		fi
		if diff -c $tmp $fn > $tmp2 2> /dev/null
		then
			set -e
		else
			set -e
			echo "Index: $f"
			sed '1,2d' < $tmp2
		fi
	fi
done
echo "fubar"

#
# clean up and go home
#
rm -f $tmp $tmp2
exit 0
