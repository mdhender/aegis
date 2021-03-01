#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1993, 1994, 1995 Peter Miller;
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
# MANIFEST: Test the -Develop_Begin_Undo functionality
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

here=`pwd`
if test $? -ne 0 ; then exit 1; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

fail()
{
	set +x
	echo FAILED test of -Develop_Begin_Undo functionality 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	echo PASSED 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
}
trap "fail" 1 2 3 15

mkdir $work
if test $? -ne 0 ; then fail; fi
cd $work
if test $? -ne 0 ; then fail; fi

worklib=$work/lib
workproj=$work/foo
workchan=$work/bar

#
# new project
#
$bin/aegis -npr foo -lib $worklib -dir $workproj
if test $? -ne 0 ; then fail; fi

#
# new change
#
cat > ncf << 'fubar'
brief_description = "please hit me";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -nc -f ncf -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# make current user a developer
#
$bin/aegis -nd $USER -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# develop begin
#
$bin/aegis -db -lib $worklib -p foo -c 1 -dir $workchan
if test $? -ne 0 ; then fail; fi
if test ! -d $workchan ; then fail ; fi

#
# add a file just to spice it up a little
#
$bin/aegis -nf $workchan/snot -lib $worklib -p foo -c 1
if test $? -ne 0 ; then fail; fi

#
# develop begin undo
#
$bin/aegis -dbu -lib $worklib -p foo -c 1
if test $? -ne 0 ; then fail; fi
if test -d $workchan ; then fail ; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
