#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2004-2008, 2012 Peter Miller
#       Copyright (C) 2008 Walter Franzini
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program. If not, see
#       <http://www.gnu.org/licenses/>.
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
AEGIS_THROTTLE=1
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

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
        sed     -e "s|$work|...|g" \
                -e 's|= [0-9][0-9]*; /.*|= TIME;|' \
                -e "s/\"$USER\"/\"USER\"/g" \
                -e 's/uuid = ".*"/uuid = "UUID"/' \
                -e 's/19[0-9][0-9]/YYYY/' \
                -e 's/20[0-9][0-9]/YYYY/' \
                -e 's/node = ".*"/node = "NODE"/' \
                -e 's/crypto = ".*"/crypto = "GUNK"/' \
                < $2 > $work/sed.out
        if test $? -ne 0; then no_result; fi
        diff -b $1 $work/sed.out
        if test $? -ne 0; then fail; fi
}

activity="create test directory 113"
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

activity="new project 135"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
        -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT


activity="project attributes 144"
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

activity="staff 157"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 165"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 175"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 179"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/fred \
        $work/test.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "echo no build required";

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

echo one > $work/test.C010/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="build 206"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 210"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="uuid set 214"
$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd000 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 218"
$bin/aegis -de -v > log 2>&1
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

activity="integrate pass 234"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# second change
#
activity="new change 241"
cat > caf << 'fubar'
brief_description = "the second change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 250"
$bin/aegis -db 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="copy file 254"
$bin/aegis -cp $work/test.C002/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo second > $work/test.C002/barney
if test $? -ne 0 ; then no_result; fi

activity="copy file 261"
$bin/aegis -cp $work/test.C002/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo third > $work/test.C002/fred
if test $? -ne 0 ; then no_result; fi

$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccdddddead -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build aedist chage set 271"
$bin/aedist -send -o $work/c2.ae -c 2 -ndh > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Nuke the uuid
#
activity="uncopy file 278"
$bin/aegis -cpu $work/test.C002/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 282"
$bin/aegis -cp $work/test.C002/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# Now make sure one of the files in the baseline will be different to
# the contents of the change set we just created.
echo second > $work/test.C002/fred
if test $? -ne 0 ; then no_result; fi

activity="build 291"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 295"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="uuid set 299"
$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccdddddeac -c 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 303"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 307"
$bin/aegis -ib 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 311"
$bin/aegis -b 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 315"
$bin/aegis -diff -c 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 319"
$bin/aegis -ipass -c 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

ulimit -c unlimited

activity="aedist -receive 325"
$bin/aedist -rec -f $work/c2.ae -c 3 -p $AEGIS_PROJECT -v -ignore-uuid -trojan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="verify change state 329"
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

activity="verify change file state 375"
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
                        uuid = "UUID";
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
activity="aedist -receive 408"
$bin/aedist -rec -f $work/c2.ae -c 44 -p $AEGIS_PROJECT -v -trojan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="verify change state 412"
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

activity="verify change file state 452"
cat > ok << 'fubar'
src =
[
        {
                file_name = "barney";
                uuid = "UUID";
                action = modify;
                edit_origin =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "UUID";
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
                        revision = "2";
                        encoding = none;
                        uuid = "UUID";
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

activity="send the change 508"
$bin/aedist -send 10 -ndh -o $work/c1.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Create a second project: test2
#
activity="new project 515"
$bin/aegis -npr test2 -version - -v -dir $work/test2.dir \
        -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test2
export AEGIS_PROJECT

activity="project attributes 523"
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

activity="staff 536"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="receive the change 544"
$bin/aedist -rec -p test2 -c 1 -f $work/c1.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="edit fred 548"
echo 'test2: first' >> $work/test2.C001/fred
if test $? -ne 0; then cat log; no_result; fi

activity="build 552"
$bin/aegis -build 1 -v -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="difference 556"
$bin/aegis -diff 1 -v -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 560"
$bin/aegis -dev_end 1 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="check 564"
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

activity="archive send 621"
$bin/aedist -send -p test2 1 -ndh -o $work/test2c1.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="check the aedist archive 625"
mkdir $work/test2c1.d > log 2>&1
if test $? -ne 0; then cat log; no_result; fi
$bin/test_cpio -extract --file $work/test2c1.ae -change-directory $work/test2c1.d \
    > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="generate the expected result 632"
cat > $work/ok <<EOF
$work/test2c1.d/etc/change-number
$work/test2c1.d/etc/change-set
$work/test2c1.d/etc/project-name
$work/test2c1.d/src/aegis.conf
$work/test2c1.d/src/barney
$work/test2c1.d/src/fred
EOF
if test $? -ne 0; then no_result; fi

activity="gather info 643"
find $work/test2c1.d -type f | sort > $work/test2c1.txt
if test $? -ne 0; then cat $work/test2c1.txt ; no_result; fi

cmp $work/test2c1.txt $work/ok > log 2>&1
if test $? -ne 0; then cat log; fail; fi

#
# Switch back to test
#

activity="archive receive 654"
$bin/aedist -rec -p test -c 4 -f $work/test2c1.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="check fstate 658"
cat > $work/ok <<EOF
src =
[
        {
                file_name = "aegis.conf";
                uuid = "UUID";
                action = modify;
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
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
                file_name = "barney";
                uuid = "UUID";
                action = modify;
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = source;
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
                action = modify;
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
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
EOF
check_it $work/ok $work/proj.dir/info/change/0/004.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
