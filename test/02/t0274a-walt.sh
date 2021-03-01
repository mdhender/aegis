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

TEST_SUBJECT="time-safe history"

# load up standard prelude and test functions
. test_funcs

#
# test the functionality
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

aegis -new-file $work/dd/fred -nl
test $? -eq 0 || no_result

cat > $work/dd/fred <<EOF
Hi, my name if Fred
EOF
test $? -eq 0 || no_result

#
# finish development of the change
#
activity="finish dev 125"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# integrate the change
#
activity="finis int 132"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# create a new change
#
activity="new change 139"
cat > cattr << 'end'
brief_description = "Second change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 2 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 148"
aegis -dev-begin 2 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 155"
aegis -copy-file $work/dd/fred
test $? -eq 0 || no_result

cat >> $work/dd/fred <<EOF
My name is Freeed!
EOF
test $? -eq 0 || no_result

activity="finish the change 164"
aefinish 2 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="integrate the change 168"
aefinish 2 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# create a new change
#
activity="new change 175"
cat > cattr << 'end'
brief_description = "Third change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 3 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 184"
aegis -dev-begin 3 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 191"
aegis -copy-file $work/dd/fred
test $? -eq 0 || no_result

cat >> $work/dd/fred <<EOF
My name is Freeed!
EOF
test $? -eq 0 || no_result

activity="finish the change 200"
aefinish 3 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="integrate the change 204"
aefinish 3 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="check 208"
#aegis -cp -independent -base-rel fred -o /dev/null
#test $? -eq 0 || no_result

#
# Save manually the history_get_command
#
activity="remember history_get_command 215"
aegis 1 -change-attr \
    aegis:history_get_command='aesvt -check-out -edit ${quote $e} -history ${quote $h} -f ${quote $o}'
test $? -eq 0 || no_result

aegis 2 -change-attr \
    aegis:history_get_command='aesvt -check-out -edit ${quote $e} -history ${quote $h} -f ${quote $o}'
test $? -eq 0 || no_result

activity="run aeannotate 224"
aeannotate fred -verbose 2> log  > /dev/null
test $? -eq 0 || no_result

activity="check aeannotate log 228"
cat > ok <<EOF
aeannotate: project "foo": change 1: history get fred
aeannotate: cd $work/proj/history
aeannotate: project "foo": change 2: history get fred
aeannotate: cd $work/proj/baseline
EOF
test $? -eq 0 || no_result

diff ok log
test $? -eq 0 || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
