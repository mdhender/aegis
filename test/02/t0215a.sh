#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2005-2008, 2012 Peter Miller
#       Copyright (C) 2006, 2008 Walter Franzini
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
        echo "FAILED test of the unchanged_file_develop_end_policy functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT when testing the unchanged_file_develop_end_policy functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
}

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

trap \"no_result\" 1 2 3 15

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
# test the unchanged_file_develop_end_policy functionality
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

proj=$AEGIS_PROJECT

activity="new change 183"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 193"
$bin/aegis -db 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 197"
$bin/aegis -nf $work/${proj}.C001/aegis.conf $work/${proj}.C001/fred \
        $work/${proj}.C001/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/${proj}.C001/aegis.conf << 'fubar'
build_command = "date > derived1 && date > derived2";

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
unchanged_file_integrate_pass_policy = remove;
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/${proj}.C001/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/${proj}.C001/barney
if test $? -ne 0 ; then no_result; fi

activity="build 225"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 229"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 233"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 237"
$bin/aegis -ib -mini 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 241"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 245"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 249"
$bin/aegis -ipass -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create another change
#
activity="new change 256"
cat > caf << 'fubar'
brief_description = "two";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 266"
$bin/aegis -db 2 -dir $work/$proj.C002 >  log 2>&1
if test $? -ne 0; then cat log; no_result; fi

$bin/aegis -cp 2 -base-rel fred barney -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

date > $work/$proj.C002/barney
if test $? -ne 0; then cat log; no_result; fi

activity="build 276"
$bin/aegis -build 2  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 280"
$bin/aegis -diff 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="dev_end 284"
$bin/aegis -dev_end 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 288"
$bin/aegis -ib -mini 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 292"
$bin/aegis -b 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 296"
$bin/aegis -diff 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 300"
$bin/aegis -ipass 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > test.ok << 'fubar'
barney
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aelcf -p $proj -c 2 > test.out
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

activity="check 002.fs 315"
cat > ok <<EOF
src =
[
        {
                file_name = "barney";
                uuid = "UUID";
                action = modify;
                edit =
                {
                        revision = "2";
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
        },
];
EOF
if test $? -ne 0; then no_result; fi

check_it ok $work/proj.dir/info/change/0/002.fs

activity="check 002.pfs 341"
cat > ok <<EOF
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
                file_name = "barney";
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
        },
];
EOF

check_it ok $work/proj.dir/info/change/0/002.pfs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
