#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1997, 1998 Peter Miller;
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
# MANIFEST: Test the whiteout functionality
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
unset LINES
unset COLS
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
AEGIS_THROTTLE=2
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
	echo 'FAILED test of the whiteout functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the whiteout functionality' 1>&2
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

workproj=$work/proj
workchan=$work/chan
worklib=$work/lib

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the whiteout functionality
#
activity="new project 100"
$bin/aegis -npr test -vers '' -dir $workproj -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 104"
$bin/aegis -nd $USER -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="staff 107"
$bin/aegis -nrv $USER -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="staff 110"
$bin/aegis -ni $USER -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 114"
cat > pa << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f pa -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="create branch 124"
$bin/aegis -nbr 4 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a change to the trunk
#
activity="new change 131"
cat > ca << 'fubar'
brief_description = "first";
description = "This is the first change.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f ca -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 144"
$bin/aegis -db 10 -p test -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 148"
$bin/aegis -nf $workchan/config $workchan/a $workchan/b/c $workchan/b/d \
	-c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/config << 'fubar'
build_command = "exit 0";

history_get_command =
	"co -u'$e' -p $h,v > $o";
history_create_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_put_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_query_command =
	"rlog -r $h,v | awk '/^head:/ {print $$2}'";

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";

diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";

link_integration_directory = true;
fubar
if test $? -ne 0 ; then no_result; fi

# build
activity="build 175"
$bin/aegis -b -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="difference 179"
$bin/aegis -diff -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 183"
$bin/aegis -de -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 187"
$bin/aegis -rpass 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 191"
$bin/aegis -ib 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# this one checks symlinks
activity="build 196"
$bin/aegis -b -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 200"
$bin/aegis -ipass 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a change to branch 4
#
activity="new change 207"
cat > ca << 'fubar'
brief_description = "first";
description = "This is the second change.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f ca -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop begin
activity="develop begin 221"
$bin/aegis -db 10 -p test.4 -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="remove file 225"
$bin/aegis -rm $workchan/b/c -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# build
activity="build 230"
$bin/aegis -b -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="difference 234"
$bin/aegis -diff -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 238"
$bin/aegis -de -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 242"
$bin/aegis -rpass 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 246"
$bin/aegis -ib 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# build
activity="build 251"
$bin/aegis -b -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="difference 255"
$bin/aegis -diff -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 259"
$bin/aegis -ipass 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a change to branch 4
#
activity="new change 266"
cat > ca << 'fubar'
brief_description = "third";
description = "This is the third change.";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f ca -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop begin
activity="develop begin 277"
$bin/aegis -db 11 -p test.4 -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# This is what it's all about,
#	make sure that we can do a directory copy of the ``b'' directory
#
activity="directory copy 285"
$bin/aegis -cp $workchan/b -p test.4 -c 11 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass