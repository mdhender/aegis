#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998 Peter Miller;
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
# MANIFEST: test -Help for each function
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
unset LINES
unset COLS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

here=`pwd`
if test $? -ne 0; then exit 2; fi
if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

pass()
{
	set +x
	echo PASSED
	cd $here
	rm -rf $work
	exit 0
}
fail()
{
	set +x
	cd $here
	rm -rf $work
	echo FAILED test of -Help for each function 1>&2
	exit 1
}
no_result()
{
	set +x
	cd $here
	rm -rf $work
	echo NO RESULT for test of -Help for each function 1>&2
	exit 2
}
trap "no_result" 1 2 3 15

work=/tmp/$$
mkdir $work $work/lib
if test $? -ne 0; then no_result ; fi
cd $work
if test $? -ne 0; then no_result ; fi

#
# make sure the various variations have -help implemented
#
PAGER=cat
export PAGER

AEGIS_FLAGS="delete_file_preference = no_keep; \
	diff_preference = automatic_merge; \
	pager_preference = never; \
	persevere_preference = all; \
	lock_wait_preference = always; \
	log_file_preference = never;"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

#
# Set the AEGIS_PATH environment variable
# so that man can search there for the man pages.
#
AEGIS_PATH=$work/lib
IFSold="$IFS"
IFS=":$IFS"
for d in ${AEGIS_PATH-$here}
do
	AEGIS_PATH="${AEGIS_PATH}:$d/lib"
done
IFS="$IFSold"
AEGIS_PATH="${AEGIS_PATH}:$work/lib"
export AEGIS_PATH

unset LANG
unset LANGUAGE

PAGER=cat
export PAGER

$bin/aegis -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -b -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -ca -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -cd -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -cp -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -cpu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -db -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -dbu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -de -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -deu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -diff -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -ib -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -ibu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -ipass -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -ifail -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -l -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -mv -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -na -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -nc -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -ncu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -nd -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -nf -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -nfu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -ni -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -npr -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -nrls -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -nrv -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -nt -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -ntu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -pa -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -ra -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -rd -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -rm -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -rmu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -rmpr -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -ri -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -rrv -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -rfail -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -rpass -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -rpu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -t -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
$bin/aegis -vers -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

#
# the things tested in this test, worked
#
pass
