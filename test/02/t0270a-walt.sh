#!/bin/sh
#
# aegis - The "aegis" program.
# Copyright (C) 2008, 2010 Walter Franzini
# Copyright (C) 2012 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.
#

TEST_SUBJECT="aediff vs. mismatching UUIDs"

# load up standard prelude and test functions
. test_funcs

check_it()
{
    sed -e "s|$work|...|g" \
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
# make a new project
#
activity="new project 44"
aegis -newpro foo -version "" -dir $work/proj -lib $work/lib
if test $? -ne 0 ; then fail; fi

AEGIS_PROJECT=foo
export AEGIS_PROJECT

#
# change project attributes
#
activity="project attributes 54"
cat > atts << 'fubar'
description = "The \"foo\" program.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
developers_may_create_changes = true;
umask = 022;
default_test_exemption = true;
default_test_regression_exemption = true;
minimum_change_number = 10;
reuse_change_numbers = true;
minimum_branch_number = 1;
skip_unlucky = false;
compress_database = false;
develop_end_action = goto_awaiting_integration;
protect_development_directory = false;
fubar
test $? -eq 0 || no_result

aegis -proatt -f atts -proj foo -lib $work/lib
test $? -eq 0 || no_result

activity="new developer 77"
aegis -newdev $USER
test $? -eq 0 || no_result
aegis -new-reviewer $USER
test $? -eq 0 || no_result
aegis -new-integrator $USER
test $? -eq 0 || no_result

#
# create a new change
#
activity="new change 88"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 1 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 97"
aegis -dev-begin 1 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 104"
aegis -new-file $work/dd/aegis.conf
test $? -eq 0 || no_result

cat > $work/dd/aegis.conf << 'fubar'
build_command = "exit 0";
diff_command =
    "set +e;"
    "diff -a ${quote $original} ${quote $input}"
    " > ${quote $output};"
    "test $? -le 1";
merge_command = "set +e; "
    "merge -p -L baseline -L Historical -L C$c "
    "${quote $mostrecent} ${quote $original} ${quote $input} "
    "> ${quote $output}; "
    "test $? -le 1";
history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;
fubar
test $? -eq 0 || no_result


#
# finish development of the change
#
activity="finish dev 133"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# integrate the change
#
activity="finis int 140"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

# --------------------------------------------------------------------------

#
# we create a branch, because the bug is about branches
#
aegis -p foo -nbr 2 -v > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

AEGIS_PROJECT=foo.2
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 158"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 1 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 167"
aegis -dev-begin 1 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 174"
aegis -new-file $work/dd/barney \
    -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1
test $? -eq 0 || no_result

cat > $work/dd/barney <<EOF
Hi, I'm Barney!
EOF
test $? -eq 0 || no_result

activity="set change uuid 184"
aegis -change-attr 1 -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddffff \
    > LOG 2>&1
test $? -eq 0 || no_result

activity="finish the change 189"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="check fstate 196"
cat > ok <<EOF
src =
[
    {
        file_name = "barney";
        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
        action = create;
        edit =
        {
            revision = "1";
            encoding = none;
            uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddffff";
        };
        edit_origin =
        {
            revision = "1";
            encoding = none;
            uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddffff";
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
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/002.fs

# --------------------------------------------------------------------------
#
# The second branch
#
# --------------------------------------------------------------------------00
aegis -p foo -nbr 3 -v > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

AEGIS_PROJECT=foo.3
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 250"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 1 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 259"
aegis -dev-begin 1 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 266"
aegis -new-file $work/dd/barney
test $? -eq 0 || no_result

activity="check fstate 270"
cat > ok <<EOF
src =
[
    {
        file_name = "barney";
        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
        action = create;
        usage = source;
    },
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/003.branch/0/001.fs

#
# We explicitly set the file's UUID to be sure that fixing aeipass
# does not make this test useless.  We want the UUIDs to be different!
#
# We need to do it manually since the new code make it impossible this
# to happen.  Or at least really difficult to make it happen.
#
cat > $work/proj/info/change/0/003.branch/0/001.fs <<EOF
src =
[
    {
        file_name = "barney";
        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
        action = create;
        usage = source;
        edit =
        {
            revision = "1";
            encoding = none;
            uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddfffe";
        };
        edit_origin =
        {
            revision = "1";
            encoding = none;
            uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddfffe";
        };
    },
];
EOF

cat > $work/dd/barney <<EOF
Hi, I'm not Barney!
EOF
test $? -eq 0 || no_result

activity="set change uuid 322"
aegis -change-attr 1 -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddfffe
test $? -eq 0 || no_result

activity="develop and the change 326"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="check fstate 333"
cat > ok <<EOF
src =
[
    {
        file_name = "barney";
        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
        action = create;
        edit =
        {
            revision = "1";
            encoding = none;
            uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddfffe";
        };
        edit_origin =
        {
            revision = "1";
            encoding = none;
            uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddfffe";
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
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/003.fs

#
# diff the files
#
activity="run aediff 376"
cat > ok <<EOF
*** 2.D001/barney
--- 3.D001/barney
***************
*** 1 ****
! Hi, I'm Barney!
--- 1 ----
! Hi, I'm not Barney!
EOF
test $? -eq 0 || no_result

aediff -context -c 2.d1 -c 3.d1 barney > test.out
if test $? -ne 0; then no_result; fi

diff -c ok test.out
if test $? -ne 0; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
