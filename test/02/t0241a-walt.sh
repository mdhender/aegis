#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2006, 2008, 2010 Walter Franzini
#       Copyright (C) 2006-2008, 2012 Peter Miller
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

TEST_SUBJECT="aeipass"

# load up standard prelude and test functions
. test_funcs

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

#
# test the aeipass functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PROJECT=example;
export AEGIS_PROJECT

#
# make a new project
#
activity="new project 54"
aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 61"
cat > tmp << 'end'
description = "A bogus project created to test the "
    "aeipass functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
end
if test $? -ne 0 ; then no_result; fi
aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 78"
aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch 86"
aegis -p $AEGIS_PROJECT -nbr 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=example.1
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 96"
cat > tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
aegis -nc 1 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 108"
aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
# Ask aeipass to not assign the file's UUID to simulate the behaviour
# of older aegis releases.
#
activity="new file 118"
aegis -nf  -no-uuid $workchan/bogus1 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 122"
aegis -nf  -no-uuid $workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus1 << 'end'
bogus1, line 1
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
merge_command = "exit 0 # $input $output $orig $most_recent";
patch_diff_command = "set +e; diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 150"
aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 157"
aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

assign="set change's UUID"
aegis -change-attr --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1 1 \
    -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 169"
aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 176"
aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 180"
aegis -diff 1 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# integrate build
#
activity="build 187"
aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 194"
aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check fstate 198"
cat > ok <<EOF
src =
[
        {
                file_name = "aegis.conf";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                };
                usage = config;
        },
        {
                file_name = "bogus1";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                };
                usage = source;
        },
];
EOF
if test $? -ne 0 ; then cat log; no_result; fi

check_it ok $workproj/info/change/0/001.branch/0/001.fs

activity="check branch fstate 230"
cat > ok <<EOF
src =
[
        {
                file_name = "aegis.conf";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
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
                file_name = "bogus1";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
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
if test $? -ne 0 ; then cat log; no_result; fi

check_it ok $workproj/info/change/0/001.fs

#
# Now we close the branch.
#
AEGIS_PROJECT=example
export AEGIS_PROJECT

activity="change branch uuid 304"
aegis -change-attr 1 --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end the branch 308"
aegis -dev_end 1 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="ibegin 312"
aegis -ibegin 1 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="ipass the branch 316"
aegis -ipass 1 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="check branch fstate 320"
cat > ok <<EOF
src =
[
        {
                file_name = "aegis.conf";
                action = create;
                edit =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                };
                usage = config;
        },
        {
                file_name = "bogus1";
                action = create;
                edit =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                };
                usage = source;
        },
];
EOF
if test $? -ne 0; then no_result; fi

check_it $work/ok $workproj/info/change/0/001.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
