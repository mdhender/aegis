#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996, 1997, 1998 Peter Miller;
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
# MANIFEST: Test the aemv directory functionality
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
	echo "FAILED test of the aemv directory functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of the aemv directory functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/$USER/USER/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

activity="working directory 84"
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

AEGIS_PATH=$worklib
export AEGIS_PATH
AEGIS_PROJECT=test
export AEGIS_PROJECT
unset AEGIS_CHANGE

#
# test the aemv directory functionality
#
activity="new project 127"
$bin/aegis -npr test -vers "" -dir $workproj -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > paf << 'fubar'
developer_may_review = true;
reviewer_may_integrate = true;
developer_may_integrate = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -pa -f paf > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 142"
cat > caf << 'fubar'
brief_description = "ten";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc -f caf -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 152"
cat > caf << 'fubar'
brief_description = "eleven";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc -f caf -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 162"
$bin/aegis -ndev $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -nrev $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -nint $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 172"
$bin/aegis -db 10 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 176"
$bin/aegis -nf $workchan/config $workchan/a $workchan/b $workchan/c/d \
	$workchan/c/e > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/config << 'fubar'
build_command = "exit 0";
diff_command = "echo $orig $input > $output";
diff3_command = "exit 0; echo $orig $mr $input $output";
history_create_command = "exit 0; echo $history $input";
history_put_command = "exit 0; echo $history $input";
history_get_command = "exit 0; echo $history $output";
history_query_command = "echo 1.1; exit 0; echo $history";
fubar
if test $? -ne 0 ; then no_result; fi

activity="build 192"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 196"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 200"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 204"
$bin/aegis -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 208"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 212"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 216"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 220"
$bin/aegis -db 11 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 224"
$bin/aegis -nf $workchan/c/x > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="move directory 228"
$bin/aegis -mv $workchan/c $workchan/bozo > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check change file state 232"
cat > ok << 'fubar'
src =
[
	{
		file_name = "bozo/d";
		action = create;
		usage = source;
		move = "c/d";
	},
	{
		file_name = "bozo/e";
		action = create;
		usage = source;
		move = "c/e";
	},
	{
		file_name = "c/d";
		action = remove;
		edit_number_origin = "1.1";
		usage = source;
		move = "bozo/d";
	},
	{
		file_name = "c/e";
		action = remove;
		edit_number_origin = "1.1";
		usage = source;
		move = "bozo/e";
	},
	{
		file_name = "c/x";
		action = create;
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/011.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
