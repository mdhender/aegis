#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2001-2008 Peter Miller
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

if sccs -V > /dev/null 2>&1
then
	:
else
	echo ''
	echo '  The "sccs" program is not in your command search PATH.'
	echo '  This test is therefore -assumed- to pass.'
	echo ''
	exit 0
fi

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

#
# Make sure the command search path includes the bin directory
# from the development directory and all parent and ancestor branches.
# (Must find ae-sccs-put on the path somewhere.)
#
BIN=
IFSold="$IFS"
IFS=":$IFS"
for d in ${AEGIS_SEARCH_PATH:-$here}
do
	BIN="${BIN}$d/${1-.}/bin:"
done
IFS="$IFSold"
PATH=${BIN}$PATH
export PATH

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
                -e 's/uuid = ".*"/uuid = "UUID"/' \
                -e 's/delta_uuid = ".*"/delta_uuid = "UUID"/' \
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
	echo "FAILED test of the aeimport sccs functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aeimport sccs functionality " \
            "($activity)" 1>&2
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

workproj=$work/example
worklib=$work/lib

AEGIS_PATH=$worklib
export AEGIS_PATH

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the aeimport sccs functionality
#
mkdir src src/sub
if test $? -ne 0 ; then no_result; fi

activity="create sccs file 156"
$bin/test_base64 -qp -i - src/s.file1 << 'fubar'
=01h10874
=01s 00001/00000/00001
=01d D 1.2 99/03/12 06:17:16 bogus2 2 1
=01c second
=01e
=01s 00001/00000/00000
=01d D 1.1 99/03/09 06:51:24 bogus1 1 0
=01c first
=01e
=01u
=01U
=01f e 0
=01t
=01T
=01I 1
=01I 2
=01lah
=01E 2
This is file 1.
=01E 1
fubar
if test $? -ne 0 ; then no_result; fi

activity="create sccs file 181"
$bin/test_base64 -qp -i - src/sub/s.file2 << 'fubar'
=01h11284
=01s 00001/00000/00001
=01d D 1.2 99/03/12 07:00:00 bogus1 2 1
=01c third
=01e
=01s 00001/00000/00000
=01d D 1.1 99/03/12 06:17:16 bogus2 1 0
=01c second
=01e
=01u
=01U
=01f e 0
=01t
=01T
=01I 1
=01I 2
blah blah
=01E 2
This is file 2.
=01E 1
fubar
if test $? -ne 0 ; then no_result; fi

#
# now that all the RCS files exist, read it all in and fake the changes
#
activity="import 209"
$bin/aeimport --format=sccs src -p example -dir $workproj -lib $worklib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# Make sure the various state files are correct.
#
activity="check the state file 216"
cat > ok << 'fubar'
next_test_number = 1;
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/state

activity="check cstate 224"
cat > ok << 'fubar'
brief_description = "The \"example\" program, branch 1.0.";
description = "The \"example\" program, branch 1.0.";
cause = internal_enhancement;
test_exempt = true;
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
build_time = TIME;
development_directory = "branch.1/branch.0";
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
	developer_may_integrate = true;
	reviewer_may_integrate = true;
	developers_may_create_changes = true;
	default_test_exemption = true;
	default_test_regression_exemption = true;
	skip_unlucky = false;
	compress_database = false;
	develop_end_action = goto_being_reviewed;
	history =
	[
		{
			delta_number = 1;
			change_number = 10;
		},
		{
			delta_number = 2;
			change_number = 11;
		},
		{
			delta_number = 3;
			change_number = 12;
		},
		{
			delta_number = 4;
			change_number = 13;
		},
	];
	change =
	[
		10,
		11,
		12,
		13,
	];
	administrator =
	[
		"USER",
	];
	developer =
	[
		"bogus1",
		"bogus2",
	];
	reviewer =
	[
		"bogus1",
		"bogus2",
	];
	integrator =
	[
		"bogus1",
		"bogus2",
	];
	minimum_change_number = 10;
	reuse_change_numbers = true;
	minimum_branch_number = 1;
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/001.branch/0/000

activity="check fstate 322"
cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = config;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "file1";
		action = create;
		edit =
		{
			revision = "1.2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "sub/file2";
		action = create;
		edit =
		{
			revision = "1.2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
];
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/001.branch/0/000.fs

activity="check cstate 413"
cat > ok << 'fubar'
brief_description = "aegis.conf file";
description = "Initial project `aegis.conf' file.";
cause = internal_enhancement;
test_exempt = true;
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
state = completed;
delta_number = 1;
delta_uuid = "UUID";
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
	{
		when = TIME;
		what = develop_end;
		who = "USER";
	},
	{
		when = TIME;
		what = review_pass;
		who = "USER";
	},
	{
		when = TIME;
		what = integrate_begin;
		who = "USER";
	},
	{
		when = TIME;
		what = integrate_pass;
		who = "USER";
	},
];
uuid = "UUID";
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/001.branch/0/000.branch/0/010

activity="check fstate 471"
cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = config;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/001.branch/0/000.branch/0/010.fs

activity="check cstate 491"
cat > ok << 'fubar'
brief_description = "first";
description = "first";
cause = internal_enhancement;
test_exempt = true;
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
state = completed;
delta_number = 2;
delta_uuid = "UUID";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "bogus1";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "bogus1";
	},
	{
		when = TIME;
		what = develop_end;
		who = "bogus1";
	},
	{
		when = TIME;
		what = review_pass;
		who = "bogus1";
	},
	{
		when = TIME;
		what = integrate_begin;
		who = "bogus1";
	},
	{
		when = TIME;
		what = integrate_pass;
		who = "bogus1";
	},
];
uuid = "UUID";
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/001.branch/0/000.branch/0/011

activity="check fstate 549"
cat > ok << 'fubar'
src =
[
	{
		file_name = "file1";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/001.branch/0/000.branch/0/011.fs

activity="check cstate 569"
cat > ok << 'fubar'
brief_description = "second";
description = "second";
cause = internal_enhancement;
test_exempt = true;
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
state = completed;
delta_number = 3;
delta_uuid = "UUID";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "bogus2";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "bogus2";
	},
	{
		when = TIME;
		what = develop_end;
		who = "bogus2";
	},
	{
		when = TIME;
		what = review_pass;
		who = "bogus2";
	},
	{
		when = TIME;
		what = integrate_begin;
		who = "bogus2";
	},
	{
		when = TIME;
		what = integrate_pass;
		who = "bogus2";
	},
];
uuid = "UUID";
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/001.branch/0/000.branch/0/012

activity="check fstate 627"
cat > ok << 'fubar'
src =
[
	{
		file_name = "file1";
		action = modify;
		edit =
		{
			revision = "1.2";
			encoding = none;
		};
		usage = source;
	},
	{
		file_name = "sub/file2";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/001.branch/0/000.branch/0/012.fs

activity="check cstate 657"
cat > ok << 'fubar'
brief_description = "third";
description = "third";
cause = internal_enhancement;
test_exempt = true;
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
state = completed;
delta_number = 4;
delta_uuid = "UUID";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "bogus1";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "bogus1";
	},
	{
		when = TIME;
		what = develop_end;
		who = "bogus1";
	},
	{
		when = TIME;
		what = review_pass;
		who = "bogus1";
	},
	{
		when = TIME;
		what = integrate_begin;
		who = "bogus1";
	},
	{
		when = TIME;
		what = integrate_pass;
		who = "bogus1";
	},
];
uuid = "UUID";
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/001.branch/0/000.branch/0/013

activity="check fstate 715"
cat > ok << 'fubar'
src =
[
	{
		file_name = "sub/file2";
		action = modify;
		edit =
		{
			revision = "1.2";
			encoding = none;
		};
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/001.branch/0/000.branch/0/013.fs

activity="check file 735"
cat > ok << 'fubar'
blah
This is file 1.
fubar
if test $? -ne 0 ; then no_result; fi

diff ok $workproj/branch.1/branch.0/baseline/file1
if test $? -ne 0 ; then fail; fi

activity="check file 745"
cat > ok << 'fubar'
blah blah
This is file 2.
fubar
if test $? -ne 0 ; then no_result; fi

diff ok $workproj/branch.1/branch.0/baseline/sub/file2
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
