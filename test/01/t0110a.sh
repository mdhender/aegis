#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1999, 2006-2008 Peter Miller
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

bin=$here/${1-.}/bin

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
	echo "FAILED test of the aenpa functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aenpa functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="working directory 99"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi
tmp="$work/temp-file"

worklib=$work/lib
workchan=$work/change-dir

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
# test the aenpa functionality
#
activity="new project 129"
$bin/aegis -npr example -version '4.2' -lib $worklib -dir $work/proj -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create an alias
#
activity="new alias 136"
$bin/aegis -npa example.4.2 foo -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > ok << 'fubar'
where =
[
	{
		project_name = "example";
		directory = ".../proj";
	},
	{
		project_name = "foo";
		alias_for = "example.4.2";
	},
];
fubar
if test $? -ne 0 ; then cat log; no_result; fi

check_it ok $worklib/state

#
# go through the alias
#
activity="use alias 160"
$bin/aegis -pa -l -p foo -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# remove an alias
#
activity="remove alias 167"
$bin/aegis -rpa foo -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > ok << 'fubar'
where =
[
	{
		project_name = "example";
		directory = ".../proj";
	},
];
fubar
if test $? -ne 0 ; then cat log; no_result; fi

check_it ok $worklib/state

#
# the things tested in this test, worked
# the things not tested in this test, may or may not work
#
pass
