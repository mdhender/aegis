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

TEST_SUBJECT="file names"

# load up standard prelude and test functions
. test_funcs

#
# make a new project
#
activity="new project 28"
aegis -newpro foo -version "" -dir $work/proj -lib $work/lib
if test $? -ne 0 ; then fail; fi

AEGIS_PROJECT=foo
export AEGIS_PROJECT

#
# change project attributes
#
activity="project attributes 38"
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

#
# create a new change
#
activity="new change 83"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 1 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 92"
aegis -dev-begin 1 -dir $work/dd1
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 99"
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
posix_filename_charset = false;
shell_safe_filenames = false;
allow_white_space_in_filenames = true;
allow_non_ascii_filenames = true;
fubar
test $? -eq 0 || no_result

activity="create file 128"
aegis -nf "$work/dd1/barney and fred"
test $? -eq 0 || fail

activity="new file 132"

#
# Now we create a file with a really ugly file name, the base64 trick
# is used to avoid:
# 1) the need to use echo -e to print the unicode character since it
#    may be not available everywhere;
# 2) the need to relax the aede-policy configuration to accept files
#    with unicode characters.
#
# The content of the file can be recreated with:
#
#   echo -e 'il nome del file \xc3\xa8 brutto' | test_base64 -o
#
activity="ugly file name 146"
cat > filename <<EOF
Content-Type: application/x-aegis-test
Content-Transfer-Encoding: base64

aWwgbm9tZSBkZWwgZmlsZSDDqCBicnV0dG8K
EOF
test $? -eq 0 || no_result

ufn=`test_base64 -i filename`

aegis -nf "$work/dd1/$ufn"
test $? -eq 0 || fail

activity="check file list 158"
cat > ok.1 <<EOF
aegis.conf
barney and fred
il nome del file \x00C3\x00A8 brutto
EOF
test $? -eq 0 || no_result

cat > ok.2 <<EOF
aegis.conf
barney and fred
$ufn
EOF
test $? -eq 0 || no_result

export LANG=C
export LC_ALL=C
aegis -l cf -ter > file_list
test $? -eq 0 || no_result

diff ok.1 file_list > /dev/null 2>/dev/null && pass

diff ok.2 file_list
test $? -eq 0 || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass

# vim: set ts=8 sw=4 et :
