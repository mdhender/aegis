#!/bin/sh
#
# aegis - The "aegis" program.
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

TEST_SUBJECT="pfs functionality"

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

activity="create branch 70"
aegis -new-br 1 -p $AEGIS_PROJECT > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

AEGIS_PROJECT=foo.1

#
# create a new change
#
activity="new change 79"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 1 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 88"
aegis -dev-begin 1 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 95"
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
cache_project_file_list_for_each_delta = true;
fubar
test $? -eq 0 || no_result

activity="new file 121"
aegis -new-file $work/dd/fred
test $? -eq 0 || no_result

cat > $work/dd/fred <<EOF
Hi, I'm Fred
EOF
test $? -eq 0 || no_result

activity="new file 130"
aegis -new-file $work/dd/barney
test $? -eq 0 || no_result

cat > $work/dd/fred <<EOF
Hi, I'm Barney
EOF
test $? -eq 0 || no_result

#
# finish development of the change
#
activity="finish dev 142"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# integrate the change
#
activity="finis int 149"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

AEGIS_PROJECT=foo

activity="develope end the branch 155"
aegis -develop-end 1
test $? -eq 0 || no_result

activity="integrate the branch 159"
aefinish 1 > LOG 2>&1
test $? -eq 0 || no_result

#
# create a branch
#
activity="new branch 166"
aegis -p foo -nbr 2 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

AEGIS_PROJECT=foo.2
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 176"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 10 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 185"
aegis -dev-begin 10 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 192"
aegis -new-file $work/dd/dino
test $? -eq 0 || no_result

cat > $work/dd/dino <<EOF
Hi, I'm Dino
EOF
test $? -eq 0 || no_result

activity="finish the change 201"
aefinish 10 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="integrate the change 205"
aefinish 10 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# create a new change
#
activity="new change 212"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 1 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 221"
aegis -dev-begin 1 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 228"
aegis -copy-file $work/dd/barney
test $? -eq 0 || no_result

cat > $work/dd/barney <<EOF
Hi, I'm Barney and she is my wife Betty.
EOF
test $? -eq 0 || no_result

activity="develop_end the change 237"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# Do not integrate change 1, yet.
#

#
# The second change
#
activity="new change 248"
cat > cattr << 'end'
brief_description = "Second change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 2 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 257"
aegis -dev-begin 2 -dir $work/dd2
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 264"
aegis -copy-file $work/dd2/fred
test $? -eq 0 || no_result

cat > $work/dd2/fred <<EOF
Hi, I'm Barney and she is my wife Wilma.
EOF
test $? -eq 0 || no_result

activity="finish the change 273"
aefinish 2 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="integrate the 2nd change 277"
aefinish 2 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="integrated the 1st change 281"
aefinish 1 > LOG 2>&1
test $? -eq 0 || no_result

#
# The third change
#
activity="new change 288"
cat > cattr << 'end'
brief_description = "Third change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 3 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 297"
aegis -dev-begin 3 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 304"
aegis -new-file $work/dd/bambam
test $? -eq 0 || no_result

cat > $work/dd/bambam <<EOF
Hi, I'm Bambam
EOF
test $? -eq 0 || no_result

activity="finish the change 313"
aefinish 3 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="integrate the 3rd change 317"
aefinish 3 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# Run aediff
#
activity="aediff 324"
cat > ok <<'EOF'
0a1
> Hi, I'm Barney and she is my wife Betty.
EOF
test $? -eq 0 || no_result

aediff -p foo.2 -delta 2 -delta 3 barney > barney.diff
test $? -eq 0 || fail

diff ok barney.diff
test $? -eq 0 || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
