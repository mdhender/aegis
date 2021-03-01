#!/bin/sh
#
# aegis - The "aegis" program.
# Copyright (C) 2010 Walter Franzini
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

TEST_SUBJECT="aeib vs. ,D files"

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

activity="create branch 69"
aegis -new-br 1 -p $AEGIS_PROJECT > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

AEGIS_PROJECT=foo.1

#
# create a new change
#
activity="new change 78"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 1 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 87"
aegis -dev-begin 1 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 94"
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

integrate_begin_exceptions = [ "*" ];

development_directory_style =
{
    source_file_symlink = true;
    source_file_link = true;
    source_file_copy = true;

    derived_file_copy = true;
    derived_at_start_only = true;
};
fubar
test $? -eq 0 || no_result

activity="create barney 131"
aegis -new-file $work/dd/barney
test $? -eq 0 || no_result

cat > $work/dd/barney <<EOF
Hi my name is Barney
EOF
test $? -eq 0 || no_result

activity="dev_end the change 140"
aefinish 1 > log 2>&1
test $? -eq 0 || no_result

activity="integrate begin 144"
aegis -ibegin 1
test $? -eq 0 || no_result

activity="complete the change 148"
aefinish 1 > log 2>&1
test $? -eq 0 || no_result

activity="new branch 152"
aegis -new-branch 10 -p $AEGIS_PROJECT
test $? -eq 0 || no_result

AEGIS_PROJECT=${AEGIS_PROJECT}.10

activity="new change 158"
cat > cattr << 'end'
brief_description = "Second change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result

aegis -new-change -p $AEGIS_PROJECT 1 -f cattr
test $? -eq 0 || no_result

aegis -dev-begin 1 -dir $work/dd
test $? -eq 0 || no_result

aegis -rm $work/dd/barney
test $? -eq 0 || no_result

aefinish 1 > log 2>&1
test $? -eq 0 || no_result

aegis -ib 1
test $? -eq 0 || no_result

aefinish 1 > log 2>&1
test $? -eq 0 || no_result

test -f $work/proj/branch.1/branch.10/baseline/barney,D
test $? -eq 0 || no_result

activity="new change 186"
cat > cattr << 'end'
brief_description = "Third change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result

activity="new change 193"
aegis -new-change -p $AEGIS_PROJECT 2 -f cattr
test $? -eq 0 || no_result

activity="devbegin change 197"
aegis -dev-begin 2 -dir $work/dd
test $? -eq 0 || no_result

activity="new file empty 201"
aegis -nf $work/dd/empty
test $? -eq 0 || no_result

activity="dev_end the change"
aefinish 2 > log 2>&1
test $? -eq 0 || no_result

activity="begin change integration 209"
aegis -ib 2
test $? -eq 0 || no_result

activity="finish the change 213"
aefinish 2 > log 2>&1
test $? -eq 0 || no_result

#test -f $work/proj/branch.1/branch.10/baseline/barney,D
#test $? -eq 0 || fail

aegis -dev-end -p foo.1 10
test $? -eq 0 || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
