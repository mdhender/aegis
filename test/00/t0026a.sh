#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994-1999, 2005-2008 Peter Miller
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
	echo "NO RESULT for test of the aegis -Delta_Name functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of the aegis -Delta_Name functionality ($activity)" 1>&2
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

activity="working directory 85"
mkdir $work $work/lib $work/lib/report
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi
chmod 0777 $work/lib
if test $? -ne 0 ; then no_result; fi

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

AEGIS_PATH=$work/lib
export AEGIS_PATH
AEGIS_PROJECT=foo
export AEGIS_PROJECT

#
# create project and changes
#
activity="new project 109"
$bin/aegis -npr foo -vers "" -dir $work/proj
if test $? -ne 0 ; then no_result; fi

activity="staff 113"
$bin/aegis -nd $USER
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nrv $USER
if test $? -ne 0 ; then no_result; fi
$bin/aegis -ni $USER
if test $? -ne 0 ; then no_result; fi

activity="new change 121"
cat > $work/fred << 'fubar'
brief_description = "just an example";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $work/fred -p foo
if test $? -ne 0 ; then no_result; fi

activity="new change 132"
cat > $work/fred << 'fubar'
brief_description = "another example";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f $work/fred -p foo
if test $? -ne 0 ; then no_result; fi

activity="project attributes 143"
cat > $work/fred << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
developers_may_create_changes = true;
umask = 022;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $work/fred
if test $? -ne 0 ; then no_result; fi

#
# do first change
#
activity="develop begin 159"
$bin/aegis -db 1 -dir $work/dd1
if test $? -ne 0 ; then no_result; fi
activity="new file 162"
$bin/aegis -nf $work/dd1/aegis.conf
if test $? -ne 0 ; then no_result; fi
cat > $work/dd1/aegis.conf << 'fubar'
build_command = "exit 0";
history_create_command = "echo $history $input > /dev/null";
history_put_command = "echo $history $input > /dev/null";
history_get_command = "echo $history > $output";
history_query_command = "echo $history > /dev/null ; echo 1.1";
diff_command = "set +e; diff $orig $in > $out; test $? -le 1";
diff3_command = "echo diff3 $orig $mr $in > $out";
fubar
if test $? -ne 0 ; then no_result; fi
activity="build 175"
$bin/aegis -build -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="diff 178"
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="develop end 181"
$bin/aegis -de
if test $? -ne 0 ; then no_result; fi
activity="review pass 184"
$bin/aegis -rpass 1
if test $? -ne 0 ; then no_result; fi
activity="integrate begin 187"
$bin/aegis -ib 1
if test $? -ne 0 ; then no_result; fi
activity="build 190"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="integrate pass 193"
$bin/aegis -ipass -nl > errlog 2>&1
if test $? -ne 0 ; then cat errlog; no_result; fi

#
# do second change
#
activity="develop begin 200"
$bin/aegis -db 2 -dir $work/dd2
if test $? -ne 0 ; then no_result; fi
activity="copy file 203"
$bin/aegis -cp $work/dd2/aegis.conf
if test $? -ne 0 ; then no_result; fi
cat > $work/dd2/aegis.conf << 'fubar'
build_command = "exit 0";
history_create_command = "echo $history $input > /dev/null";
history_put_command = "echo $history $input > /dev/null";
history_get_command = "echo $history > $output";
history_query_command = "echo $history > /dev/null ; echo 1.2";
diff_command = "set +e; diff $orig $in > $out; test $? -le 1";
diff3_command = "echo diff3 $orig $mr $in > $out";
fubar
if test $? -ne 0 ; then no_result; fi
activity="build 216"
$bin/aegis -build -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="diff 219"
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="develop end 222"
$bin/aegis -de
if test $? -ne 0 ; then no_result; fi
activity="review pass 225"
$bin/aegis -rpass 2
if test $? -ne 0 ; then no_result; fi
activity="integrate begin 228"
$bin/aegis -ib 2
if test $? -ne 0 ; then no_result; fi
activity="build 231"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="integrate pass 234"
$bin/aegis -ipass -nl > errlog 2>&1
if test $? -ne 0 ; then cat errlog; no_result; fi

#
# now test using the delta names
#
activity="delta name 241"
$bin/aegis -dn 1 test
if test $? -ne 0 ; then fail; fi
activity="delta name 244"
$bin/aegis -dn 2 retest
if test $? -ne 0 ; then fail; fi
activity="delta name 247"
$bin/aegis -dn 1 test -ow
if test $? -ne 0 ; then fail; fi
activity="delta name 250"
$bin/aegis -dn 2 test > log 2>&1
if test $? -ne 1 ; then cat log; fail; fi

#
# make sure the listing works
#	the listing test is locale-specific
#	will need to change if fails too many places
#
activity="delta name 259"
$bin/aegis -l phi -tw 0 -pw=79 > test.out
if test $? -ne 0 ; then fail; fi
sed -e 's/[A-Z][a-z][a-z] [A-Z][a-z][a-z] [ 123][0-9]/XXX XXX XX/g' \
	-e 's/[012][0-9]:[0-5][0-9]:[0-5][0-9] [0-9][0-9][0-9][0-9]/XX:XX:XX XXXX/g' \
	< test.out > test.out2
if test $? -ne 0 ; then no_result; fi
cat > test.ok << 'fubar'



Project "foo"                                                            Page 1
History                                                XXX XXX XX XX:XX:XX XXXX

Delta   Date and Time   Change  Description
------- --------------- ------- -------------
Name: "test"
   1    XXX XXX XX         1    just an example
        XX:XX:XX XXXX

Name: "retest"
   2    XXX XXX XX         2    another example
        XX:XX:XX XXXX
fubar
if test $? -ne 0 ; then no_result; fi
diff test.ok test.out2
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
