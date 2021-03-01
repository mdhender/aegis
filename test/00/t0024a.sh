#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994-1998, 2005-2008 Peter Miller
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

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$
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

LINES=66
export LINES
COLS=80
export COLS

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

no_result()
{
	set +x
	echo "NO RESULT for test of the report generator functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of the report generator functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	echo PASSED
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
}
trap \"no_result\" 1 2 3 15

activity="working directory 75"
mkdir $work $work/lib $work/lib/report
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi
chmod a+rwx $work/lib
if test $? -ne 0 ; then no_result; fi

AEGIS_PATH=$work/lib
export AEGIS_PATH

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# create project and changes
#
activity="working directory 111"
$bin/aegis -npr foo -vers "" -dir $work/proj
if test $? -ne 0 ; then no_result; fi

activity="new change 115"
cat > $work/fred << 'fubar'
brief_description = "just an example";
cause = internal_bug;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $work/fred -p foo
if test $? -ne 0 ; then no_result; fi

activity="new change 124"
cat > $work/fred << 'fubar'
brief_description = "another example";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f $work/fred -p foo
if test $? -ne 0 ; then no_result; fi

#
# test do and while loops
#
activity="while 136"
cat > test.in << 'fubar'
title("Change State", "delete this line");
columns
(
	{ name = "Number\n------"; width = 6; },
	{ name = "Description\n-----------"; right = 0; }
);
auto ps, cn, cs;
ps = project["foo"].state.branch;
for (cn in sort(keys(ps.change)))
{
	cs = ps.change[cn];
	print(sprintf("%4d", cs.change_number), cs.description);
}
fubar
if test $? -ne 0 ; then no_result; fi
cat > test.ok << 'fubar'
1 just an example
2 another example
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -rpt -f test.in -o test.out -unf
if test $? -ne 0 ; then cat test.out; fail; fi
diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
