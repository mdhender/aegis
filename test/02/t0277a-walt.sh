#!/bin/sh
#
# aegis - The "aegis" program.
# Copyright (C) 2008 Walter Franzini
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.
#

TEST_SUBJECT="aesub vs. -bl"

# load up standard prelude and test functions
. test_funcs

#
# test the functionality
#
activity="new project 47"
aegis -newpro foo -version 1.2.3 -dir $work/proj -lib $work/lib
if test $? -ne 0 ; then fail; fi

cat > ok <<EOF
foo.1.2.3
EOF
test $? -eq 0 || no_result

aesub -proj foo.1.2.3 -bl '${project}' > out
test $? -eq 0 || no_result

diff ok out
test $? -eq 0 || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
