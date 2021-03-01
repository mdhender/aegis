#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 2011 Walter Franzini
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

TEST_SUBJECT="aecp -ind vs. renames"

# load up standard prelude and test functions
. test_funcs

#
# make a new project
#
activity="new project 28"
aegis -newpro foo -version "" -dir $work/proj -lib $work/lib
if test $? -ne 0 ; then fail; fi

#
# change project attributes
#
activity="project attributes 35"
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

AEGIS_PROJECT=foo
export AEGIS_PROJECT

activity="new developer 61"
aegis -newdev $USER
test $? -eq 0 || no_result
aegis -new-reviewer $USER
test $? -eq 0 || no_result
aegis -new-integrator $USER
test $? -eq 0 || no_result

activity="new branch 69"
aegis -new-branch 1 -p $AEGIS_PROJECT
test $? -eq 0 || no_result

AEGIS_PROJECT=foo.1

activity="new branch 75"
aegis -new-branch 1 -p $AEGIS_PROJECT

AEGIS_PROJECT=foo.1.1
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 84"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
test_exempt = true;
end
test $? -eq 0 || no_result
aegis -new_change 10 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 94"
aegis -dev-begin 10 -dir $work/dd1
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 101"
aegis -new-file $work/dd1/aegis.conf
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
posix_filename_charset = false;
shell_safe_filenames = false;
allow_white_space_in_filenames = true;
allow_non_ascii_filenames = true;
fubar
test $? -eq 0 || no_result

activity="create file 130"
aegis -nf $work/dd1/file1
test $? -eq 0 || no_result

cat > $work/dd1/file1 <<EOF
file1 change1
EOF
test $? -eq 0 || no_result

activity="dev_end the change 139"
aefinish 10 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate begin 143"
aegis -ib 10
test $? -eq 0 || no_result

activity="integrate the change 147"
aefinish 10 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# create a new change
#
activity="new change 154"
cat > cattr << 'end'
brief_description = "Second change.";
cause = internal_enhancement;
test_exempt = true;
end
test $? -eq 0 || no_result
aegis -new_change 11 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 164"
aegis -dev-begin 11 -dir $work/dd
test $? -eq 0 || no_result

activity="rename the file 168"
aegis -mv $work/dd/file1 $work/dd/file1a
test $? -eq 0 || no_result

cat > $work/dd/file1a <<EOF
file1a change2
EOF
test $? -eq 0 || no_result

activity="dev_end the change 177"
aefinish 11 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate begin 181"
aegis -ib 11
test $? -eq 0 || no_result

activity="integrate the change 185"
aefinish 11 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# create a new change
#
activity="new change 192"
cat > cattr << 'end'
brief_description = "Third change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 12 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 201"
aegis -dev-begin 12 -dir $work/dd
test $? -eq 0 || no_result

activity="rename the change 205"
aegis -mv $work/dd/file1a $work/dd/file1b
test $? -eq 0 || no_result

cat > $work/dd/file1b <<EOF
file1b change3
EOF

activit="dev_end the change"
aefinish 12 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate begin 217"
aegis -ib 12
test $? -eq 0 || no_result

activity="integrate the change 221"
aefinish 12 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="run aecp -ind 225"
mkdir ind
test $? -eq 0 || no_result

cd ind
test $? -eq 0 || no_result

aegis -cp -ind -delta-from-change 11 . > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cd ..
test -$? -eq 0 || no_result

activity="test aegis.conf exists 238"
test -f ind/aegis.conf || fail

activity="test file1a exists 241"
test -f ind/file1a || fail

activity="test file not exists 244"
test ! -f ind/file || fail

activity="test file1b not exists 247"
test ! -f ind/file1b || fail

activity="check file1a content 250"
cat > ok <<EOF
file1a change2
EOF
test $? -eq 0 || no_result

diff ok ind/file1a
test $? -eq 0 || no_result

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
