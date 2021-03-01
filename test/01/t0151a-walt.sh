#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004 Walter Franzini;
#	All rights reserved.
#       Copyright (C) 2007, 2008 Peter Miller
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
umask 022

LINES=24
export LINES
COLS=80
export COLS

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$
workchan=${work}/foo.chan
workproj=${work}/foo.proj
tmp=${work}/tmp
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
AEGIS_PROJECT=foo
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin=$here/${1-.}/bin

pass()
{
	set +x
	echo PASSED 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
}
fail()
{
	set +x
	echo "FAILED test of the aenbru functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aenbru functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# make a new project
#
activity="new project 128"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch 1  132"
$bin/aegis -nbr -p foo 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch 1.1 136"
$bin/aegis -nbr -p foo.1 1 >  log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch alias 140"
$bin/aegis -npa -p foo.1.1 balias > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new project alias 144"
$bin/aegis -npa -p foo palias > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="alias list 148"
$bin/aegis -list pa -unf | grep alias | sort > alias.list
if test $? -ne 0; then no_result; fi

cat > alias.expect <<EOF
balias foo.1.1
palias foo
EOF

diff alias.expect alias.list > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

cat > alias2.expect <<EOF
palias foo
EOF

activity="new branch undo 164"
$bin/aegis -nbru -p foo.1 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="alias list after nbru 168"
$bin/aegis -list pa -unf | grep alias | sort > alias2.list
if test $? -ne 0; then cat log; no_result; fi

diff alias2.expect alias2.list > log
if test $? -ne 0; then cat log ; fail ; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
