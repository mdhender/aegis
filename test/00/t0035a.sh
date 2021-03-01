#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996-2003, 2005-2008 Peter Miller
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
	echo "FAILED test of the zero'th change functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of the zero'th change functionality ($activity)" 1>&2
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

activity="create working directory 84"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

worklib="$work/lib"
workproj="$work/proj"
workchan="$work/chan"

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the zero'th change functionality
#
activity="new project 122"
$bin/aegis -npr foo -version "" -lib $worklib -dir $workproj
if test $? -ne 0 ; then no_result; fi

#
# add staff to the project
#
activity="add staff 129"
$bin/aegis -nd $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nrv $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi
$bin/aegis -ni $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# create a new branch
#
activity="new branch 140"
$bin/aegis -nbr 0 -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

activity="check branch state 144"
cat > ok << 'fubar'
brief_description = "The \"foo\" program.";
description = "The \"foo\" program.";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = false;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
state = being_developed;
development_directory = ".";
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
branch =
{
	umask = 022;
	developer_may_review = false;
	developer_may_integrate = false;
	reviewer_may_integrate = false;
	developers_may_create_changes = false;
	default_test_exemption = false;
	default_test_regression_exemption = true;
	skip_unlucky = false;
	compress_database = false;
	develop_end_action = goto_being_reviewed;
	change =
	[
		ZERO,
	];
	sub_branch =
	[
		ZERO,
	];
	administrator =
	[
		"USER",
	];
	developer =
	[
		"USER",
	];
	reviewer =
	[
		"USER",
	];
	integrator =
	[
		"USER",
	];
	minimum_change_number = 10;
	reuse_change_numbers = true;
	minimum_branch_number = 1;
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $workproj/info/trunk

activity="check branch state 209"
cat > ok << 'fubar'
brief_description = "The \"foo\" program, branch 0.";
description = "The \"foo\" program, branch 0.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
state = being_developed;
development_directory = "branch.0";
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
branch =
{
	umask = 022;
	developer_may_review = false;
	developer_may_integrate = false;
	reviewer_may_integrate = false;
	developers_may_create_changes = false;
	default_test_exemption = false;
	default_test_regression_exemption = true;
	skip_unlucky = false;
	compress_database = false;
	develop_end_action = goto_being_reviewed;
	change =
	[
	];
	administrator =
	[
		"USER",
	];
	developer =
	[
		"USER",
	];
	reviewer =
	[
		"USER",
	];
	integrator =
	[
		"USER",
	];
	minimum_change_number = 10;
	reuse_change_numbers = true;
	minimum_branch_number = 1;
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $workproj/info/change/0/000

#
# create a zero change on the zero branch
#
activity="new change 272"
cat > ncf << 'fubar'
brief_description = "nothing";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 0 -file ncf -lib $worklib -p foo.0
if test $? -ne 0 ; then fail; fi

#
# develop begin
#
$bin/aegis -db 0 -lib $worklib -p foo.0 -dir $workchan
if test $? -ne 0 ; then fail; fi

#
# make sure the state files look like we expect
#
activity="check change state 290"
cat > ok << 'fubar'
brief_description = "nothing";
description = "nothing";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
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
check_it ok $workproj/info/change/0/000.branch/0/000

activity="check project state 321"
cat > ok << 'fubar'
brief_description = "The \"foo\" program, branch 0.";
description = "The \"foo\" program, branch 0.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
state = being_developed;
development_directory = "branch.0";
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
branch =
{
	umask = 022;
	developer_may_review = false;
	developer_may_integrate = false;
	reviewer_may_integrate = false;
	developers_may_create_changes = false;
	default_test_exemption = false;
	default_test_regression_exemption = true;
	skip_unlucky = false;
	compress_database = false;
	develop_end_action = goto_being_reviewed;
	change =
	[
		ZERO,
	];
	administrator =
	[
		"USER",
	];
	developer =
	[
		"USER",
	];
	reviewer =
	[
		"USER",
	];
	integrator =
	[
		"USER",
	];
	minimum_change_number = 10;
	reuse_change_numbers = true;
	minimum_branch_number = 1;
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $workproj/info/change/0/000

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
