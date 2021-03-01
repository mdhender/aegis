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
AEGIS_THROTTLE=-1
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
        echo "FAILED test of the aemv/merge functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT when testing the aemv/merge functionality ($activity)" 1>&2
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
                < $2 > $work/sed.out
        if test $? -ne 0; then no_result; fi
        diff -b $1 $work/sed.out
        if test $? -ne 0; then fail; fi
}

activity="create test directory 106"
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
# test the aemv/merge functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 134"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 141"
cat > tmp << 'end'
description = "A bogus project created to test the aemv/merge "
    "functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 157"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# create a new change
#
activity="new change 170"
cat > tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 188"
$bin/aegis -nf  $workchan/bogus1 -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/bogus2 -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/aegis.conf -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus1 << 'end'
bogus1, line 1
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/bogus2 << 'end'
line one
line two
line three
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
merge_command = "(diff3 -e $i $orig $mr | sed -e '/^w$$/d' -e '/^q$$/d'; \
        echo '1,$$p' ) | ed - $i > $out";
patch_diff_command = "set +e; diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 233"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 240"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="set the change UUID"
$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 247"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 254"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 261"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 268"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 275"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# Make sure the project looks the way we expect
#
activity="check project file state 284"
cat > ok << 'fubar'
src =
[
        {
                file_name = "aegis.conf";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
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
                file_name = "bogus1";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
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
                file_name = "bogus2";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
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

check_it ok $workproj/info/trunk.fs

# --------------------------------------------------------------------------

#
# create a new change
#
activity="new change 365"
cat > tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 377"
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Use the second change to move bogus2 to bogus3
#
activity="move file 384"
$bin/aegis -c 2 -mv -baserel bogus2 bogus3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Make sure change 2 looks the way we expect
#
activity="check change file state 391"
cat > ok << 'fubar'
src =
[
        {
                file_name = "bogus2";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
                action = remove;
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
                };
                usage = source;
                move = "bogus3";
        },
        {
                file_name = "bogus3";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
                action = create;
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
                };
                usage = source;
                move = "bogus2";
        },
];
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.fs

# --------------------------------------------------------------------------

#
# Create a third change to modify bogus2
#
activity="new change 430"
cat > tmp << 'end'
brief_description = "The third change.";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 439"
$bin/aegis -db 3 -dir ${workchan}3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 443"
$bin/aegis -cp 3 -baserel bogus2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > ${workchan}3/bogus2 << 'end'
line one
line two
line two-and-a-half
line three
end
if test $? -ne 0 ; then no_result; fi

activity="build 455"
$bin/aegis -build 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 459"
$bin/aegis -diff 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="set the change UUID 481"
$bin/aegis -ca 3 -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd5 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 463"
$bin/aegis -de 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 467"
$bin/aegis -rpass -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 471"
$bin/aegis -ib 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 475"
$bin/aegis -b -nl -v 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 479"
$bin/aegis -intpass -nl 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# Make sure the project looks the way we expect
#
activity="check project file state 488"
cat > ok << 'fubar'
src =
[
        {
                file_name = "aegis.conf";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
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
                file_name = "bogus1";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
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
                file_name = "bogus2";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
                action = create;
                edit =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd5";
                };
                edit_origin =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd5";
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

check_it ok $workproj/info/trunk.fs

# --------------------------------------------------------------------------

ulimit -c unlimited

#
# Resume change 2, make sure that aed --merge works.
#
activity="aed --merge 571"
$bin/aegis --diff --merge-only > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

# --------------------------------------------------------------------------

#
# Make sure the change looks the way we expect
#
activity="check change file state 580"
cat > ok << 'fubar'
src =
[
        {
                file_name = "bogus2";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
                action = remove;
                edit_origin =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd5";
                };
                usage = source;
                move = "bogus3";
        },
        {
                file_name = "bogus3";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
                action = create;
                edit_origin =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd5";
                };
                usage = source;
                move = "bogus2";
        },
];
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.fs

#
# Make sure the change tracks the baseline.
#
diff $workproj/baseline/bogus2 $workchan/bogus3
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
