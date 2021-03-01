#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2005 Peter Miller;
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
# MANIFEST: Test the aesvt meta-data functionality
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
	log_file_preference = never; \
	default_development_directory = \"$work\";"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

# This tells aeintegratq that it is being used by a test.
AEGIS_TEST_DIR=$work
export AEGIS_TEST_DIR

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

AEGIS_DATADIR=$here/lib
export AEGIS_DATADIR

#
# set the path, so that the aegis command that aepatch/aedist invokes
# is from the same test set as the aepatch/aedist command itself.
#
PATH=${bin}:$PATH
export PATH

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
	echo "FAILED test of the aesvt meta-data functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aesvt meta-data functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory"
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

cat > test.in1 << 'fubar'
line one
line two
line three
line four
line five
fubar
test $? -eq 0 || no_result

cat > test.in2 << 'fubar'
line two
line three
line four
line four and a half
line five
line six
line seven
fubar
test $? -eq 0 || no_result

cat > test.in3 << 'fubar'
line two
line three
line four
line five
line six
line seven
line eight
fubar
test $? -eq 0 || no_result

cat > test.ok << 'fubar'
Checksum: 2454591773
Content-Length: 71
Content-Transfer-Encoding: 8bit
Content-Type: text/plain; charset=us-ascii
Date: the-date
User: somebody else
Version: 4.2

Checksum: 2550537303
Content-Length: 81
Content-Transfer-Encoding: 8bit
Content-Type: text/plain; charset=us-ascii
Date: the-date
Remark: nothing much
User: the-user
Version: penguin61

Checksum: 3027046709
Content-Length: 49
Content-Transfer-Encoding: 8bit
Content-Type: text/plain; charset=us-ascii
Date: a while back
User: the-user
Version: penguin060
fubar
test $? -eq 0 || no_result

$bin/aesvt -check-in -hist test,svt -f test.in1 \
    version=penguin060 date='a while back'
test $? -eq 0 || fail

$bin/aesvt -check-in -hist test,svt -f test.in2 \
    remark='nothing much'
test $? -eq 0 || fail

$bin/aesvt -check-in -hist test,svt -f test.in3 \
    version=4.2 user='somebody else'
test $? -eq 0 || fail

$bin/aesvt -hist test,svt -l > test.out.raw
test $? -eq 0 || fail

sed -e "s|User: $USER|User: the-user|" \
    -e "s|User: uid:.*|User: the-user|" \
    -e 's|Date: ..., .. ... .... ..:..:.*|Date: the-date|' \
    test.out.raw > test.out
test $? -eq 0 || no_result

diff test.ok test.out
test $? -eq 0 || fail

#
# The things tested here, worked.
# No other guarantees are made.
#
pass

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
