#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2005, 2006, 2008 Peter Miller
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
here=`pwd`
test $? -eq 0 || exit 2
work=${TMP_DIR-/tmp}/$$

tear_down()
{
	cd $here
	rm -rf $work
}

pass()
{
	tear_down
	echo PASSED
	exit 0
}

fail()
{
	tear_down
	echo 'FAILED test of the aesvt checkin functionality'
	exit 1
}

no_result()
{
	tear_down
	echo 'NO RESULT for test of the aesvt checkin functionality'
	exit 2
}

trap "no_result" 1 2 3 15

bin=$here/${1-.}/bin
mkdir $work $work/var $work/etc
test $? -eq 0 || no_result
cd $work
test $? -eq 0 || no_result

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
Version: 3

line two
line three
line four
line five
line six
line seven
line eight
Checksum: 2550537303
Content-Length: 81
Content-Transfer-Encoding: 8bit
Content-Type: text/plain; charset=us-ascii
Version: 2

line two
line three
line four
line four and a half
line five
line six
line seven
Checksum: 3027046709
Content-Length: 49
Content-Transfer-Encoding: 8bit
Content-Type: text/plain; charset=us-ascii
Version: 1

line one
line two
line three
line four
line five
fubar
test $? -eq 0 || no_result

# this tests the case where there is no existing history file
$bin/aesvt -check-in -history test,svt -f test.in1
test $? -eq 0 || fail

# this tests the case where there is an existing history file
$bin/aesvt -check-in -history test,svt -f test.in2
test $? -eq 0 || fail

# this tests the case where there is an existing history file
$bin/aesvt -check-in --compression-algorithm=gzip -history test,svt -f test.in3
test $? -eq 0 || fail

gunzip < test,svt > test.out.messy
test $? -eq 0 || fail

sed -e '/Date:/d' -e '/User:/d' test.out.messy > test.out
test $? -eq 0 || no_result

diff test.ok test.out
test $? -eq 0 || fail

#
# The things tested here, worked.
# No other guarantees are made.
#
pass
