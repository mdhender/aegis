#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996-1998, 2000, 2001, 2004-2008 Peter Miller
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
	echo "FAILED test of the cross-branch merge functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of the cross-branch merge functionality ($activity)" 1>&2
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
		-e 's/uuid = ".*"/uuid = "UUID"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

activity="working directory 99"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

AEGIS_PATH=$work/lib
export AEGIS_PATH
AEGIS_PROJECT=test
export AEGIS_PROJECT

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the cross-branch merge functionality
#
activity="new project 123"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $work/test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 127"
cat > paf << 'fubar'
developer_may_review = true;
reviewer_may_integrate = true;
developer_may_integrate = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -p $AEGIS_PROJECT -file paf > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 138"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# the first change established the files and their versions
#
activity="new change 149"
cat > caf << 'fubar'
brief_description = "ten";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 158"
$bin/aegis -db 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 162"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/fred > log 2>&1
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
fubar
if test $? -ne 0 ; then no_result; fi

cat > $work/test.C010/fred << 'fubar'
one
two
three
four
five
fubar
if test $? -ne 0 ; then no_result; fi

activity="build 191"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 195"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 199"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 203"
$bin/aegis -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 207"
$bin/aegis -ib 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 211"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 215"
$bin/aegis -ipass > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# the branch is so that it can get out-of-date,
# necessitating the cross-branch merge
# between the trunk and the branch
#
activity="new branch 224"
$bin/aegis -nbr -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="change attributes 228"
cat > caf << 'fubar'
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -ca -f caf -p $AEGIS_PROJECT -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

activity="new change 242"
cat > caf << 'fubar'
brief_description = "one point ten";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 251"
$bin/aegis -db 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 255"
$bin/aegis -cp $work/test.1.C010/fred > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/test.1.C010/fred << 'fubar'
one
two
three
four
five
six
fubar
if test $? -ne 0 ; then no_result; fi

activity="build 269"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 273"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 277"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 281"
$bin/aegis -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 285"
$bin/aegis -ib 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 289"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 293"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 297"
$bin/aegis -ipass > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Now perform another change on the trunk.  This will make "fred" on
# the trunk different to "fred" on the branch.
#

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="new change 309"
cat > caf << 'fubar'
brief_description = "eleven";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 318"
$bin/aegis -db 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 322"
$bin/aegis -cp $work/test.C011/fred -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/test.C011/fred << 'fubar'
three
four
five
fubar
if test $? -ne 0 ; then no_result; fi

activity="build 333"
$bin/aegis -b -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 337"
$bin/aegis -diff -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 341"
$bin/aegis -de -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 345"
$bin/aegis -rpass 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 349"
$bin/aegis -ib 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 353"
$bin/aegis -b -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 357"
$bin/aegis -ipass -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# At this point the branch is out-of-date with respect to the trunk.
# Make sure that aede on the branch fails.
#
activity="check project file state 365"
cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = config;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "fred";
		uuid = "UUID";
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
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/trunk.fs

activity="check branch file state 418"
cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1";
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
check_it ok $work/test/info/change/0/001.fs

activity="develop end [fail] 455"
$bin/aegis -de 1 -v > log 2>&1
if test $? -ne 1 ; then cat log; no_result; fi

#
# create a new change on the branch specifically to perform a cross
# branch merge.
#

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

activity="new change 467"
cat > caf << 'fubar'
brief_description = "one point ten";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 476"
$bin/aegis -db 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 480"
$bin/aegis -cp $work/test.1.C011/fred > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="cross branch merge 484"
$bin/aegis -diff -branch '' -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

mv $work/test.1.C011/fred,D $work/test.1.C011/fred
if test $? -ne 0 ; then no_result; fi

activity="check change file state 491"
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
		edit_origin_new =
		{
			revision = "3";
			encoding = none;
		};
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/001.branch/0/011.fs

activity="build 516"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 520"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 524"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 528"
$bin/aegis -rpass 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 532"
$bin/aegis -ib 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 536"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 540"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 544"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check branch file state 548"
cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "4";
			encoding = none;
		};
		edit_origin =
		{
			revision = "3";
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
check_it ok $work/test/info/change/0/001.fs

#
# now the branch should end development cleanly
#
AEGIS_PROJECT=test
export AEGIS_PROJECT

$bin/aegis -de -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -rpass -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -ib -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
