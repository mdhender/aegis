#!/bin/sh
#
# aegis - project change supervisor.
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

TEST_SUBJECT="debian:#610984"

# load up standard prelude and test functions
. test_funcs

#
# test the functionality
#
activity="setup project 179"
aegis -npr test -v -dir $work/test -lib $work/lib -vers - > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="project attributes 189"
cat > paf << 'EOF'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
EOF
if test $? -ne 0 ; then no_result; fi
aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 200"
aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# The first change set to add all the Aegis machinery.
#
activity="(10) new change 208"
cat > caf << 'fubar'
brief_description = "setup";
description = "Project Setup";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
aegis -nc -f caf -v -p $AEGIS_PROJECT \
    > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(10) develop begin 219"
aegis -db 10 -dir $work/test.C010 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(10) new file 223"
aegis -nf $work/test.C010/aegis.conf \
    -uuid 00112233-4455-6677-8899-aabbccddeeff \
    -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "exit 0";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
        echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
if test $? -ne 0 ; then no_result; fi

# can only set the uuid after all file mods are done
aegis -ca -c 10 -uuid 10101010-1010-1010-1010-101010101010
if test $? -ne 0 ; then no_result; fi

activity="(10) build 285"
aegis -b -c 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(10) diff 289"
aegis -diff -c 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(10) develop end 293"
aegis -de -c 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(10) review pass 297"
aegis -rpass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(10) integrate begin 301"
aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(10) integrate build 305"
aegis -b -c 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(10) integrate pass 309"
aegis -ipass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# second change set to add files "f1" and "f3"
#
activity="(11) new change 208"
cat > caf << 'fubar'
brief_description = "nf f1 f3";
description = "New files 'f1' and 'f3'.";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
aegis -nc 11 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(11) develop begin 219"
aegis -db 11 -dir $work/test.C011 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(11) new file 223"
aegis -nf $work/test.C011/f1 \
    -uuid f1f1f1f1-f1f1-f1f1-f1f1-f1f1f1f1f1f1 \
    -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C011/f1 << 'fubar'
i
one
1
fubar
if test $? -ne 0 ; then no_result; fi

aegis -nf $work/test.C011/f3 \
    -uuid f3f3f3f3-f3f3-f3f3-f3f3-f3f3f3f3f3f3 \
    -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C011/f3 << 'fubar'
iii
three
3
fubar
if test $? -ne 0 ; then no_result; fi

# can only set the uuid after all file mods are done
aegis -ca -c 11 -uuid 11111111-1111-1111-1111-111111111111
if test $? -ne 0 ; then no_result; fi

activity="(11) build 285"
aegis -b -c 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(11) diff 289"
aegis -diff -c 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(11) develop end 293"
aegis -de -c 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(11) review pass 297"
aegis -rpass 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(11) integrate begin 301"
aegis -ib 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(11) integrate build 305"
aegis -b -c 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(11) integrate pass 309"
aegis -ipass 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a branch, since the bug related to aede(1)ing a branch.
#
activity="(1) new branch"
aegis -nbr -p $AEGIS_PROJECT 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

#
# third change set, to remove "f3" from the branch
#
activity="(12) new change 208"
cat > caf << 'fubar'
brief_description = "rm f3";
description = "Remove file 'f3'.";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
aegis -nc 12 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(12) develop begin 219"
aegis -db 12 -dir $work/test.1.C012 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(12) remove file 223"
aegis -rm $work/test.1.C012/f3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# can only set the uuid after all file mods are done
aegis -ca -c 12 -uuid 12121212-1212-1212-1212-121212121212
if test $? -ne 0 ; then no_result; fi

activity="(12) build 285"
aegis -b -c 12 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(12) diff 289"
aegis -diff -c 12 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(12) develop end 293"
aegis -de -c 12 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(12) review pass 297"
aegis -rpass 12 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(12) integrate begin 301"
aegis -ib 12 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(12) integrate build 305"
aegis -b -c 12 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(12) integrate diff 305"
aegis -diff -c 12 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(12) integrate pass 309"
aegis -ipass 12 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# fourth change set, to rename 'f1' to 'f3' on the branch
#
activity="(13) new change 208"
cat > caf << 'fubar'
brief_description = "mv f1 f3";
description = "Rename file 'f1' to 'f3'.";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
aegis -nc 13 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(13) develop begin 219"
aegis -db 13 -dir $work/test.1.C013 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(13) move file 223"
aegis -mv $work/test.1.C013/f1 $work/test.1.C013/f3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
echo moved >> $work/test.1.C013/f3
if test $? -ne 0 ; then no_result; fi

# can only set the uuid after all file mods are done
aegis -ca -c 13 -uuid 13131313-1313-1313-1313-131313131313
if test $? -ne 0 ; then no_result; fi

activity="(13) build 285"
aegis -b -c 13 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(13) diff 289"
aegis -diff -c 13 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(13) develop end 293"
aegis -de -c 13 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(13) review pass 297"
aegis -rpass 13 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(13) integrate begin 301"
aegis -ib 13 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(13) integrate build 305"
aegis -b -c 13 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(13) integrate diff 305"
aegis -diff -c 13 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(13) integrate pass 309"
aegis -ipass 13 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Now we are ready for the operation under test:
# we want to aede the branch
#
AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="(1) end branch"
aegis -de -p $AEGIS_PROJECT -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="(1) review pass 297"
aegis -rpass 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(1) integrate begin 301"
aegis -ib 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(1) integrate build 305"
aegis -b -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(1) integrate diff 305"
aegis -diff -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="(1) integrate pass 309"
aegis -ipass 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# make sure the UUID of the moved file is correct
#
cat > ok << 'fubar'
src =
[
    {
        file_name = "aegis.conf";
        uuid = "UUID";
        action = create;
        edit =
        {
            revision = "1";
            encoding = none;
            uuid = "10101010-1010-1010-1010-101010101010";
        };
        edit_origin =
        {
            revision = "1";
            encoding = none;
            uuid = "10101010-1010-1010-1010-101010101010";
        };
        usage = config;
        file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
    },
    {
        file_name = "f1";
        uuid = "f1f1f1f1-f1f1-f1f1-f1f1-f1f1f1f1f1f1";
        action = remove;
        edit =
        {
            revision = "1";
            encoding = none;
            uuid = "11111111-1111-1111-1111-111111111111";
        };
        edit_origin =
        {
            revision = "1";
            encoding = none;
            uuid = "11111111-1111-1111-1111-111111111111";
        };
        usage = source;
        move = "f3";
        deleted_by = 1;
    },
    {
        file_name = "f3";
        uuid = "f1f1f1f1-f1f1-f1f1-f1f1-f1f1f1f1f1f1";
        action = create;
        edit =
        {
            revision = "3";
            encoding = none;
            uuid = "UUID";
        };
        edit_origin =
        {
            revision = "3";
            encoding = none;
            uuid = "UUID";
        };
        usage = source;
        file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
        diff_file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
    },
];
fubar
if test $? -ne 0 ; then no_result; fi

#
# The check_it function is useful for checking aegis' database,
# normalising the trickier stuff that changes with every test run.
# (For this test script, we leave the UUIDs alone, because we want to
# track what the file UUIDs do.)
#
# Usage: check_it expected actual
#
check_it()
{
    sed -e "s|$work|...|g" \
        -e 's|= [0-9][0-9]*; /.*|= TIME;|' \
        -e "s/\"$USER\"/\"USER\"/g" \
        -e 's/19[0-9][0-9]/YYYY/' \
        -e 's/20[0-9][0-9]/YYYY/' \
        -e 's/node = ".*"/node = "NODE"/' \
        -e 's/crypto = ".*"/crypto = "GUNK"/' \
        -e 's/uuid = ".*[4-9a-e].*"/uuid = "UUID"/' \
        < $2 | expand -t4 > $work/sed.out
    if test $? -ne 0; then no_result; fi
    diff $1 $work/sed.out
    if test $? -ne 0; then fail; fi
}

check_it ok $work/test/info/trunk.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
