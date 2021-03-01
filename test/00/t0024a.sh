#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994, 1995 Peter Miller;
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
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# MANIFEST: Test the report generator functionality
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$
PAGER=cat
export PAGER

AEGIS_FLAGS="delete_file_preference = no_keep; \
	diff_preference = automatic_merge;"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

LINES=66
export LINES
COLS=80
export COLS

here=`pwd`
if test $? -ne 0 ; then exit 1; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

fail()
{
	set +x
	echo 'FAILED test of the report generator functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
}
trap \"fail\" 1 2 3 15

mkdir $work $work/lib $work/lib/report
if test $? -ne 0 ; then fail; fi
cd $work
if test $? -ne 0 ; then fail; fi
chmod a+rwx $work/lib
if test $? -ne 0 ; then fail; fi

AEGIS_PATH=$work/lib
export AEGIS_PATH

#
# create project and changes
#
$bin/aegis -npr foo -dir $work/proj
if test $? -ne 0 ; then fail; fi
cat > $work/fred << 'fubar'
brief_description = "just an example";
cause = internal_bug;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -nc -f $work/fred -p foo
if test $? -ne 0 ; then fail; fi
cat > $work/fred << 'fubar'
brief_description = "another example";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -nc -f $work/fred -p foo
if test $? -ne 0 ; then fail; fi

#
# test do and while loops
#
cat > test.in << 'fubar'
title("Change State", "delete this line");
columns
(
	{ name = "Number\n------"; width = 6; },
	{ name = "Description\n-----------"; right = 0; }
);
auto ps, cn, cs;
ps = project["foo"].state;
for (cn in sort(keys(ps.change)))
{
	cs = ps.change[cn];
	print(sprintf("%4d", cs.change_number), cs.description);
}
fubar
if test $? -ne 0 ; then fail; fi
cat > test.ok << 'fubar'
1 just an example
2 another example
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -rpt -f test.in -o test.out -unf
if test $? -ne 0 ; then fail; fi
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
