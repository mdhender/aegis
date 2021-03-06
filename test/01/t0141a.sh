#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2003-2008, 2012 Peter Miller
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

#
# set the path, so that the aegis command that aepath invokes
# is from the same test set as the aepatch command itself.
#
PATH=${bin}:$PATH
export PATH

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

no_result()
{
        set +x
        echo "NO RESULT for test of aenf functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
}
fail()
{
        set +x
        echo "FAILED test of aenf functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
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
trap "no_result" 1 2 3 15

#
# some variable to make things earier to read
#
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

worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT

#
# make the directories
#
activity="working directory 118"
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
# make a new project
#
activity="new project 137"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 144"
cat > $tmp << 'end'
description = "A bogus project created to test the aenf functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 159"
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
activity="new change 172"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 190"
$bin/aegis -nf  $workchan/bogus -nl \
        -uuid "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1" > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/aegis.conf -nl \
        -uuid "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2" > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus << 'end'
the trunk version
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
create_symlinks_before_build = true;
remove_symlinks_after_build = false;
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
        echo '1,$$p' ) | ed - $mr > $out";
patch_diff_command = "set +e; diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 227"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 234"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="set the change uuid"
$bin/aegis -change_attr --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3 \
    > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 241"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 248"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 255"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 262"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 269"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

activity="check project file state 275"
cat > ok << fubar
src =
[
        {
                file_name = "aegis.conf";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3";
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
                file_name = "bogus";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3";
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
activity="new change 334"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_baseline_exempt = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 42 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 42 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add files to the change
# remove and then add, to change type
#
activity="copy file 354"
$bin/aegis -rm $workchan/bogus -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nt $workchan/bogus -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo "exit 0" > $workchan/bogus
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 366"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 373"
$bin/aegis -test -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 380"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -ca -f $tmp;
if test $? -ne 0 ; then cat log; no_result; fi

activity="set the change uuid"
$bin/aegis -change_attr --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4 \
    > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 390"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 394"
$bin/aegis -rpass 42 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate the change
#
activity="integrate begin 401"
$bin/aegis -ib 42 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 405"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate test 409"
$bin/aegis -test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 413"
$bin/aegis -ipass > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

activity="check project file state 419"
cat > ok << fubar
src =
[
        {
                file_name = "aegis.conf";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3";
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
                file_name = "bogus";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                action = create;
                edit =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
                };
                edit_origin =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4";
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
];
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/trunk.fs

#
# the things tested in this test, worked
# can't speak for the rest of the code
#
pass
# vim: set ts=8 sw=4 et :
