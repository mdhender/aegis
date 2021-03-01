#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996-1998, 2000, 2002-2008 Peter Miller
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
	echo "FAILED test of the aenfu functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of the aenfu functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

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

activity="working directory 96"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

workchan=$work/chan
workproj=$work/proj
worklib=$work/lib
tmp=$work/tmp

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
activity="new project 120"
$bin/aegis -newpro foo -version "" -dir $workproj -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# change project attributes
#
activity="project attributes 127"
cat > $tmp << 'end'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -proatt -f $tmp -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# create a new change
#
activity="new change 140"
cat > $tmp << 'end'
brief_description = "one";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_change 1 -f $tmp -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add a new developer
#
activity="new developer 152"
$bin/aegis -newdev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# begin development of a change
#
activity="develop begin 159"
$bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add a new files to the change
#
activity="new file 166"
$bin/aegis -new_file $workchan/aegis.conf $workchan/main.c -nl -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
history_get_command = "exit 0";
history_create_command = "exit 0";
history_put_command = "exit 0";
history_query_command = "echo HAHA";
diff_command =  "exit 0";
diff3_command =  "exit 0";
end
if test $? -ne 0 ; then no_result; fi

#
# new file undo
#
activity="new file undo 184"
$bin/aegis -nfu $workchan/main.c -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# check the change state
#
activity="check change state 191"
cat > ok << 'fubar'
brief_description = "one";
description = "one";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
copyright_years =
[
	YYYY,
];
state = being_developed;
given_regression_test_exemption = true;
development_directory = ".../chan";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "USER";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "USER";
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $workproj/info/change/0/001

activity="check change file state 227"
cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		action = create;
		usage = config;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $workproj/info/change/0/001.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
