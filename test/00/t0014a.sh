#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1993-1998, 2006, 2007 Peter Miller
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
# MANIFEST: Test the -Develop_Begin_Undo functionality
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
unset LINES
unset COLS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$
PAGER=cat
export PAGER

AEGIS_FLAGS="delete_file_preference = no_keep; \
	lock_wait_preference = always; \
	diff_preference = automatic_merge; \
	pager_preference = never; \
	persevere_preference = all; \
	log_file_preference = never;"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

no_result()
{
	set +x
	echo "NO RESULT for test of -Develop_Begin_Undo functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of -Develop_Begin_Undo functionality ($activity)" 1>&2
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
trap "no_result" 1 2 3 15

activity="working directory 70"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

worklib=$work/lib
workproj=$work/foo
workchan=$work/bar

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# new project
#
activity="new project 107"
$bin/aegis -npr foo -version "" -lib $worklib -dir $workproj
if test $? -ne 0 ; then no_result; fi

#
# new change
#
activity="new change 114"
cat > ncf << 'fubar'
brief_description = "please hit me";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f ncf -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi

#
# make current user a developer
#
activity="new developer 126"
$bin/aegis -nd $USER -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi

#
# develop begin
#
activity="develop begin 133"
$bin/aegis -db -lib $worklib -p foo -c 1 -dir $workchan
if test $? -ne 0 ; then no_result; fi
if test ! -d $workchan ; then no_result ; fi

#
# add a file just to spice it up a little
#
activity="new file 141"
$bin/aegis -nf $workchan/snot -lib $worklib -p foo -c 1
if test $? -ne 0 ; then no_result; fi

#
# develop begin undo
#
activity="develop begin undo 148"
$bin/aegis -dbu -lib $worklib -p foo -c 1
if test $? -ne 0 ; then fail; fi
if test -d $workchan ; then fail ; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
