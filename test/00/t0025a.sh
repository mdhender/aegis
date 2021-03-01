#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994-1998, 2004, 2006-2008 Peter Miller
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

TEST_SUBJECT="aechown functionality"

# load up standard prelude and test functions
. test_funcs

#
# test the aechown functionality
#
activity="new project 30"
aegis -npr foo -vers "" -dir $work/proj.dir -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=foo
export AEGIS_PROJECT

activity="new developer 37"
aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 41"
cat > eric << 'fubar'
brief_description = "the change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
aegis -nc 1 -f eric -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 50"
aegis -db 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 54"
aegis -nf foo.C001/xyzzy > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="chown 58"
aegis -chown -c 1 -u $USER > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

if test ! -f foo.D001/xyzzy ; then fail; fi

activity="chown to not developer 64"
aegis -chown -c 1 -u not$USER > log 2>&1
if test $? -ne 1; then cat log; fail; fi

cat > $work/ok <<EOF
aegis: project "foo": not$USER not developer
EOF
if test $? -ne 0; then no_result; fi

diff $work/ok log
if test $? -ne 0; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
