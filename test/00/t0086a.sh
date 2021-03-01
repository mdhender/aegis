#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1999, 2006-2008 Peter Miller
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
	echo 'FAILED test of the skip_unlucky functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the skip_unlucky functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

#
# test the skip_unlucky functionality
#
worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT

#
# make the directories
#
activity="working directory 81"
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
activity="new project 161"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 168"
cat > $tmp << 'end'
description = "A bogus project created to test the new change -o functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
skip_unlucky = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# create a new change
#
activity="new change 183"
cat > $tmp << 'end'
brief_description = "nothing much";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc -o $work/NUM -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > ok << 'fubar'
10
fubar
if test $? -ne 0 ; then no_result; fi
diff ok NUM
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -nc -o $work/NUM -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > ok << 'fubar'
11
fubar
if test $? -ne 0 ; then no_result; fi
diff ok NUM
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -nc -o $work/NUM -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > ok << 'fubar'
12
fubar
if test $? -ne 0 ; then no_result; fi
diff ok NUM
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -nc -o $work/NUM -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > ok << 'fubar'
15
fubar
if test $? -ne 0 ; then no_result; fi
diff ok NUM
if test $? -ne 0 ; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
