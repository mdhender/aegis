#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 1998, 2000, 2001, 2004-2008, 2012 Peter Miller
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

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
unset LINES
unset COLS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$
PAGER=cat
export PAGER
AEGIS_FLAGS="delete_file_preference = no_keep; \
        lock_wait_preference = always; \
        diff_preference = automatic_merge; \
        pager_preference = never; \
        persevere_preference = all; \
        log_file_preference = never; \
        default_project_directory = \"$work\"; \
        default_development_directory = \"$work\";"
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

check_it()
{
        sed     -e "s|$work|...|g" \
                -e 's|= [0-9][0-9]*; /.*|= TIME;|' \
                -e "s/\"$USER\"/\"USER\"/g" \
                -e 's/19[0-9][0-9]/YYYY/' \
                -e 's/20[0-9][0-9]/YYYY/' \
                -e 's/node = ".*"/node = "NODE"/' \
                -e 's/crypto = ".*"/crypto = "GUNK"/' \
                -e 's/uuid = ".*"/uuid = "UUID"/' \
                < $2 > $work/sed.out
        if test $? -ne 0; then no_result; fi
        diff -b $1 $work/sed.out
        if test $? -ne 0; then fail; fi
}

no_result()
{
        set +x
        echo "NO RESULT for test of the aecp -delta functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
}
fail()
{
        set +x
        echo "FAILED test of the aecp -delta functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
pass()
{
        set +x
        echo PASSED 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 0
}
trap \"no_result\" 1 2 3 15

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

AEGIS_PROJECT=test-4.2
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

#
# test the aecp -delta functionality
#
activity="new project 122"
$bin/aegis -npr test -version 4.2 -v -dir $work/test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 126"
cat > paf << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 137"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 145"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test-4.2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 155"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 159"
$bin/aegis -nf $work/test.4.2.C010/aegis.conf $work/test.4.2.C010/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.4.2.C010/aegis.conf << 'fubar'
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
echo hello > $work/test.4.2.C010/fred
if test $? -ne 0 ; then no_result; fi

activity="new test 181"
$bin/aegis -nt -v > log 2>&1
echo exit 0 > $work/test.4.2.C010/test/00/t0001a.sh
if test $? -ne 0 ; then no_result; fi
$bin/aegis -ca -f caf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 188"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 192"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 196"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 200"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 204"
$bin/aegis -rpass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 208"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 212"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 216"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate test 220"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 224"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# now remove a file
#
activity="new change 231"
cat > caf << 'fubar'
brief_description = "two";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test-4.2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 240"
$bin/aegis -db 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="remove file 244"
$bin/aegis -rm $work/test.4.2.C011/fred $work/test.4.2.C011/test/00/t0001a.sh -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 248"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 252"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 256"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 260"
$bin/aegis -rpass 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 264"
$bin/aegis -ib 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 268"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 272"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 276"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# now dopy an old delta
#
activity="new change 283"
cat > caf << 'fubar'
brief_description = "three";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test-4.2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 292"
$bin/aegis -db 12 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy historical file 296"
$bin/aegis -cp $work/test.4.2.C012 -delta 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi


activity="check change file state 301"
cat > ok << 'fubar'
src =
[
        {
                file_name = "aegis.conf";
                uuid = "UUID";
                action = modify;
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = config;
        },
        {
                file_name = "fred";
                uuid = "UUID";
                action = modify;
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = source;
        },
        {
                file_name = "test/00/t0001a.sh";
                uuid = "UUID";
                action = modify;
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = test;
        },
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/004.branch/0/002.branch/0/012.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
