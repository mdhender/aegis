#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994-1999, 2006-2008 Peter Miller
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
unset COLS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

no_result()
{
	set +x
	echo "NO RESULT for test of aedist -send -baseline functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of aedist -send -baseline functionality ($activity)" 1>&2
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

worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT

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

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

activity="text, no headers, line 112"

cat > test.ae << 'fubar'
H4sIAAAAAAAAA5VSwWrDMAztNf4Kt+eljZtBB6WHHXYY7FR6GyMorpJ6pE6wlW0w9u+z17Q0
poPEGISkJ/k9S8kqWSWCn86ffRBw3/lJYEXg5//gAisudUhy0Zj6HSXFGo44YYClsvN0LgRj
yTWX5SguYrkfxoUX11zkAXSJsUWaMMZyo7DI9milUQ2pWvMNn9FBWe4ucEJLszUL8rtenp86
ctdx7rASWosOpTSh0VBleVuumQdm+IXHhlyOTItdLAeLldIYJg2WBq11L2a3ai0B+VfgExQp
XToJH1jVzRE1uayRfMNeWfTNoqhQFWb+58/SYnch7qRFEchOlzTomvpQa6H0cFu3RvrIzx17
W7vv6s8rHbk7xcDdSc++07HoM/Yz648n4JSM5DTUXvZ+t318fnnaTqdTz+UX2SjIU0wDAAA=
fubar
test "$?" -eq 0 || no_result

$bin/aedist -l -f test.ae -o test.out
test "$?" -eq 0 || fail

#
# the things tested in this test, worked
#
pass
