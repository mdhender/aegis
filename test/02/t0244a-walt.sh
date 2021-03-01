#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2006, 2008, 2009 Walter Franzini
#       Copyright (C) 2007, 2008, 2012 Peter Miller
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program. If not, see
#       <http://www.gnu.org/licenses/>.
#

TEST_SUBJECT="aenpr -keep"

# load up standard prelude and test functions
. test_funcs

#
# test the aenpr -keep functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 39"
aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 46"
cat > tmp << 'end'
description = "A bogus project created to test the "
    "aenpr functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
end
if test $? -ne 0 ; then no_result; fi
aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch 60"
aegis -p example -nbr 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="new branch 64"
aegis -p example.1 -nbr 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Create an archive of the repository.
#
activity="create a cpio archive 71"
test_cpio -create -file $work/example.cpio -cd $workproj \
    `(cd $workproj && find . -type f)` > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

mkdir $work/proj2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Now we force the gid of the just created $work/proj2 directory
# otherwise the test will fail because the project directory does not
# follow the Aegis rules about file ownership.
#
# On BSD systems a newly created file gets the group id of the
# directory in which it is created.  Aegis tests are usually run under
# /tmp/, so the the gid of $work/proj2 is 0 and the same is true for
# the file we are going to create.
#
chgrp `id -g` $work/proj2
test $? -eq 0 || no_result

activity="extract the archive 92"
test_cpio -extract -file $work/example.cpio -cd $work/proj2 \
    > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="modify the trunk 97"
sed -e "s/$USER/${USER}not/g" < $work/proj2/info/trunk \
    > $work/proj2/info/trunk.new
if test $? -ne 0; then no_result; fi

mv $work/proj2/info/trunk.new $work/proj2/info/trunk > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="modify example.1 105"
sed -e "s/$USER/${USER}not/g" < $work/proj2/info/change/0/001 \
    > $work/proj2/info/change/0/001.new
if test $? -ne 0; then no_result; fi

mv $work/proj2/info/change/0/001.new $work/proj2/info/change/0/001 \
    > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="modify example.1.1 114"
sed -e "s/$USER/${USER}not/g" < $work/proj2/info/change/0/001.branch/0/001 \
    > $work/proj2/info/change/0/001.branch/0/001.new
if test $? -ne 0; then no_result; fi

mv $work/proj2/info/change/0/001.branch/0/001.new \
    $work/proj2/info/change/0/001.branch/0/001 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="create a new project 123"
aegis -npr -p example2 -keep -dir $work/proj2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
$USER
${USER}not
EOF
if test $? -ne 0; then no_result; fi

activity="check example2 133"
aegis -list admin -p example2 -ter > example2.adm
if test $? -ne 0; then no_result; fi

diff $work/ok $work/example2.adm
if test $? -ne 0; then fail; fi

activity="check example2.1 140"
aegis -list admin -p example2.1 -ter > example2.1.adm
if test $? -ne 0; then no_result; fi

diff $work/ok $work/example2.1.adm
if test $? -ne 0; then fail; fi

activity="check example2.1.1 147"
aegis -list admin -p example2.1.1 -ter > example2.1.1.adm
if test $? -ne 0; then no_result; fi

diff $work/ok $work/example2.1.1.adm
if test $? -ne 0; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
