#!/bin/sh
#
# aegis - project change supervisor.
# Copyright (C) 2008 Peter Miller
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

TEST_SUBJECT="aexml project-files-by-delta"

# load up standard prelude and test functions
. test_funcs

#
# some variables to make things earier to read
#
worklib=$work/lib
workproj=$work/example.proj
workchan=$work/example.chan
tmp=$work/tmp
export tmp

AEGIS_PROJECT=example
export AEGIS_PROJECT

#
# make a new project
#
activity="new project 40"
aegis -newpro $AEGIS_PROJECT -version - -dir $workproj -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# change project attributes
#
activity="project attributes 47"
cat > $tmp << 'TheEnd'
description = "aexml testing";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
develop_end_action = goto_awaiting_integration;
default_test_exemption = true;
default_test_regression_exemption = true;
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -proatt -f $tmp -proj $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# add staff
#
activity="new developer 65"
aegis -newdev $USER -proj $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="new integrator 69"
aegis -newint $USER -proj $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# create a new change
#
activity="new change 76"
cat > $tmp << 'TheEnd'
brief_description = "first";
description = "The first change";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -new_change 1 -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# begin development of the change
#
activity="develop begin 90"
aegis -devbeg -c 1 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the new files to the change
#
activity="new file 97"
aegis -new_file $workchan/aegis.conf -nl -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# Add the config file content.
cat > $workchan/aegis.conf << 'fubar'
build_command = "exit 0";
diff_command = "exit 0";
merge_command = "exit 0";
history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;
fubar
if test $? -ne 0 ; then no_result; fi

activity="new file 115"
aegis -new_file $workchan/a $workchan/b $workchan/c $workchan/d -nl -c 1 -v \
        > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo first > $workchan/a
if test $? -ne 0 ; then no_result; fi

echo first > $workchan/b
if test $? -ne 0 ; then no_result; fi

echo first > $workchan/c
if test $? -ne 0 ; then no_result; fi

echo first > $workchan/d
if test $? -ne 0 ; then no_result; fi

activity="finish development 132"
aefinish -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate 136"
aefinish -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# create another new change
#
activity="new change 145"
cat > $tmp << 'TheEnd'
brief_description = "second";
description = "The second change set";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -new_change 2 -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# begin development of the change
#
workchan=$work/example.chan.2
activity="develop begin 160"
aegis -devbeg -c 2 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a file
#
activity="new file 167"
aegis -new-file -c 2 $workchan/e -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo second > $workchan/e
if test $? -ne 0 ; then no_result; fi

#
# modify a file
#
activity="copy file 177"
aegis -copy-file -c 2 $workchan/a -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo second > $workchan/a
if test $? -ne 0 ; then no_result; fi

#
# remove a file
#
activity="remove file 187"
aegis -remove-file -c 2 $workchan/b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish the change
#
activity="finish development 194"
aefinish 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate the change
#
activity="integrate 201"
aefinish 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# create another new change
#
activity="new change 210"
cat > $tmp << 'TheEnd'
brief_description = "third";
description = "The third change set";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -new-change 3 -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# begin development of the change
#
workchan=$work/example.chan.3
activity="develop begin 225"
aegis -devbeg -c 3 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a file
#
activity="new file 232"
aegis -new-file -c 3 $workchan/f -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo third > $workchan/f
if test $? -ne 0 ; then no_result; fi

#
# modify a file
#
activity="modify file 242"
aegis -copy-file -c 3 $workchan/c -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo third > $workchan/c
if test $? -ne 0 ; then no_result; fi

#
# remove a file
#
activity="remove file 252"
aegis -remove-file -c 3 $workchan/d -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish the change
#
activity="finish development 259"
aefinish 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate the change
#
activity="integrate 266"
aefinish 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# run aexml project-files-by-delta to obtain historical project file lists
#

cat > ok << 'fubar'
<fstate>
<src>
<fstate_src>
<file_name>a</file_name>
<edit>
<revision>1</revision>
</edit>
</fstate_src>
<fstate_src>
<file_name>aegis.conf</file_name>
<edit>
<revision>1</revision>
</edit>
<usage>config</usage>
</fstate_src>
<fstate_src>
<file_name>b</file_name>
<edit>
<revision>1</revision>
</edit>
</fstate_src>
<fstate_src>
<file_name>c</file_name>
<edit>
<revision>1</revision>
</edit>
</fstate_src>
<fstate_src>
<file_name>d</file_name>
<edit>
<revision>1</revision>
</edit>
</fstate_src>
</src>
</fstate>
fubar
if test $? -ne 0 ; then no_result; fi

aexml project-files-by-delta -c 1 -o test.out.1
if test $? -ne 0 ; then fail; fi

sed '/uuid/d' < test.out.1 > test.out
if test $? -ne 0 ; then no_result; fi

diff ok test.out
if test $? -ne 0 ; then fail; fi

# --------------------------------------------------------------------------

#
# run aexml project-files-by-delta to obtain historical project file lists
#

cat > ok << 'fubar'
<fstate>
<src>
<fstate_src>
<file_name>a</file_name>
<action>modify</action>
<edit>
<revision>2</revision>
</edit>
<edit_origin>
<revision>1</revision>
</edit_origin>
</fstate_src>
<fstate_src>
<file_name>aegis.conf</file_name>
<edit>
<revision>1</revision>
</edit>
<usage>config</usage>
</fstate_src>
<fstate_src>
<file_name>b</file_name>
<action>remove</action>
<edit_origin>
<revision>1</revision>
</edit_origin>
</fstate_src>
<fstate_src>
<file_name>c</file_name>
<edit>
<revision>1</revision>
</edit>
</fstate_src>
<fstate_src>
<file_name>d</file_name>
<edit>
<revision>1</revision>
</edit>
</fstate_src>
<fstate_src>
<file_name>e</file_name>
<edit>
<revision>1</revision>
</edit>
</fstate_src>
</src>
</fstate>
fubar
if test $? -ne 0 ; then no_result; fi

aexml project-files-by-delta -c 2 -o test.out.1
if test $? -ne 0 ; then fail; fi

sed '/uuid/d' < test.out.1 > test.out
if test $? -ne 0 ; then no_result; fi

diff ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
