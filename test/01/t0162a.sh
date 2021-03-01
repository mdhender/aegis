#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 2002, 2005-2008, 2011, 2012 Peter Miller
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
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
umask 022

LINES=24
export LINES
COLS=80
export COLS

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$
PAGER=cat
export PAGER
AEGIS_FLAGS="delete_file_preference = no_keep; \
 lock_wait_preference = always; \
 diff_preference = automatic_merge; \
 pager_preference = never; \
 persevere_preference = all; \
 log_file_preference = never;"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin=$here/${1-.}/bin

if test "$EXEC_SEARCH_PATH" != ""
then
    tpath=
    hold="$IFS"
    IFS=":$IFS"
    for tpath2 in $EXEC_SEARCH_PATH
    do
        tpath=${tpath}${tpath2}/${1-.}/bin:
    done
    IFS="$hold"
    PATH=${tpath}${PATH}
else
    PATH=${bin}:${PATH}
fi
export PATH

pass()
{
    set +x
    echo PASSED 1>&2
    cd $here
    find $work -type d -user $USER -exec chmod u+w {} \;
    rm -rf $work
    exit 0
}
fail()
{
    set +x
    echo "FAILED test of the aecp -ind functionality ($activity)" 1>&2
    cd $here
    find $work -type d -user $USER -exec chmod u+w {} \;
    rm -rf $work
    exit 1
}
no_result()
{
    set +x
    echo "NO RESULT when testing the aecp -ind functionality ($activity)" \
        1>&2
    cd $here
    find $work -type d -user $USER -exec chmod u+w {} \;
    rm -rf $work
    exit 2
}
trap \"no_result\" 1 2 3 15

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# make the directories
#
activity="working directory 96"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

worklib=$work/lib
workproj=$work/proj
workchan=$work/chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 122"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 129"
cat > $tmp << 'end'
description = "A bogus project created to test the aedist functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 144"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -nbr -p foo 4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=foo.4 ; export AEGIS_PROJECT

#
# create a new change
#
activity="new change 160"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p foo.4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 178"
$bin/aegis -nf $workchan/main.c $workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.c << 'end'
int main() { test(); exit(0); return 0; }
end
if test $? -ne 0 ; then no_result; fi

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
merge_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
    echo '1,$$p' ) | ed - $mr > $out";
history_put_trashes_file = warn;
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 208"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 215"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# develop end
#
activity="develop end 222"
$bin/aegis -de 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# review pass
#
activity="review pass 229"
$bin/aegis -rpass 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate
#
activity="integrate begin 236"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate diff 239"
$bin/aegis -diff 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate build 242"
$bin/aegis -b 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate pass 245"
$bin/aegis -ipass 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# end the branch
#
AEGIS_PROJECT=foo ; export AEGIS_PROJECT
activity="develop end 253"
$bin/aegis -de 4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="review pass 256"
$bin/aegis -rpass 4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate begin 259"
$bin/aegis -ib 4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate diff 262"
$bin/aegis -diff 4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate build 265"
$bin/aegis -b 4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate pass 268"
$bin/aegis -ipass 4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Now we see if we can copy the file.
#
$bin/aegis -cp -ind  -baserel main.c -p foo.4 -o test.out -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
