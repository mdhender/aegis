#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1997, 1998, 2005-2008 Peter Miller
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program. If not, see
#	<http://www.gnu.org/licenses/>.
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
	echo 'FAILED test of the whiteout functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the whiteout functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

workproj=$work/proj
workchan=$work/chan
worklib=$work/lib

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the whiteout functionality
#
activity="new project 103"
$bin/aegis -npr test -vers '' -dir $workproj -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 107"
$bin/aegis -nd $USER -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="staff 110"
$bin/aegis -nrv $USER -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="staff 113"
$bin/aegis -ni $USER -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 117"
cat > pa << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f pa -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="create branch 127"
$bin/aegis -nbr 4 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a change to the trunk
#
activity="new change 134"
cat > ca << 'fubar'
brief_description = "first";
description = "This is the first change.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f ca -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 147"
$bin/aegis -db 10 -p test -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 151"
$bin/aegis -nf $workchan/aegis.conf $workchan/a $workchan/b/c $workchan/b/d \
	-c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/aegis.conf << 'fubar'
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

# build
activity="build 178"
$bin/aegis -b -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="difference 182"
$bin/aegis -diff -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 186"
$bin/aegis -de -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 190"
$bin/aegis -rpass 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 194"
$bin/aegis -ib 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# this one checks symlinks
activity="build 199"
$bin/aegis -b -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 203"
$bin/aegis -ipass 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a change to branch 4
#
activity="new change 210"
cat > ca << 'fubar'
brief_description = "first";
description = "This is the second change.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f ca -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop begin
workchan=$work/chan.10
activity="develop begin 224"
$bin/aegis -db 10 -p test.4 -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="remove file 228"
$bin/aegis -rm $workchan/b/c -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# build
activity="build 233"
$bin/aegis -b -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="difference 237"
$bin/aegis -diff -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 241"
$bin/aegis -de -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 245"
$bin/aegis -rpass 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 249"
$bin/aegis -ib 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# build
activity="build 254"
$bin/aegis -b -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="difference 258"
$bin/aegis -diff -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 262"
$bin/aegis -ipass 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a change to branch 4
#
activity="new change 269"
cat > ca << 'fubar'
brief_description = "third";
description = "This is the third change.";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f ca -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop begin
workchan=$work/chan.11
activity="develop begin 280"
$bin/aegis -db 11 -p test.4 -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# This is what it's all about,
#	make sure that we can do a directory copy of the "b" directory
#
activity="directory copy 288"
$bin/aegis -cp $workchan/b -p test.4 -c 11 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
