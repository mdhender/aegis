#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994, 1995 Peter Miller;
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
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# MANIFEST: Test the aegis -Delta_Name functionality
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
	diff_preference = automatic_merge;"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

LINES=66
export LINES
COLS=80
export COLS

here=`pwd`
if test $? -ne 0 ; then exit 1; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

fail()
{
	set +x
	echo 'FAILED test of the aegis -Delta_Name functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
}
trap \"fail\" 1 2 3 15

mkdir $work $work/lib $work/lib/report
if test $? -ne 0 ; then fail; fi
cd $work
if test $? -ne 0 ; then fail; fi
chmod a+rwx $work/lib
if test $? -ne 0 ; then fail; fi

AEGIS_PATH=$work/lib
export AEGIS_PATH
AEGIS_PROJECT=foo
export AEGIS_PROJECT

#
# create project and changes
#
$bin/aegis -npr foo -dir $work/proj
if test $? -ne 0 ; then fail; fi
$bin/aegis -nd $USER
if test $? -ne 0 ; then fail; fi
$bin/aegis -nrv $USER
if test $? -ne 0 ; then fail; fi
$bin/aegis -ni $USER
if test $? -ne 0 ; then fail; fi
cat > $work/fred << 'fubar'
brief_description = "just an example";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -nc -f $work/fred -p foo
if test $? -ne 0 ; then fail; fi
cat > $work/fred << 'fubar'
brief_description = "another example";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -nc -f $work/fred -p foo
if test $? -ne 0 ; then fail; fi
cat > $work/fred << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
developers_may_create_changes = true;
umask = 022;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -pa -f $work/fred
if test $? -ne 0 ; then fail; fi

#
# do first change
#
$bin/aegis -db 1 -dir $work/dd1
if test $? -ne 0 ; then fail; fi
$bin/aegis -nf $work/dd1/config
if test $? -ne 0 ; then fail; fi
cat > $work/dd1/config << 'fubar'
build_command = "exit 0";
history_create_command = "exit 0";
history_put_command = "exit 0";
history_get_command = "echo snot > $output";
history_query_command = "echo 1.1";
diff_command = "set +e; diff $orig $in > $out; test $? -le 1";
diff3_command = "echo diff3 $orig $mr $in > $out";
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -build -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -de
if test $? -ne 0 ; then fail; fi
$bin/aegis -rpass 1
if test $? -ne 0 ; then fail; fi
$bin/aegis -ib 1
if test $? -ne 0 ; then fail; fi
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -ipass -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# do second change
#
$bin/aegis -db 2 -dir $work/dd2
if test $? -ne 0 ; then fail; fi
$bin/aegis -cp $work/dd2/config
if test $? -ne 0 ; then fail; fi
cat > $work/dd2/config << 'fubar'
build_command = "exit 0";
history_create_command = "exit 0";
history_put_command = "exit 0";
history_get_command = "echo snot > $output";
history_query_command = "echo 1.2";
diff_command = "set +e; diff $orig $in > $out; test $? -le 1";
diff3_command = "echo diff3 $orig $mr $in > $out";
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -build -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -de
if test $? -ne 0 ; then fail; fi
$bin/aegis -rpass 2
if test $? -ne 0 ; then fail; fi
$bin/aegis -ib 2
if test $? -ne 0 ; then fail; fi
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -ipass -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# now test using the delta names
#
$bin/aegis -dn 1 test
if test $? -ne 0 ; then fail; fi
$bin/aegis -dn 2 retest
if test $? -ne 0 ; then fail; fi
$bin/aegis -dn 1 test -ow
if test $? -ne 0 ; then fail; fi
$bin/aegis -dn 2 test > /dev/null 2>&1
if test $? -ne 1 ; then fail; fi

#
# make sure the listing works
#	the listing test is locale-specific
#	will need to change if fails too many places
#
$bin/aegis -l phi -tw 0 > test.out
if test $? -ne 0 ; then fail; fi
sed -e 's/[A-Z][a-z][a-z] [A-Z][a-z][a-z] [ 123][0-9]/XXX XXX XX/g' \
	-e 's/[012][0-9]:[0-5][0-9]:[0-5][0-9] [0-9][0-9][0-9][0-9]/XX:XX:XX XXXX/g' \
	< test.out > test.out2
if test $? -ne 0 ; then fail; fi
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
if test $? -ne 0 ; then fail; fi
diff test.ok test.out2
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
