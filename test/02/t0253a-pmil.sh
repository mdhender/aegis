#!/bin/sh
#
# aegis - project change supervisor.
# Copyright (C) 2008 Peter Miller
# Copyright (C) 2008 Walter Franzini
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

TEST_SUBJECT="aefinish vs branch"

# load up standard prelude and test functions
. test_funcs

#
# make a new project
#
activity="new project 29"
aegis -newpro foo -version "" -dir $work/proj -lib $work/lib
if test $? -ne 0 ; then fail; fi

AEGIS_PROJECT=foo
export AEGIS_PROJECT

#
# change project attributes
#
activity="project attributes 39"
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

activity="new developer 62"
aegis -newdev $USER
test $? -eq 0 || no_result
aegis -new-reviewer $USER
test $? -eq 0 || no_result
aegis -new-integrator $USER
test $? -eq 0 || no_result

#
# create a new change
#
activity="new change 73"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 1 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 82"
aegis -dev-begin 1 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 89"
aegis -new-file $work/dd/aegis.conf $work/dd/barney
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

echo first > $work/dd/barney
test $? -eq 0 || no_result

#
# finish development of the change
#
activity="finish dev 116"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# integrate the change
#
activity="finis int 123"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

# --------------------------------------------------------------------------

#
# we create a branch, because the bug is about branches with a
# transparent file.
#
activity="new branch 133"
aegis -new-branch -p foo 2
test $? -eq 0 || no_result

AEGIS_PROJECT=foo.2
export AEGIS_PROJECT

# --------------------------------------------------------------------------

#
# create a new change, to copy the file we will make transparent
#
activity="new change 145"
cat > cattr << 'end'
brief_description = "Third change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 3 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 154"
aegis -dev-begin 3 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="copy file 161"
aegis -copy-file $work/dd/barney
test $? -eq 0 || no_result

echo third-third-third > $work/dd/barney
test $? -eq 0 || no_result

#
# finish development of the change
#
activity="finish dev 171"
aefinish 3 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# integrate the change
#
activity="finish int 178"
aefinish 3 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

# --------------------------------------------------------------------------

#
# create a new change, to make the file transparent
#
activity="new change 187"
cat > cattr << 'end'
brief_description = "Fourth change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new-change 4 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 196"
aegis -dev-begin 4 -dir $work/dd
test $? -eq 0 || no_result

#
# transparent file
#
activity="transparent file 203"
aegis -make-transparent $work/dd/barney
test $? -eq 0 || no_result

#
# finish development of the change
#
activity="finish dev 210"
aefinish 4 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# integrate the change
#
activity="finish int 217"
aefinish 4 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

# --------------------------------------------------------------------------
#
# This is what we wanted to actually test
#
# The problem was that it would segfault
#
aefinish -p foo -c 2 > LOG 2>&1
if test $? -ne 0
then
    cat LOG
    fail
fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
