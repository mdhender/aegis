#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 1997, 1998, 2000, 2001, 2004-2008, 2012 Peter Miller
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
        echo "FAILED test of the branch ipass functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT when testing the branch ipass functionality ($activity)" 1>&2
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
                -e 's/19[0-9][0-9]/YYYY/' \
                -e 's/20[0-9][0-9]/YYYY/' \
                -e 's/node = ".*"/node = "NODE"/' \
                -e 's/crypto = ".*"/crypto = "GUNK"/' \
                -e 's/uuid = ".*"/uuid = "UUID"/' \
                < $2 > $work/sed.out
        if test $? -ne 0; then fail; fi
        diff -b $1 $work/sed.out
        if test $? -ne 0; then fail; fi
}

mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

workproj=$work/proj
workchan=$work/chan
worklib=$work/lib

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the branch ipass functionality
#
activity="new project 119"
$bin/aegis -npr test -vers '' -dir $workproj -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 123"
$bin/aegis -nd $USER -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="staff 126"
$bin/aegis -nrv $USER -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="staff 129"
$bin/aegis -ni $USER -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 133"
cat > pa << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f pa -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="create branch 143"
$bin/aegis -nbr 4 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a change to the branch
#
activity="new change 150"
cat > ca << 'fubar'
brief_description = "first";
description = "This is the first change.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f ca -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 163"
$bin/aegis -db 10 -p test.4 -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 167"
$bin/aegis -nf $workchan/aegis.conf -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/aegis.conf << 'fubar'
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

# build
activity="build 193"
$bin/aegis -b -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="difference 197"
$bin/aegis -diff -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 201"
$bin/aegis -de -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 205"
$bin/aegis -rpass 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 209"
$bin/aegis -ib 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# build
activity="build 214"
$bin/aegis -b -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# diff
activity="diff 219"
$bin/aegis -diff -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 223"
$bin/aegis -ipass 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create another change to branch 4
#
activity="new change 230"
cat > ca << 'fubar'
brief_description = "second";
description = "This is the second change.";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f ca -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop begin
activity="develop begin 244"
$bin/aegis -db 11 -p test.4 -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 248"
$bin/aegis -nf $workchan/fred -c 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new test 252"
$bin/aegis -nt $workchan/a.test.sh -c 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo exit 0 > $workchan.a.test.sh
if test $? -ne 0 ; then no_result; fi

activity="change attributes 259"
$bin/aegis -ca -f ca -c 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# build
activity="build 264"
$bin/aegis -b -c 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# test
activity="test 269"
$bin/aegis -t -c 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="difference 273"
$bin/aegis -diff -c 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 277"
$bin/aegis -de -c 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 281"
$bin/aegis -rpass 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 285"
$bin/aegis -ib 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# build
activity="build 290"
$bin/aegis -b -c 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# test
activity="test 295"
$bin/aegis -t -c 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="difference 299"
$bin/aegis -diff -c 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 303"
$bin/aegis -ipass 11 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check file state 307"
cat > ok << 'fubar'
src =
[
        {
                file_name = "a.test.sh";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = test;
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
                architecture_times =
                [
                        {
                                variant = "unspecified";
                                test_time = TIME;
                        },
                ];
        },
        {
                file_name = "aegis.conf";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
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
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
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
                diff_file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
                test =
                [
                        "a.test.sh",
                ];
        },
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $workproj/info/change/0/004.fs

#
# end branch 4 and integrate it
#
activity="change attributes 414"
cat > ca << 'fubar'
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -ca -f ca -c 4 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 423"
$bin/aegis -de -c 4 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 427"
$bin/aegis -rpass -c 4 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 431"
$bin/aegis -ib -c 4 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 435"
$bin/aegis -b -c 4 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 439"
$bin/aegis -t -c 4 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 443"
$bin/aegis -ipass -c 4 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Now make sure that the test correlations look as they should
#
activity="check file state 450"
cat > ok << 'fubar'
src =
[
        {
                file_name = "a.test.sh";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "UUID";
                };
                edit_origin =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = test;
                file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
                architecture_times =
                [
                        {
                                variant = "unspecified";
                                test_time = TIME;
                        },
                ];
        },
        {
                file_name = "aegis.conf";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "UUID";
                };
                edit_origin =
                {
                        revision = "2";
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
                file_name = "fred";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "UUID";
                };
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
                test =
                [
                        "a.test.sh",
                ];
        },
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $workproj/info/trunk.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
