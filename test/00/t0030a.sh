#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1995-2002, 2005-2008 Peter Miller
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

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi


no_result()
{
	set +x
	echo "NO RESULT for test of the project format conversion functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of the project format conversion functionality ($activity)" 1>&2
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
trap \"no_result\" 1 2 3 15

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		< $1 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $2 $work/sed.out
	if test $? -ne 0; then fail; fi
}

activity="create working directory 83"
mkdir $work
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

workproj=$work/proj
worklib=$work/lib

#
# test the project format conversion functionality
#
# Construct a project out of the blue, rather than through aegis,
# then do an aenpr -keep to get things going.
# The actual conversion is accomplished using aepa.
#
activity="create old-style project 99"
$bin/aegis -npr foo -vers "" -lib $worklib -dir $workproj
if test $? -ne 0 ; then no_result; fi

cat > $workproj/info/state << fubar
description = "project change supervisor.";
owner_name = "$USER";
group_name = "other";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
developers_may_create_changes = true;
develop_end_notify_command = "sh \${source lib/de.sh} \$project \$change";
develop_end_undo_notify_command = "sh \${source lib/deu.sh} \$project \$change \$developer";
review_pass_notify_command = "sh \${source lib/rp.sh} \$project \$change \$developer \$reviewer";
review_pass_undo_notify_command = "sh \${source lib/rpu.sh} \$project \$change \$developer \$reviewer";
review_fail_notify_command = "sh \${source lib/rf.sh} \$project \$change \$developer \$reviewer";
integrate_pass_notify_command = "sh \${source lib/ip.sh} \$project \$change \$developer \$reviewer \$integrator";
integrate_fail_notify_command = "sh \${source lib/if.sh} \$project \$change \$developer \$reviewer \$integrator";
umask = 022;
default_test_exemption = true;
copyright_years = [ 1991, 1992, 1993, 1994, 1995, ];
next_change_number = 30;
next_delta_number = 14;
next_test_number = 31;
src =
[
	{
		file_name = "config";
		usage = source;
		edit_number = "3";
	},
	{
		file_name = "main.c";
		usage = build;
		edit_number = "2";
	},
];
history =
[
	{ delta_number = 1; change_number = 1; }, { delta_number = 2;
	change_number = 2; }, { delta_number = 5; change_number = 15; },
	{ delta_number = 6; change_number = 12; }, { delta_number = 8;
	change_number = 14; }, { delta_number = 9; change_number = 16;
	}, { delta_number = 10; change_number = 4; }, { delta_number =
	11; change_number = 19; }, { delta_number = 12; change_number =
	20; }, { delta_number = 13; change_number = 21; },
];
change = [ 1, 2, 4, 12, 14, 15, 16, 19, 20, 21 ];
administrator = [ "$USER", ];
developer = [ "$USER", ];
reviewer = [ "$USER", ];
integrator = [ "$USER", ];
version_major = 3;
version_minor = 42;
version_previous = "2.3.D033";
fubar
if test $? -ne 0 ; then no_result; fi

activity="convert project 161"
cat > paf << 'fubar'
description = "some other description";
fubar
$bin/aegis -pa -f paf -p foo -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# check that the various state files exist,
# and are in the correct format
#
activity="check file formats 172"
cat > ok << 'fubar'
next_test_number = 31;
version_major = 3;
version_minor = 42;
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/state ok
cat > ok << 'fubar'
brief_description = "some other description";
description = "some other description";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = false;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
copyright_years =
[
	YYYY,
	YYYY,
	YYYY,
	YYYY,
	YYYY,
];
version_previous = "2.3.D033";
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
	developer_may_review = true;
	developer_may_integrate = true;
	reviewer_may_integrate = true;
	developers_may_create_changes = true;
	develop_end_notify_command = "sh ${source lib/de.sh} $project $change";
	develop_end_undo_notify_command = "sh ${source lib/deu.sh} $project $change $developer";
	review_pass_notify_command = "sh ${source lib/rp.sh} $project $change $developer $reviewer";
	review_pass_undo_notify_command = "sh ${source lib/rpu.sh} $project $change $developer $reviewer";
	review_fail_notify_command = "sh ${source lib/rf.sh} $project $change $developer $reviewer";
	integrate_pass_notify_command = "sh ${source lib/ip.sh} $project $change $developer $reviewer $integrator";
	integrate_fail_notify_command = "sh ${source lib/if.sh} $project $change $developer $reviewer $integrator";
	default_test_exemption = true;
	default_test_regression_exemption = true;
	skip_unlucky = false;
	compress_database = false;
	develop_end_action = goto_being_reviewed;
	history =
	[
		{
			delta_number = 1;
			change_number = 1;
		},
		{
			delta_number = 2;
			change_number = 2;
		},
		{
			delta_number = 5;
			change_number = 15;
		},
		{
			delta_number = 6;
			change_number = 12;
		},
		{
			delta_number = 8;
			change_number = 14;
		},
		{
			delta_number = 9;
			change_number = 16;
		},
		{
			delta_number = 10;
			change_number = 4;
		},
		{
			delta_number = 11;
			change_number = 19;
		},
		{
			delta_number = 12;
			change_number = 20;
		},
		{
			delta_number = 13;
			change_number = 21;
		},
	];
	change =
	[
		1,
		2,
		4,
		12,
		14,
		15,
		16,
		19,
		20,
		21,
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
	reuse_change_numbers = true;
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/trunk ok
cat > ok << 'fubar'
src =
[
	{
		file_name = "config";
		action = create;
		edit =
		{
			revision = "3";
			encoding = none;
		};
		edit_origin =
		{
			revision = "3";
			encoding = none;
		};
		usage = source;
	},
	{
		file_name = "main.c";
		action = create;
		edit =
		{
			revision = "2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "2";
			encoding = none;
		};
		usage = build;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/trunk.fs ok

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
