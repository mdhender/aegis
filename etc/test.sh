#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1995, 1997 Peter Miller;
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
# MANIFEST: shell script to run tests
#

progname=$0
shell=/bin/sh


usage() {
	echo "usage: $progname -run test-file results-file" 1>&2
	echo "       $progname -summary results-files..." 1>&2
	exit 1
}


run() {
	$shell $1
	echo $? > $2
}


summary() {
	total=$#
	npassed=0
	nfailed=0
	noresult=0
	for f in $*
	do
		case "`cat $f`" in
		0)
			npassed=`expr $npassed + 1`
			;;
		1)
			nfailed=`expr $nfailed + 1`
			;;
		*)
			noresult=`expr $noresult + 1`
			;;
		esac
	done
	if test $npassed -gt 0 ; then
		if test $npassed -eq $total ; then
			echo Passed All Tests
		else
			echo "Passed $npassed of $total tests."
		fi
	fi
	if test $nfailed -gt 0 ; then
		echo "Failed $nfailed of $total tests."
	fi
	if test $noresult -gt 0 ; then
		echo "No result for $noresult of $total tests."
	fi
	if test $nfailed -gt 0 -o $noresult -gt 0 ; then
		exit 1
	fi
}

if test "$1" = "-shell" ; then
	shift
	shell="$1"
	shift
fi

case $1 in
-run)
	shift
	if test $# -ne 2; then usage; fi
	run $*
	;;
-summary)
	shift
	if test $# -lt 1; then usage; fi
	summary $*
	;;
*)
	usage
	exit 1
	;;
esac
exit 0
