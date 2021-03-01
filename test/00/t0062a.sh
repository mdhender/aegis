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
	echo "FAILED test of the symlink branch functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of the symlink branch functionality ($activity)" 1>&2
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

worklib=$work/lib
workproj=$work/proj
workchan=$work/chan

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the symlink branch functionality
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
activity="create branch 130"
$bin/aegis -nbr 2 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a change to the trunk
#
activity="new change 137"
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

activity="develop begin 150"
$bin/aegis -db 10 -p test -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 154"
$bin/aegis -nf $workchan/aegis.conf -c 10 -p test -lib $worklib > log 2>&1
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
create_symlinks_before_build = true;
fubar
if test $? -ne 0 ; then no_result; fi

# this one checks symlinks
activity="build 181"
$bin/aegis -b -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="difference 185"
$bin/aegis -diff -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 189"
$bin/aegis -de -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 193"
$bin/aegis -rpass 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 197"
$bin/aegis -ib 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# this one checks symlinks
activity="build 202"
$bin/aegis -b -c 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="integrate pass 206"
$bin/aegis -ipass 10 -p test -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a change to branch 4
#
activity="new change 213"
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

# this one checks symlinks
activity="develop begin 227"
$bin/aegis -db 10 -p test.4 -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="new file 231"
$bin/aegis -nf $workchan/second -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
echo second > $workchan/second
if test $? -ne 0 ; then no_result; fi

# this one checks symlinks
activity="build 238"
$bin/aegis -b -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="difference 242"
$bin/aegis -diff -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 246"
$bin/aegis -de -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 250"
$bin/aegis -rpass 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 254"
$bin/aegis -ib 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# this one checks symlinks
activity="build 259"
$bin/aegis -b -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="difference 263"
$bin/aegis -diff -c 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 267"
$bin/aegis -ipass 10 -p test.4 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a change to branch 4.2
#
activity="new change 274"
cat > ca << 'fubar'
brief_description = "third";
description = "This is the third change.";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f ca -p test.4.2 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# this one checks symlinks
activity="develop begin 285"
$bin/aegis -db 10 -p test.4.2 -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# This is what it's all about,
#	make sure that the aegis.conf file is linked
#
# The diff will fail if the files are (a) absent, or (b) point to limbo,
# or (c) different
#
diff $workproj/baseline/aegis.conf $workchan/aegis.conf
if test $? -ne 0 ; then fail; fi
diff $workproj/branch.4/baseline/second $workchan/second
if test $? -ne 0 ; then fail; fi

# symlinks to ,D file should not be present
test -h $workproj/branch.4/baseline/second,D && fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
