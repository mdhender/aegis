#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991-1999, 2006-2008 Peter Miller
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program. If not, see
#	<http://www.gnu.org/licenses/>.
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
unset LINES
unset COLS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

work=/tmp/$$
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
	echo "FAILED test of 'aegis -VERSion'" 1>&2
	exit 1
}
no_result()
{
	set +x
	cd $here
	rm -rf $work
	echo "NO RESULT for test of 'aegis -VERSion'" 1>&2
	exit 2
}
trap "no_result" 1 2 3 15

mkdir $work
if test $? -ne 0; then exit 2; fi
mkdir $work/lib
if test $? -ne 0; then no_result; fi
cd $work
if test $? -ne 0; then no_result; fi

PAGER=cat
export PAGER

AEGIS_FLAGS="delete_file_preference = no_keep; \
	lock_wait_preference = always; \
	diff_preference = automatic_merge; \
	pager_preference = never; \
	persevere_preference = all; \
	log_file_preference = never;"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

#
# Set the AEGIS_PATH environment variable
# so that man can search there for the man pages.
#
AEGIS_PATH=$work/lib
IFSold="$IFS"
IFS=":$IFS"
for d in ${AEGIS_PATH:-$here}
do
	AEGIS_PATH="${AEGIS_PATH}:$d/lib"
done
IFS="$IFSold"
AEGIS_PATH="${AEGIS_PATH}:$work/lib"
export AEGIS_PATH

unset LANG
unset LANGUAGE

$bin/aegis -vers -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

$bin/aegis -vers -pw=79 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

$bin/aegis -vers lic > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# the things tested by this test, worked
#
pass
