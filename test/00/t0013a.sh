#! /bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1993 Peter Miller.
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
# MANIFEST: Test the -New_Change_Undo functionality
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE

work=${AEGIS_TMP-/tmp}/$$
PAGER=cat
export PAGER

here=`pwd`
if test $? -ne 0 ; then exit 1; fi

fail()
{
	set +x
	cd $here
	echo FAILED test of -New_Change_Undo functionality 1>&2
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	cd $here
	echo PASSED 1>&2
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
}
trap "fail" 1 2 3 15

mkdir $work
if test $? -ne 0 ; then exit 1; fi
cd $work
if test $? -ne 0 ; then fail; fi

worklib=$work/lib
workproj=$work/foo

#
# create the project
#
$here/bin/aegis -npr foo -lib $worklib -dir $workproj
if test $? -ne 0 ; then fail; fi

#
# create a change
#
cat > ncf << 'fubar'
brief_description = "hello";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -nc ncf -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -f $workproj/info/change/0/001; then fail; fi

#
# remove the change
#
$here/bin/aegis -ncu -lib $worklib -p foo -c 1
if test $? -ne 0 ; then fail; fi
if test -f $workproj/info/change/0/001; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
