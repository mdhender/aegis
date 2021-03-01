#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1997-2008 Peter Miller
#	Copyright (C) 2007 Walter Franzini
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
	log_file_preference = never; \
	default_project_directory = \"$work\"; \
	default_development_directory = \"$work\";"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin=$here/${1-.}/bin

if test "$EXEC_SEARCH_PATH" != ""
then
    tpath=
    hold="$IFS"
    IFS=":$IFS"
    for tpath2 in $EXEC_SEARCH_PATH
    do
	tpath=${tpath}${tpath2}/${1-.}/bin:
    done
    IFS="$hold"
    PATH=${tpath}${PATH}
else
    PATH=${bin}:${PATH}
fi
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
	echo "FAILED test of the aeclone functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of the aeclone functionality ($activity)" 1>&2
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

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

AEGIS_PROJECT=test
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

#
# test the aeclone functionality
#
activity="new project 136"
$bin/aegis -npr test -version '' -v -dir $work/test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 140"
cat > paf << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
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
$bin/aegis -nc -f caf -v -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 169"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 173"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "exit 0";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
if test $? -ne 0 ; then no_result; fi
echo hello > $work/test.C010/fred
if test $? -ne 0 ; then no_result; fi

activity="new test 195"
$bin/aegis -nt -v > log 2>&1
echo exit 0 > $work/test.C010/test/00/t0001a.sh
if test $? -ne 0 ; then no_result; fi
$bin/aegis -ca -f caf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 202"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 206"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 210"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 214"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 218"
$bin/aegis -rpass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 222"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 226"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 230"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate test 234"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 238"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change a file
#
activity="new change 245"
cat > caf << 'fubar'
brief_description = "two";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 254"
$bin/aegis -db 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 258"
$bin/aegis -cp $work/test.C011/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo "fred mark 2" > $work/test.C011/fred
if test $? -ne 0 ; then no_result; fi

activity="build 265"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 269"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="set the change's UUID 273"
$bin/aegis -cattr -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccdddddead > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 277"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 281"
$bin/aegis -rpass 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 285"
$bin/aegis -ib 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 289"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 293"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 297"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="clone 301"
$bin/aegis -clone 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check change file state 305"
cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "2";
			encoding = none;
		};
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/012.fs

activity="check change state 325"
cat > ok << 'fubar'
brief_description = "two (clone of D002)";
description = "two";
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
development_directory = ".../test.C012";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "USER";
		why = "Cloned from change 11.";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "USER";
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/012

activity="check project state 368"
cat > ok << 'fubar'
brief_description = "The \"test\" program.";
description = "The \"test\" program.";
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
];
state = being_developed;
architecture_times =
[
];
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
	developers_may_create_changes = false;
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
	];
	change =
	[
		10,
		11,
		12,
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
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/trunk

activity="build 455"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 459"
$bin/aegis -de -v > log 2>&1
if test $? -ne 1 ; then cat log; no_result; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
