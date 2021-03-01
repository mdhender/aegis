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

TEST_SUBJECT="unlink before open functionality"

# load up standard prelude and test functions
. test_funcs

AEGIS_PROJECT=example
export AEGIS_PROJECT

workproj=$work/proj
workchan=$work/chan

#
# make a new project
#
activity="new project 34"
aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 41"
cat > tmp << 'end'
description = "A bogus project created to test the unlink before open "
    "functionality.";
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
activity="staff 58"
aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 69"
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
aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new files 87"
aegis -nf  $workchan/bogus1 -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nf  $workchan/aegis.conf -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus1 << 'end'
OLD CONTENT
bogus1, line 1
bogus1, line 2
end
if test $? -ne 0 ; then no_result; fi

#
# Save the content of bogus1 for future use
#
cp $workchan/bogus1 $work/bogus2
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

diff_command = "set +e; $diff $orig $i > $out; test $$? -le 1";
merge_command = "(diff3 -e $i $orig $mr | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $i > $out";
patch_diff_command = "set +e; $diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="finish the change 130"
aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 137"
aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish integration
#
activity="integrate the change 144"
aefinish -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cp -al $workproj $work/proj2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="create new project 151"
aegis -npr -p example2 -keep -dir $work/proj2 -verb > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

aegis -list c -p example -unf > example.changes 2>log
if test $? -ne 0; then cat log; no_result; fi

aegis -list c -p example2 -unf > example2.changes 2>log
if test $? -ne 0; then cat log; no_result; fi

diff example.changes example2.changes
if test $? -ne 0; then no_result; fi

activity="change description 164"
cat > tmp << 'end'
brief_description = "The first change (modified)";
cause = internal_enhancement;
end
if test $? -ne 0 ; then no_result; fi

aegis -ca 1 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check description 174"
aegis -ca -l 1 -p example -unf > example.changes 2>log
if test $? -ne 0; then cat log; no_result; fi

aegis -ca -l 1 -p example2 -unf > example2.changes 2>log
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
1c1
< brief_description = "The first change (modified)";
---
> brief_description = "The first change";
EOF
if test $? -ne 0; then no_result; fi

diff example.changes example2.changes > difference
if test $? -ne 1; then no_result; fi

diff ok difference
if test $? -ne 0; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
