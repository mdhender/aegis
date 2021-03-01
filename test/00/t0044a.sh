#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996-1998, 2000, 2004-2008 Peter Miller
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

work=${AEGIS_TMP:-/tmp}/$$
PAGER=cat
export PAGER
AEGIS_FLAGS="default_project_directory = \"$work\"; \
	lock_wait_preference = always; \
	default_development_directory = \"$work\"; \
	delete_file_preference = no_keep; \
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
	echo "FAILED test of the aenfu directory functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of the aenfu directory functionality ($activity)" 1>&2
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
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

activity="working directory 98"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT
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
# test the aenfu directory functionality
#
activity="new project 122"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -v -dir $work/test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 126"
cat > paf << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 137"
cat > caf << 'fubar'
brief_description = "ten";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -p $AEGIS_PROJECT -f caf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 146"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 154"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 158"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/a \
	$work/test.C010/c/x $work/test.C010/c/y -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "exit 0";
diff_command = "echo $orig $input > $output";
diff3_command = "exit 0; echo $orig $mr $input $output";
history_create_command = "exit 0; echo $history $input";
history_put_command = "exit 0; echo $history $input";
history_get_command = "exit 0; echo $history $output";
history_query_command = "echo 1.1; exit 0; echo $history";
fubar
if test $? -ne 0 ; then no_result; fi

activity="new file undo 174"
$bin/aegis -nfu $work/test.C010/c -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check change file state 178"
cat > ok << 'fubar'
src =
[
	{
		file_name = "a";
		action = create;
		usage = source;
	},
	{
		file_name = "aegis.conf";
		action = create;
		usage = config;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/010.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
