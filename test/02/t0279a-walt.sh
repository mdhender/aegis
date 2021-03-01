#!/bin/sh
#
# aegis - The "aegis" program.
# Copyright (C) 2009, 2010 Walter Franzini
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

TEST_SUBJECT="aeip vs. aegis:history_get_command"

rcs -V > /dev/null
if test $? -ne 0
then
    echo
    echo 'Your system does not appear to have RCS installed.'
    echo 'This test is therefore -assumed- to pass.'
    echo
    exit 0
fi

# load up standard prelude and test functions
. test_funcs

check_it()
{
    sed -e "s|$work|...|g" \
        -e 's|= [0-9][0-9]*; /.*|= TIME;|' \
        -e "s/\"$USER\"/\"USER\"/g" \
        -e 's/uuid = ".*"/uuid = "UUID"/' \
        -e 's/19[0-9][0-9]/YYYY/' \
        -e 's/20[0-9][0-9]/YYYY/' \
        -e 's/node = ".*"/node = "NODE"/' \
        -e 's/crypto = ".*"/crypto = "GUNK"/' \
        < $2 > $work/sed.out
    if test $? -ne 0; then no_result; fi
    diff $1 $work/sed.out
    if test $? -ne 0; then fail; fi
}

#
# test the functionality
#
activity="create the project 54"
aegis -npr test -version - -v -dir $work/proj.dir \
    -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="project attributes 62"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
test $? -eq 0 || no_result

aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 76"
aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

aegis -nbr -p test 1
test $? -eq 0 || no_result

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

activity="create the first change 90"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
test $? -eq 0 || no_result

aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 101"
aegis -db 10 -v  -dir $work/dd/ > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 105"
aegis -nf -config -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd001 \
    $work/dd/aegis.conf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`

sed -e "s|{TAB}|$TAB|g" > $work/dd/aegis.conf << 'fubar'
build_command = "echo no build required";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input} UUID=${change uuid}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
{TAB}echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
test $? -eq 0 || no_result

activity="new file 129"
aegis -nf $work/dd/00dummy \
    -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd002
test $? -eq 0 || no_result

echo '/* dummy */' > $work/dd/00dummy

activity="develop end 136"
aefinish 10 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the change 140"
aefinish 10 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

sed -e "s|{TAB}|$TAB|g" > ok <<'EOF'
brief_description = "one";
description = "one";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
{TAB}"unspecified",
];
copyright_years =
[
{TAB}YYYY,
];
attribute =
[
{TAB}{
{TAB}{TAB}name = "aegis:history_get_command";
{TAB}{TAB}value = "aesvt -check-out -edit ${quote $edit} -history ${quote $history} -f ${quote $output}";
{TAB}},
];
state = completed;
given_test_exemption = true;
given_regression_test_exemption = true;
delta_number = 1;
delta_uuid = "UUID";
history =
[
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = new_change;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = develop_begin;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = develop_end_2ai;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = integrate_begin;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = integrate_pass;
{TAB}{TAB}who = "USER";
{TAB}},
];
uuid = "UUID";
EOF
test $? -eq 0 || no_result

check_it ok $work/proj.dir/info/change/0/001.branch/0/010

activity="check fstate 205"
sed -e "s|{TAB}|$TAB|g" > ok <<'EOF'
src =
[
{TAB}{
{TAB}{TAB}file_name = "00dummy";
{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}action = create;
{TAB}{TAB}edit =
{TAB}{TAB}{
{TAB}{TAB}{TAB}revision = "1";
{TAB}{TAB}{TAB}encoding = none;
{TAB}{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}};
{TAB}{TAB}usage = source;
{TAB}},
{TAB}{
{TAB}{TAB}file_name = "aegis.conf";
{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}action = create;
{TAB}{TAB}edit =
{TAB}{TAB}{
{TAB}{TAB}{TAB}revision = "1";
{TAB}{TAB}{TAB}encoding = none;
{TAB}{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}};
{TAB}{TAB}usage = config;
{TAB}},
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj.dir/info/change/0/001.branch/0/010.fs

activity="second change 239"
cat > caf << 'fubar'
brief_description = "change the history tool (2nd change)";
cause = internal_enhancement;
fubar
test $? -eq 0 || no_result

aegis -nc 2 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 249"
aegis -db 2 -v  -dir $work/dd > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="change the history tool 253"
aegis -cp $work/dd/aegis.conf
test $? -eq 0 || no_result

sed -e "s|{TAB}|$TAB|g" > $work/dd/aegis.conf << 'fubar'
build_command = "echo no build required";
history_get_command = "co -r${quote $e} -p ${quote $h,v} > ${quote $o}";
history_put_command = "ci -u -d -M -m${quote UUID: ${change uuid}} -t/dev/null ${quote $i} ${quote $h,v};"
        " rcs -U ${quote $h,v}";
history_query_command =  "rlog -r ${quote $h,v} "
        "| awk '/^revision/ {print $$2}'";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
{TAB}echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
test $? -eq 0 || no_result

activity="develop end 273"
aefinish 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the change 277"
aefinish 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="check cstate 281"
sed -e "s|{TAB}|$TAB|g" > ok <<'EOF'
brief_description = "change the history tool (2nd change)";
description = "change the history tool (2nd change)";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
{TAB}"unspecified",
];
copyright_years =
[
{TAB}YYYY,
];
attribute =
[
{TAB}{
{TAB}{TAB}name = "aegis:history_get_command";
{TAB}{TAB}value = "co -r${quote $e} -p ${quote $h,v} > ${quote $o}";
{TAB}},
];
state = completed;
given_test_exemption = true;
given_regression_test_exemption = true;
delta_number = 2;
delta_uuid = "UUID";
project_file_command_sync = 290;
history =
[
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = new_change;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = develop_begin;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = develop_end_2ai;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = integrate_begin;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = integrate_pass;
{TAB}{TAB}who = "USER";
{TAB}},
];
uuid = "UUID";
EOF
test $? -eq 0 || no_result

check_it ok $work/proj.dir/info/change/0/001.branch/0/002

activity="check fstate 344"
sed -e "s|{TAB}|$TAB|g" > ok <<'EOF'
src =
[
{TAB}{
{TAB}{TAB}file_name = "aegis.conf";
{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}action = modify;
{TAB}{TAB}edit =
{TAB}{TAB}{
{TAB}{TAB}{TAB}revision = "1.1";
{TAB}{TAB}{TAB}encoding = none;
{TAB}{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}};
{TAB}{TAB}edit_origin =
{TAB}{TAB}{
{TAB}{TAB}{TAB}revision = "1";
{TAB}{TAB}{TAB}encoding = none;
{TAB}{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}};
{TAB}{TAB}usage = config;
{TAB}},
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj.dir/info/change/0/001.branch/0/002.fs

activity="third change 372"
activity="new change 373"
cat > caf << 'fubar'
brief_description = "the third change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
aegis -nc 3 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 382"
aegis -db 3 -v  -dir $work/dd > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="change the build command 386"
aegis -cp $work/dd/aegis.conf
test $? -eq 0 || no_result

sed -e "s|{TAB}|$TAB|g" > $work/dd/aegis.conf << 'fubar'
build_command = "exit 0";

history_get_command = "co -r${quote $e} -p ${quote $h,v} > ${quote $o}";
history_put_command = "ci -u -d -M -m${quote UUID: ${change uuid}} -t/dev/null ${quote $i} ${quote $h,v};"
        " rcs -U ${quote $h,v}";
history_query_command =  "rlog -r ${quote $h,v} "
        "| awk '/^revision/ {print $$2}'";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
{TAB}echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
test $? -eq 0 || no_result

activity="develop end 407"
aefinish 3 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the change 411"
aefinish 3 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Get the difference between aegis.conf at delta1 and delta2 since two
# different history commands needs to be used.
#
activity="check aegis.conf diff 419"
aediff -delta 1 -delta 2 aegis.conf > diff 2> log
test $? -eq 0 || no_result

sed -e 's/X$//' > ok <<'EOF'
2,7c2,6X
< X
< history_get_command = "aesvt -check-out -edit ${quote $edit} "X
<     "-history ${quote $history} -f ${quote $output}";X
< history_put_command = "aesvt -check-in -history ${quote $history} "X
<     "-f ${quote $input} UUID=${change uuid}";X
< history_query_command = "aesvt -query -history ${quote $history}";X
---X
> history_get_command = "co -r${quote $e} -p ${quote $h,v} > ${quote $o}";X
> history_put_command = "ci -u -d -M -m${quote UUID: ${change uuid}} -t/dev/null ${quote $i} ${quote $h,v};"X
>         " rcs -U ${quote $h,v}";X
> history_query_command =  "rlog -r ${quote $h,v} "X
>         "| awk '/^revision/ {print $$2}'";X
EOF
test $? -eq 0 || no_result

diff diff ok
test $? -eq 0 || fail

#
# Now close the first branch.
#
AEGIS_PROJECT=test

activity="close test.1 448"
aegis -dev-end 1
test $? -eq 0 || no_result

aegis -ib 1
test $? -eq 0 || no_result

aefinish 1 > log 2>&1
test $? -eq 0 || no_result

sed -e "s|{TAB}|$TAB|g" > ok <<'EOF'
brief_description = "The \"test\" program, branch 1.";
description = "The \"test\" program, branch 1.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
{TAB}"unspecified",
];
copyright_years =
[
{TAB}YYYY,
];
attribute =
[
{TAB}{
{TAB}{TAB}name = "aegis:history_get_command";
{TAB}{TAB}value = "co -r${quote $e} -p ${quote $h,v} > ${quote $o}";
{TAB}},
];
state = completed;
delta_number = 1;
delta_uuid = "UUID";
history =
[
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = new_change;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = develop_begin;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = develop_end_2ai;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = integrate_begin;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = integrate_pass;
{TAB}{TAB}who = "USER";
{TAB}},
];
uuid = "UUID";
branch =
{
{TAB}umask = 022;
{TAB}developer_may_review = true;
{TAB}developer_may_integrate = true;
{TAB}reviewer_may_integrate = true;
{TAB}developers_may_create_changes = false;
{TAB}default_test_exemption = true;
{TAB}default_test_regression_exemption = true;
{TAB}skip_unlucky = false;
{TAB}compress_database = false;
{TAB}develop_end_action = goto_awaiting_integration;
{TAB}history =
{TAB}[
{TAB}{TAB}{
{TAB}{TAB}{TAB}delta_number = 1;
{TAB}{TAB}{TAB}change_number = 10;
{TAB}{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}{TAB}when = TIME;
{TAB}{TAB}{TAB}is_a_branch = no;
{TAB}{TAB}},
{TAB}{TAB}{
{TAB}{TAB}{TAB}delta_number = 2;
{TAB}{TAB}{TAB}change_number = 2;
{TAB}{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}{TAB}when = TIME;
{TAB}{TAB}{TAB}is_a_branch = no;
{TAB}{TAB}},
{TAB}{TAB}{
{TAB}{TAB}{TAB}delta_number = 3;
{TAB}{TAB}{TAB}change_number = 3;
{TAB}{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}{TAB}when = TIME;
{TAB}{TAB}{TAB}is_a_branch = no;
{TAB}{TAB}},
{TAB}];
{TAB}change =
{TAB}[
{TAB}{TAB}2,
{TAB}{TAB}3,
{TAB}{TAB}10,
{TAB}];
{TAB}administrator =
{TAB}[
{TAB}{TAB}"USER",
{TAB}];
{TAB}developer =
{TAB}[
{TAB}{TAB}"USER",
{TAB}];
{TAB}reviewer =
{TAB}[
{TAB}{TAB}"USER",
{TAB}];
{TAB}integrator =
{TAB}[
{TAB}{TAB}"USER",
{TAB}];
{TAB}default_development_directory = "...";
{TAB}minimum_change_number = 10;
{TAB}reuse_change_numbers = true;
{TAB}minimum_branch_number = 1;
{TAB}protect_development_directory = false;
};
EOF
test $? -eq 0 || no_result

check_it ok $work/proj.dir/info/change/0/001

activity="check fstate 581"
sed -e "s|{TAB}|$TAB|g" > ok <<'EOF'
src =
[
{TAB}{
{TAB}{TAB}file_name = "00dummy";
{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}action = create;
{TAB}{TAB}edit =
{TAB}{TAB}{
{TAB}{TAB}{TAB}revision = "1.1";
{TAB}{TAB}{TAB}encoding = none;
{TAB}{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}};
{TAB}{TAB}edit_origin =
{TAB}{TAB}{
{TAB}{TAB}{TAB}revision = "1";
{TAB}{TAB}{TAB}encoding = none;
{TAB}{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}};
{TAB}{TAB}usage = source;
{TAB}},
{TAB}{
{TAB}{TAB}file_name = "aegis.conf";
{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}action = create;
{TAB}{TAB}edit =
{TAB}{TAB}{
{TAB}{TAB}{TAB}revision = "1.2";
{TAB}{TAB}{TAB}encoding = none;
{TAB}{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}};
{TAB}{TAB}edit_origin =
{TAB}{TAB}{
{TAB}{TAB}{TAB}revision = "1";
{TAB}{TAB}{TAB}encoding = none;
{TAB}{TAB}{TAB}uuid = "UUID";
{TAB}{TAB}};
{TAB}{TAB}usage = config;
{TAB}},
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj.dir/info/change/0/001.fs

activity="second change 627"
cat > caf << 'fubar'
brief_description = "the second change";
cause = internal_enhancement;
fubar
test $? -eq 0 || no_result

aegis -nc 2 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 637"
aegis -db 2 -v  -dir $work/dd > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="change the build command 641"
aegis -cp $work/dd/aegis.conf
test $? -eq 0 || no_result

sed -e "s|{TAB}|$TAB|g" > $work/dd/aegis.conf << 'fubar'
build_command = "/bin/true";

history_get_command = "co -r${quote $e} -p ${quote $h,v} > ${quote $o}";
history_put_command = "ci -u -d -M -m${quote UUID: ${change uuid}} -t/dev/null ${quote $i} ${quote $h,v};"
        " rcs -U ${quote $h,v}";
history_query_command =  "rlog -r ${quote $h,v} "
        "| awk '/^revision/ {print $$2}'";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
{TAB}echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
test $? -eq 0 || no_result

activity="develop end 662"
aefinish 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the change 666"
aefinish 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

sed -e "s|{TAB}|$TAB|g" > ok <<'EOF'
brief_description = "the second change";
description = "the second change";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
{TAB}"unspecified",
];
copyright_years =
[
{TAB}YYYY,
];
attribute =
[
{TAB}{
{TAB}{TAB}name = "aegis:history_get_command";
{TAB}{TAB}value = "co -r${quote $e} -p ${quote $h,v} > ${quote $o}";
{TAB}},
];
state = completed;
given_test_exemption = true;
given_regression_test_exemption = true;
delta_number = 2;
delta_uuid = "UUID";
project_file_command_sync = 1;
history =
[
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = new_change;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = develop_begin;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = develop_end_2ai;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = integrate_begin;
{TAB}{TAB}who = "USER";
{TAB}},
{TAB}{
{TAB}{TAB}when = TIME;
{TAB}{TAB}what = integrate_pass;
{TAB}{TAB}who = "USER";
{TAB}},
];
uuid = "UUID";
EOF
test $? -eq 0 || no_result

check_it ok $work/proj.dir/info/change/0/002

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
