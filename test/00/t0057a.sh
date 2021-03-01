#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996-1998, 2005-2008 Peter Miller
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
	echo 'NO RESULT for test of the throw/catch report functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo 'FAILED test of the throw/catch report functionality' 1>&2
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
# test the uncaught exception functionality
#
cat  > test.in << 'fubar'
title("throw catch test");
columns(80);
auto a, b;
a = 10;
b = 0;
print(a / b);
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
aegis: test.in: 6: division by zero
aegis: report aborted
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -rpt -f test.in -ter > test.out 2>&1
if test $? -ne 1 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the try/catch statement
#
cat  > test.in << 'fubar'
title("throw catch test");
columns(8, 8, 8);
auto a, b, c;
for (a = -2.; a <= 2; ++a)
{
	for (b = -2.; b <= 2; ++b)
	{
		try
			print(a, b, a/b);
		catch (c)
			print(a, b, "infinity");
	}
}
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
-2 -2 1
-2 -1 2
-2 0 infinity
-2 1 -2
-2 2 -1
-1 -2 0.5
-1 -1 1
-1 0 infinity
-1 1 -1
-1 2 -0.5
0 -2 0
0 -1 0
0 0 infinity
0 1 0
0 2 0
1 -2 -0.5
1 -1 -1
1 0 infinity
1 1 1
1 2 0.5
2 -2 -1
2 -1 -2
2 0 infinity
2 1 2
2 2 1
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -rpt -f test.in -ter -o test.out
if test $? -ne 0 ; then fail; fi

diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the throw statement
#
cat  > test.in << 'fubar'
title("throw catch test");
columns(80);
auto a;
try
{
	throw("boo boo");
	print("this should never happen");
}
catch (a)
	print(a);
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
boo boo
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -rpt -f test.in -ter -o test.out
if test $? -ne 0 ; then fail; fi

diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
