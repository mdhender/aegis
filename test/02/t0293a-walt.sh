#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 2010 Walter Franzini
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

TEST_SUBJECT="aesub \$version"

# load up standard prelude and test functions
. test_funcs

#
# test the aesub functionality
#
activity="new project 129"
aegis -npr example.1.0 -version '' -lib $work/lib -dir $work/proj -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Check the version
#
activity="check version"
cat > ok <<EOF
1.C000
EOF
test $? -eq 0 || no_result

aesub -p example.1 -c 0 '$version' > result
test $? -eq 0 || no_result

diff ok result
test $? -eq 0 || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
