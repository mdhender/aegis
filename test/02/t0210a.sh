#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2005-2008 Peter Miller
#	Copyright (C) 2007 Walter Franzini
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
#	along with this program; if not, see
#	<http://www.gnu.org/licenses/>.
#

TEST_SUBJECT="aesvt checkout functionality"

# load up standard prelude and test functions
. test_funcs

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

aesvt -check-in -hist test,svt -f test.in1
test $? -eq 0 || fail

aesvt -check-in -history test,svt -f test.in2
test $? -eq 0 || fail

#
# Check also the gzip algorithm.  The bzip2 algorithm is already
# tested since it's the default.
#
aesvt -check-in -history test,svt -f test.in3 -comp-alg=gzip
test $? -eq 0 || fail

aesvt -check-out -history test,svt -e 2 -f - > test.out
test $? -eq 0 || fail

diff test.in2 test.out
test $? -eq 0 || fail

aesvt -check-out -history test,svt -f test.out
test $? -eq 0 || fail

diff test.in3 test.out
test $? -eq 0 || fail

aesvt -check-out -history test,svt -e 1 -f test.out
test $? -eq 0 || fail

diff test.in1 test.out
test $? -eq 0 || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
