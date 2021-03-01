#!/bin/sh
#
# aegis - The "aegis" program.
# Copyright (C) 2010 Walter Franzini
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

TEST_SUBJECT="aenf vs. file's UUID"

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

activity="new branch 85"
aegis -new-branch 1 -p $AEGIS_PROJECT
test $? -eq 0 || no_result

AEGIS_PROJECT=foo.1

activity="new branch 91"
aegis -new-branch 1 -p $AEGIS_PROJECT

AEGIS_PROJECT=foo.1.1

#
# create a new change
#
activity="new change 99"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 1 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 108"
aegis -dev-begin 1 -dir $work/dd1
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 115"
aegis -new-file -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd000 $work/dd1/aegis.conf
test $? -eq 0 || fail

cat > $work/dd1/aegis.conf << 'fubar'
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

activity="create barney 140"
aegis -new-file -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd001 $work/dd1/barney -nl
test $? -eq 0 || no_result

cat > $work/dd1/barney <<EOF
Hi my name is Barney
EOF
test $? -eq 0 || no_result

activity="create test file 149"
aegis -new-test -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd002 $work/dd1/test.sh -nl
test $? -eq 0 || no_result

cat > $work/dd1/test.sh <<EOF
#!/bin/sh

exit 0
EOF
test $? -eq 0 || no_result

activity="change uuid 160"
aegis -cattr --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd00f 1
test $? -eq 0 || no_result

activity="dev_end change 164"
aefinish 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the change 168"
aefinish 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi


activity="dev_end branch 173"
aegis -dev_end -p foo.1 1
if test $? -ne 0; then cat log; no_result; fi


activity="integrate the branch 178"
aefinish -p foo.1 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi


activity="check branch fstate 183"
cat > ok <<EOF
src =
[
    {
        file_name = "aegis.conf";
        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd000";
        action = create;
        edit =
        {
            revision = "1";
            encoding = none;
            uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd00f";
        };
        usage = config;
    },
    {
        file_name = "barney";
        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd001";
        action = create;
        edit =
        {
            revision = "1";
            encoding = none;
            uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd00f";
        };
        usage = source;
    },
    {
        file_name = "test.sh";
        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd002";
        action = create;
        edit =
        {
            revision = "1";
            encoding = none;
            uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd00f";
        };
        usage = test;
    },
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/001.branch/0/001.branch/0/001.fs

#
# create a new change in a different branch
#
activity="new branch 232"
aegis -new-branch -p foo 2
test $? -eq 0 || no_result

AEGIS_PROJECT=foo.2

activity="new change 238"
cat > cattr << 'end'
brief_description = "Second change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 2 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 247"
aegis -dev-begin 2 -dir $work/dd2
test $? -eq 0 || no_result

activity="config file 251"
aegis -new-file 2 $work/dd2/aegis.conf -nl
test $? -eq 0 || no_result

activity="source file 255"
aegis -new-file 2 $work/dd2/barney -nl
test $? -eq 0 || no_result

activity="test file 259"
aegis -new-test 2 $work/dd2/test.sh -nl
test $? -eq 0 || no_result

activity="check fstate 263"
cat > ok <<EOF
src =
[
    {
        file_name = "aegis.conf";
        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd000";
        action = create;
        usage = config;
    },
    {
        file_name = "barney";
        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd001";
        action = create;
        usage = source;
    },
    {
        file_name = "test.sh";
        uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd002";
        action = create;
        usage = test;
    },
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/002.branch/0/002.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
