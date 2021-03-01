#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2002-2005 Peter Miller;
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
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: Test the aeimport functionality
#

rcs -V > /dev/null 2>&1
if test $? -ne 0
then
    echo
    echo '	Your system does not appear to have RCS installed.'
    echo '	This test is therefore -assumed- to pass.'
    echo
    exit 0
fi

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
AEGIS_THROTTLE=2
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
	echo "FAILED test of the aeimport functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aeimport functionality " \
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
# test the aeimport functionality
#
mkdir src src/sub src/Attic
if test $? -ne 0 ; then no_result; fi

activity="create rcs file 129"
cat > src/file1,v << 'fubar'
head	1.2;
access;
symbols;
locks;
strict;
comment	@# @;


1.2
date	99.03.12.06.17.16;	author bogus2;	state Exp;
branches;
next	1.1;

1.1
date	99.03.09.06.51.24;	author bogus1;	state Exp;
branches;
next	;


desc
@@


1.2
log
@second
@
text
@This is file1
@


1.1
log
@first
@
text
@This is actually broken, but, hey,
it will not be accessed by the test.
@
fubar
if test $? -ne 0 ; then no_result; fi

activity="create rcs file 174"
cat > src/sub/file2,v << 'fubar'
head	1.2;
access;
symbols
	sometag:1.2
	;
locks;
strict;
comment	@# @;


1.2
date	99.03.12.07.00.00;	author bogus1;	state Exp;
branches;
next	1.1;

1.1
date	99.03.12.06.17.16;	author bogus2;	state Exp;
branches;
next	;


desc
@@


1.2
log
@third
@
text
@This is file2
@


1.1
log
@second
@
text
@This is actually broken, but, hey,
it will not be accessed by the test.
@
fubar
if test $? -ne 0 ; then no_result; fi

activity="create rcs file 221"
cat > src/Attic/file3,v << 'fubar'
head	1.2;
access;
symbols;
locks;
strict;
comment	@# @;


1.2
date	99.03.12.06.17.16;	author bogus2;	state dead;
branches;
next	1.1;

1.1
date	99.03.09.06.51.24;	author bogus1;	state Exp;
branches;
next	;


desc
@@


1.2
log
@@
text
@This is actually broken, but, hey,
it will not be accessed by the test.
@


1.1
log
@@
text
@This is actually broken, but, hey,
it will not be accessed by the test.
@
fubar
if test $? -ne 0 ; then no_result; fi

#
# now that all the RCS files exist, read it all in and fake the changes
#
activity="import 268"
$bin/aeimport src -p example -dir $workproj -lib $worklib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# Make sure the various state files are correct.
#
activity="check state 275"
cat > ok << 'fubar'
next_test_number = 1;
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/state

activity="check cstate 283"
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
			name =
			[
				"sometag",
			];
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

activity="check fstate 385"
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
		file_name = "file3";
		action = remove;
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
		usage = source;
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		deleted_by = 12;
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

activity="check cstate 498"
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

activity="check fstate 556"
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

activity="check cstate 576"
cat > ok << 'fubar'
brief_description = "first";
description = "first\n\
";
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

activity="check fstate 635"
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
	{
		file_name = "file3";
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

activity="check cstate 665"
cat > ok << 'fubar'
brief_description = "second";
description = "second\n\
";
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

activity="check fstate 724"
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
		file_name = "file3";
		action = remove;
		edit =
		{
			revision = "1.1";
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

activity="check cstate 764"
cat > ok << 'fubar'
brief_description = "third";
description = "third\n\
";
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

activity="check fstate 823"
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

activity="check file 843"
cat > ok << 'fubar'
This is file1
fubar
if test $? -ne 0 ; then no_result; fi

diff ok $workproj/branch.1/branch.0/baseline/file1
if test $? -ne 0 ; then fail; fi

activity="check file 852"
cat > ok << 'fubar'
This is file2
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
