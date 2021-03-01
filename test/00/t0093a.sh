#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 1999, 2000, 2005-2008, 2012 Peter Miller
#       Copyright (C) 2008, 2010 Walter Franzini
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

TEST_SUBJECT="aedist functionality"

# load up standard prelude and test functions
. test_funcs

check_it()
{
        sed     -e "s|$work|...|g" \
                -e 's|= [0-9][0-9]*; /.*|= TIME;|' \
                -e "s/\"$USER\"/\"USER\"/g" \
                -e 's/19[0-9][0-9]/YYYY/' \
                -e 's/20[0-9][0-9]/YYYY/' \
                -e 's/node = ".*"/node = "NODE"/' \
                -e 's/crypto = ".*"/crypto = "GUNK"/' \
                -e 's|"text/plain"|"text/plain; charset=us-ascii"|' \
                < $2 > $work/sed.out
        if test $? -ne 0; then no_result; fi
        diff -b $1 $work/sed.out
        if test $? -ne 0; then fail; fi
}

worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 53"
aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 60"
cat > $tmp << 'end'
description = "A bogus project created to test the aedist functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 75"
aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

aegis -nbr -p foo 4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

aegis -nbr -p foo.4 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=foo.4.2 ; export AEGIS_PROJECT

#
# create a new change
#
activity="new change 94"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
aegis -nc 1 -f $tmp -p foo.4.2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 112"
aegis -nf $workchan/main.c -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.c << 'end'
/* $Id$ */
int main() { test(); exit(0); return 0; }
end
if test $? -ne 0 ; then no_result; fi

aegis -nf -no-uuid -config $workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
link_integration_directory = true;
create_symlinks_before_build = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
        echo '1,$$p' ) | ed - $mr > $out";
history_put_trashes_file = warn;
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 146"
aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 153"
aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# now make a distribution set
#
activity="aedist -send 160"
aedist -send -o test.out -ndh > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# set the path, so that the aegis command that aedist invokes
# is fromthe same test set as the aedist command itself.
#
PATH=${bin}:$PATH
export PATH

#
# now receive it
#
activity="aedist -receive 174"
aedist -receive -f test.out -dir $workchan.2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > ok <<EOF
attribute =
[
        {
                name = "aeipass-option:assign-file-uuid";
                value = "false";
        },
        {
                name = "usage";
                value = "config";
        },
        {
                name = "content-type";
                value = "text/plain; charset=us-ascii";
        },
];
EOF
if test $? -ne 0; then no_result; fi

aegis -file-attr -list -c 10 -base-rel aegis.conf > aegis.conf.fa
if test $? -ne 0; then no_result; fi

check_it ok aegis.conf.fa
if test $? -ne 0; then fail; fi

#
# the things tested in this test, worked
#
pass


# vim: set ts=8 sw=4 et :
