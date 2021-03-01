#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2007, 2008 Peter Miller
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

TEST_SUBJECT="shared_ptr functionality"

# load up standard prelude and test functions
. test_funcs

#
# test the shared_ptr functionality
#
activity="test basic shared pointers"
cat > test.ok << 'fubar'
constructor (4)
constructor (1)
constructor (2)
constructor (3)
destructor (1)
destructor (2)
destructor (3)
destructor (4)
fubar
if test $? -ne 0 ; then no_result; fi

test_shared_ptr > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
