#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004, 2005 Peter Miller;
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
# MANIFEST: Test the aedist functionality
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
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

#
# set the path, so that the aegis command that aepatch/aedist invokes
# is from the same test set as the aepatch/aedist command itself.
#
PATH=${bin}:$PATH
export PATH

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
	echo "FAILED test of the aedist functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aedist functionality ($activity)" 1>&2
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
		-e 's/uuid = ".*"/uuid = "UUID"/' \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

activity="create test directory 107"
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

#
# test the aedist functionality
#
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="new project 129"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT


activity="project attributes 138"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 151"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 159"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 169"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 173"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/fred \
	$work/test.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "exit 0";
history_get_command =
	"co -u'$e' -p $h,v > $o";
history_create_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_put_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_query_command =
	"rlog -r $h,v | awk '/^head:/ {print $$2}'";
diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="build 200"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 204"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="uuid set 208"
$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd000 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 212"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 216"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 220"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 224"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 228"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# second change
#
activity="new change 235"
cat > caf << 'fubar'
brief_description = "the second change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 244"
$bin/aegis -db 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="copy file 248"
$bin/aegis -cp $work/test.C002/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo second > $work/test.C002/barney
if test $? -ne 0 ; then no_result; fi

activity="copy file 255"
$bin/aegis -cp $work/test.C002/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo third > $work/test.C002/fred
if test $? -ne 0 ; then no_result; fi

$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccdddddead -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build aedist chage set 265"
$bin/aedist -send -o $work/c2.ae -c 2 -ndh > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Nuke the uuid
#
activity="uncopy file 272"
$bin/aegis -cpu $work/test.C002/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 276"
$bin/aegis -cp $work/test.C002/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# Now make sure one of the files in the baseline will be different to
# the contents of the change set we just created.
echo second > $work/test.C002/fred
if test $? -ne 0 ; then no_result; fi

activity="build 285"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 289"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="uuid set 293"
$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccdddddeac -c 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 297"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 301"
$bin/aegis -ib 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 305"
$bin/aegis -b 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 309"
$bin/aegis -diff -c 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 313"
$bin/aegis -ipass -c 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

ulimit -c unlimited

activity="aedist -receive 319"
$bin/aedist -rec -f $work/c2.ae -c 3 -p $AEGIS_PROJECT -v -ignore-uuid -trojan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="verify change state 323"
cat > ok << 'fubar'
brief_description = "the second change";
description = "the second change";
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
attribute =
[
	{
		name = "original-UUID";
		value = "aaaaaaaa-bbbb-4bbb-8ccc-ccccdddddead";
	},
];
state = being_developed;
given_test_exemption = true;
given_regression_test_exemption = true;
project_file_command_sync = 2;
development_directory = ".../test.C003";
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
if test $? -ne 0 ; then cat log; no_result; fi

check_it ok $work/proj.dir/info/change/0/003

activity="verify change file state 369"
cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "1.2";
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
if test $? -ne 0 ; then cat log; no_result; fi
check_it ok $work/proj.dir/info/change/0/003.fs

# ------------------------------------------------------------------------
activity="aedist -receive 402"
$bin/aedist -rec -f $work/c2.ae -c 44 -p $AEGIS_PROJECT -v -trojan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="verify change state 406"
cat > ok << 'fubar'
brief_description = "the second change";
description = "the second change";
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
given_test_exemption = true;
given_regression_test_exemption = true;
project_file_command_sync = 2;
development_directory = ".../test.C044";
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
uuid = "UUID";
fubar
if test $? -ne 0 ; then cat log; no_result; fi

check_it ok $work/proj.dir/info/change/0/044

activity="verify change file state 446"
cat > ok << 'fubar'
src =
[
	{
		file_name = "barney";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "1.2";
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
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "1.2";
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
if test $? -ne 0 ; then cat log; no_result; fi
check_it ok $work/proj.dir/info/change/0/044.fs
# ------------------------------------------------------------------------

activity="send the change 502"
$bin/aedist -send 10 -ndh -o $work/c1.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Create a second project: test2
#
activity="new project 509"
$bin/aegis -npr test2 -version - -v -dir $work/test2.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test2
export AEGIS_PROJECT

activity="project attributes 517"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 530"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="receive the change 538"
$bin/aedist -rec -p test2 -c 1 -f $work/c1.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="edit fred 542"
echo 'test2: first' >> $work/test2.C001/fred
if test $? -ne 0; then cat log; no_result; fi

activity="build 546"
$bin/aegis -build 1 -v -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="difference 550"
$bin/aegis -diff 1 -v -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 554"
$bin/aegis -dev_end 1 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="check 558"
cat > ok <<EOF
brief_description = "one";
description = "one";
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
attribute =
[
	{
		name = "original-UUID";
		value = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd000";
	},
];
state = awaiting_integration;
given_test_exemption = true;
given_regression_test_exemption = true;
build_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
	},
];
development_directory = ".../test2.C001";
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
		what = develop_end_2ai;
		who = "USER";
	},
];
EOF
check_it ok $work/test2.dir/info/change/0/001

activity="archive send 615"
$bin/aedist -send -p test2 1 -ndh -o $work/test2c1.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Switch back to test
#

activity="archive receive 623"
$bin/aedist -rec -p test -c 4 -f $work/test2c1.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="check fstate 627"
cat > $work/ok <<EOF
src =
[
	{
		file_name = "barney";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = source;
	},
	{
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = source;
	},
];
EOF
check_it $work/ok $work/proj.dir/info/change/0/004.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
