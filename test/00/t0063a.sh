#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1997-1999, 2004, 2006-2008 Peter Miller
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

LINES=25
export LINES
COLS=80
export COLS

#
# The working days calculations depend on the results of the localtime()
# function, so the timezone will affect the results.  By setting TZ we
# get consistent behaviour.
#
TZ=GMT0
export TZ

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

no_result()
{
	set +x
	echo 'NO_RESULT test of the working_days functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo 'FAILED test of the working_days functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	echo PASSED 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
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
# test the working_days functionality
#
# There are no timezones specified, and it is not near a DLS change, so
# it should (I hope) always work.
#
cat > test.in << 'fubar'
columns({width=15;}, {width=15;}, {width=0;});
auto a, b, incr;
a = "22-Nov-1995 10:47:01";
b =  "6-Dec-1995 19:21:49";
print(a, b, sprintf("%5.3f", working_days(mktime(a), mktime(b))));
print();
a = "22-Nov-1995 11:56:23";
b =  "6-Dec-1995 19:21:49";
print(a, b, sprintf("%5.3f", working_days(mktime(a), mktime(b))));
fubar
if test $? -ne 0 ; then fail; fi

cat > test.ok << 'fubar'
22-Nov-1995	6-Dec-1995	11.000
10:47:01	19:21:49

22-Nov-1995	6-Dec-1995	10.990
11:56:23	19:21:49
fubar
if test $? -ne 0 ; then fail; fi

$bin/aegis -rpt -f test.in -o test.out -pw=79 -pl=24
if test $? -ne 0 ; then fail; fi

sed '1,4d' < test.out > test.out2
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out2
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
