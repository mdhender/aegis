#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1999, 2005-2008 Peter Miller
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
umask 022

LINES=24
export LINES
COLS=80
export COLS

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

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
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
fail()
{
	set +x
	echo 'FAILED test of the aenc functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the aenc functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

AEGIS_PATH=$work/lib
export AEGIS_PATH

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the aenc functionality
#
$bin/aegis -npr junk -version 1.0 -directory $work/junk -lib $work/lib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > ca << 'fubar'
brief_description = "none";
cause = external_bug;
attribute =
[
    {
        name = "Attribute1";
        value = "Value1";
    },
];
fubar
if test $? -ne 0 ; then no_result; fi

activity="new change 136"
$bin/aegis -nc 1 -f ca -p junk.1.0 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="check the cstate file 140"
cat > ok <<EOF
brief_description = "none";
description = "none";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
attribute =
[
	{
		name = "Attribute1";
		value = "Value1";
	},
];
state = awaiting_development;
given_regression_test_exemption = true;
history =
[
	{
		when = TIME;
		what = new_change;
		who = "USER";
	},
];
EOF

check_it ok $work/junk/info/change/0/001.branch/0/000.branch/0/001

activity="new change undo 173"
$bin/aegis -ncu -l -p junk.1.0 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
