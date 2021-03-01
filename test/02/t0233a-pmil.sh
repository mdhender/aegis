#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2007, 2008, 2012 Peter Miller
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
        log_file_preference = never; \
        default_development_directory = \"$work\";"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

# This tells aeintegratq that it is being used by a test.
AEGIS_TEST_DIR=$work
export AEGIS_TEST_DIR

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

parch=
test "$1" != "" && parch="$1/"
bin="$here/${parch}bin"

if test "$EXEC_SEARCH_PATH" != ""
then
    tpath=
    hold="$IFS"
    IFS=":$IFS"
    for tpath2 in $EXEC_SEARCH_PATH
    do
        tpath=${tpath}${tpath2}/${parch}bin:
    done
    IFS="$hold"
    PATH=${tpath}${PATH}
else
    PATH=${bin}:${PATH}
fi
export PATH

AEGIS_DATADIR=$here/lib
export AEGIS_DATADIR

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
        echo "FAILED test of the aedist uuid functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT when testing the aedist uuid functionality ($activity)" 1>&2
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

activity="create test directory 130"
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
# test the aedist uuid functionality
#
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="new project 152"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
        -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="project attributes 160"
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

activity="staff 173"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 181"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 191"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 195"
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

activity="build 222"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 226"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="uuid set 230"
$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd000 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 234"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 238"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 242"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 246"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 250"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# second change
#
activity="new change 257"
cat > caf << 'fubar'
brief_description = "the second change";
cause = internal_enhancement;
attribute =
[
    { name = "original-uuid"; value = "11111111-2222-3333-4444-555555555555"; }
];
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 270"
$bin/aegis -db 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="copy file 274"
$bin/aegis -cp $work/test.C002/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo two > $work/test.C002/barney
if test $? -ne 0 ; then no_result; fi

activity="copy file 281"
$bin/aegis -cp $work/test.C002/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo two > $work/test.C002/fred
if test $? -ne 0 ; then no_result; fi

$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccdddddead -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build aedist change set 291"
$bin/aedist -send -o $work/c2.ae -c 2 -ndh > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Nuke the uuid
#
activity="uncopy file 298"
$bin/aegis -cpu $work/test.C002/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > ca << 'fubar'
attribute =
[
    { name = "original-uuid"; value = "11111111-2222-3333-4444-555555555555"; }
];
fubar
if test $? -ne 0; then no_result; fi

$bin/aegis -ca -f ca -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 313"
$bin/aegis -cp $work/test.C002/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# the contents the same as before
echo two > $work/test.C002/fred
if test $? -ne 0 ; then no_result; fi

activity="build 321"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 325"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="uuid set 329"
$bin/aegis -ca -uuid deaddead-dead-4ead-8ead-deaddeaddead -c 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 333"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 337"
$bin/aegis -ib 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 341"
$bin/aegis -b 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 345"
$bin/aegis -diff -c 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 349"
$bin/aegis -ipass -c 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------
#
# Now we receive exactly the same change set.
# It sould result in change 3 being abandoned,
# and change 2 getting an additional original-uuid attribute.
#
activity="aedist -receive 359"
$bin/aedist -rec -f $work/c2.ae -c 3 -p $AEGIS_PROJECT -v -trojan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="verify change state 363"
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
                name = "original-uuid";
                value = "11111111-2222-3333-4444-555555555555";
        },
        {
                name = "aegis:history_get_command";
                value = "aesvt -check-out -edit ${quote $edit} -history ${quote $history} -f ${quote $output}";
        },
        {
                name = "original-UUID";
                value = "aaaaaaaa-bbbb-4bbb-8ccc-ccccdddddead";
        },
];
state = completed;
given_test_exemption = true;
given_regression_test_exemption = true;
delta_number = 2;
delta_uuid = "UUID";
project_file_command_sync = 10;
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

check_it ok $work/proj.dir/info/change/0/002

if [ -r $work/proj.dir/info/change/0/003 ]
then
    cat log
    fail
fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
