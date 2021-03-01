#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2001, 2005-2008 Peter Miller
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
	echo 'FAILED test of the uuencode functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the uuencode functionality' 1>&2
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

# --------------------------------------------------------------------------
#
# test the uuencode functionality
#
cat > test.in << 'fubar'
This is a test with several lines in it, because I wanted more than 64
characters in the input.
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
Content-Type: application/x-aegis-test
Content-Transfer-Encoding: uuencode

begin 644 test.out
M5&AI<R!I<R!A('1E<W0@=VET:"!S979E<F%L(&QI;F5S(&EN(&ET+"!B96-A
M=7-E($D@=V%N=&5D(&UO<F4@=&AA;B`V-`IC:&%R86-T97)S(&EN('1H92!I
&;G!U="X*
`
end
fubar
if test $? -ne 0 ; then no_result; fi

$bin/test_base64 -uu -o test.in test.out
if test $? -ne 0 ; then fail; fi

diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

# --------------------------------------------------------------------------
#
# test the uudecode functionality
#
mv test.in test.ok
if test $? -ne 0 ; then no_result; fi

mv test.out test.in
if test $? -ne 0 ; then no_result; fi

$bin/test_base64 -uu -i test.in test.out
if test $? -ne 0 ; then fail; fi

diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
