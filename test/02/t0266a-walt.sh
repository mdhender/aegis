#!/bin/sh
#
# aegis - The "aegis" program.
# Copyright (C) 2008 Walter Franzini
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

TEST_SUBJECT="aedist -send vs. aemv"

# load up standard prelude and test functions
. test_funcs

check_it()
{
        sed     -e "s|$work|...|g" \
                -e 's|= [0-9][0-9]*; /.*|= TIME;|' \
                -e "s/\"$USER\"/\"USER\"/g" \
                -e 's/uuid = ".*"/uuid = "UUID"/' \
                -e 's/19[0-9][0-9]/YYYY/' \
                -e 's/20[0-9][0-9]/YYYY/' \
                -e 's/node = ".*"/node = "NODE"/' \
                -e 's/crypto = ".*"/crypto = "GUNK"/' \
                < $2 > $work/sed.out
        if test $? -ne 0; then no_result; fi
        diff -b $1 $work/sed.out
        if test $? -ne 0; then fail; fi
}

#
# test the aedist/aemv functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 52"
aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 59"
cat > tmp << 'end'
description = "A bogus project created to test the aedist/aemv "
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
activity="staff 76"
aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 87"
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
activity="new files 105"
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
activity="finish the change 148"
aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 155"
aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish integration
#
activity="integrate the change 162"
aefinish -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="send c1 166"
aedist -send -c 1 -o $work/c1.ae -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Alter the project to simulate the integration with an older
# (pre-UUID) Aegis version
#
cat > clean_uuid.sed <<EOF
/uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";/d
EOF
if test $? -ne 0 ; then no_result; fi

activity="uuid reset 001.fs 179"
sed -f clean_uuid.sed < $work/proj/info/change/0/001.fs \
    > $work/proj/info/change/0/001.fs.new
if test $? -ne 0 ; then no_result; fi

mv $work/proj/info/change/0/001.fs.new $work/proj/info/change/0/001.fs
if test $? -ne 0 ; then no_result; fi

activity="uuid reset 001.pfs 187"
sed -f clean_uuid.sed < $work/proj/info/change/0/001.pfs \
    > $work/proj/info/change/0/001.pfs.new
if test $? -ne 0 ; then no_result; fi

mv $work/proj/info/change/0/001.pfs.new $work/proj/info/change/0/001.pfs
if test $? -ne 0 ; then no_result; fi

activity="UUID reset trunk.fs 195"
sed -f clean_uuid.sed < $work/proj/info/trunk.fs \
    > $work/proj/info/trunk.fs.new
if test $? -ne 0 ; then no_result; fi

mv $work/proj/info/trunk.fs.new $work/proj/info/trunk.fs
if test $? -ne 0 ; then no_result; fi

#
# create a new change
#
activity="new change 206"
cat > tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
aegis -nc 2 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 218"
aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Use the second change to move bogus1 to bogus2
#
activity="move files 225"
aegis -c 2 -mv -baserel bogus1 bogus2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Modify bogus2
#
activity="modify bogus2 232"
cat > $workchan/bogus2 <<EOF
NEW CONTENT
this file is now named bogus2
EOF
if test $? -ne 0 ; then no_result; fi

#
# develop_end the change
#
activity="aefinish 242"
aefinish > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

acctivity="integrate begin"
aegis -ibegin 2 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate the change 250"
aefinish -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check 002.fs 254"
cat > ok <<EOF
src =
[
        {
                file_name = "bogus1";
                action = remove;
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = source;
                move = "bogus2";
        },
        {
                file_name = "bogus2";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = source;
                move = "bogus1";
        },
];
EOF
if test $? -ne 0 ; then no_result; fi

check_it ok $work/proj/info/change/0/002.fs

activity="send the completed change 287"
aedist -send -c 2 -ndh -nmh -out $work/c02.ae > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

#
# make a new project
#
AEGIS_PROJECT=foo
export AEGIS_PROJECT

workproj=$work/foo.proj

activity="new project 299"
aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 306"
cat > tmp << 'end'
description = "A bogus project created to test the aedist/aemv "
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
activity="staff 323"
aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="receive c1 331"
aedist -rec -p $AEGIS_PROJECT -f $work/c1.ae -no_trojan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate the change 335"
aegis -ib -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="finish the change 339"
aefinish 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="receive c2 343"
aedist -rec -p $AEGIS_PROJECT -f $work/c02.ae -c 2 \
    -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > ok <<EOF
NEW CONTENT
this file is now named bogus2
EOF
if test $? -ne 0 ; then cat log; no_result; fi

cmp $workchan/bogus2 ok
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
