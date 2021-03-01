#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1998, 2005-2008 Peter Miller
#	Copyright (C) 2006 Walter Franzini;
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

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
unset LINES
unset COOK
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

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

COLS=65
export COLS
work=${AEGIS_TMP:-/tmp}/$$

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

no_result()
{
	set +x
	echo "NO RESULT for test of aeclean ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of aeclean ($activity)" 1>&2
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

#
# some variable to make things earier to read
#
worklib=$work/lib
workproj=$work/example.proj
workchan=$work/example.chan
tmp=$work/tmp

#
# make the directories
#
activity="working directory 96"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

AEGIS_PATH=$worklib
export AEGIS_PATH
PATH=$bin:$PATH
export PATH
AEGIS_PROJECT=example
export AEGIS_PROJECT

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# make a new project
#
activity="new project 122"
$bin/aegis -newpro example -version "" -dir $workproj -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# change project attributes
#
activity="project attributes 129"
cat > $tmp << 'TheEnd'
description = "aegis user's guide";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -proatt -f $tmp -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# create a new change
#
activity="new change 144"
cat > $tmp << 'TheEnd'
brief_description = "Place under aegis";
description = "A simple calculator using native floating point precision.  \
The four basic arithmetic operators to be provided, \
using conventional infix notation.  \
Parentheses and negation also required.";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -new_change 1 -f $tmp -project example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# add a new developer
#
activity="new developer 161"
$bin/aegis -newdev $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# begin development of the change
#
activity="develop begin 168"
$bin/aegis -devbeg 1 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the new files to the change
#
activity="new file 175"
$bin/aegis -new_file $workchan/aegis.conf -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/aegis.conf << 'fubar'
build_command = "exit 0";
history_create_command = "exit 0";
history_get_command = "exit 0";
history_put_command = "exit 0";
history_query_command = "exit 0";
diff_command = "exit 0";
merge_command = "exit 0";
clean_exceptions = [ "*.keep" ];
fubar
if test $? -ne 0 ; then cat log; no_result; fi

date > $workchan/junk
if test $? -ne 0 ; then no_result; fi

date > $workchan/garbage.keep
if test $? -ne 0 ; then no_result; fi

$bin/aegis -clean -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

test -r $workchan/junk && fail
test ! -r $workchan/garbage.keep && fail
test ! -r $workchan/aegis.conf && fail


#
# the things tested in this test, worked
# the things not tested in this test, may or may not work
#
pass
