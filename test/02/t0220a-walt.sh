#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004-2008 Peter Miller
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

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

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
	echo "FAILED test of the dev dir style functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the dev dir style functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 91"
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

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
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
# test the dev dir style functionality
#
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="new project 128"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT


activity="project attributes 137"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 150"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -nbr -p $AEGIS_PROJECT 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="new change 164"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 174"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 178"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/fred \
	$work/test.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "date > derived1 && date > derived2";

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
development_directory_style =
{
        source_file_copy = true;
        derived_file_copy = true;
        during_build_only = true;
};
integration_directory_style =
{
        source_file_copy = true;
        derived_file_copy = true;
        during_build_only = true;
};
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="build 217"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 221"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 225"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 229"
$bin/aegis -ib -mini 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 233"
delta_dir=`$bin/aegis -cd`
test -f $delta_dir/aegis.conf || fail
test -f $delta_dir/fred || fail
test -f $delta_dir/barney || fail

activity="integrate build 239"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 243"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 247"
delta_dir=`$bin/aegis -cd`
test -f $delta_dir/aegis.conf || fail
test -f $delta_dir/fred || fail
test -f $delta_dir/barney || fail

activity="integrate pass 253"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 257"
test -f $work/proj.dir/baseline/aegis.conf || fail
test -f $work/proj.dir/baseline/fred || fail
test -f $work/proj.dir/baseline/barney || fail

##

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

activity="new change 267"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf 1 -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 277"
$bin/aegis -db 1 -dir $work/test.1.C001 >  log 2>&1
if test $? -ne 0; then cat log; no_result; fi

$bin/aegis -cp 1 $work/test.1.C001/fred -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

echo 'append something' >> $work/test.1.C001/fred
if test $? -ne 0; then no_result; fi

activity="build 287"
$bin/aegis -build 1  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 291"
$bin/aegis -diff 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="dev_end 295"
$bin/aegis -dev_end 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="ibegin 299"
$bin/aegis -ib  1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check dir content 303"
delta_dir=`$bin/aegis -cd -c 1`
test -f $delta_dir/fred || fail

activity="build 307"
$bin/aegis -build 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 311"
delta_dir=`$bin/aegis -cd -c 1`
test -f $delta_dir/fred || fail
test -f $delta_dir/derived1 || fail
test -f $delta_dir/derived2 || fail

activity="diff  317"
$bin/aegis -diff 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 321"
$bin/aegis -ipass 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 325"
test -f $work/proj.dir/branch.1/baseline/fred || fail
test -f $work/proj.dir/branch.1/baseline/derived1 || fail
test -f $work/proj.dir/branch.1/baseline/derived2 || fail

###

activity="new change 332"
cat > caf << 'fubar'
brief_description = "two";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf 2 -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 342"
$bin/aegis -db 2 -dir $work/test.1.C002 >  log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="source remove 346"
$bin/aegis -rm -c 2 $work/test.1.C002/barney -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 350"
$bin/aegis -build 2  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 354"
$bin/aegis -diff 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="dev_end 358"
$bin/aegis -dev_end 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="ibegin 362"
$bin/aegis -ib  2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check dir content 366"
delta_dir=`$bin/aegis -cd -c 2`
test -f $delta_dir/aegis.conf && fail
test -f $delta_dir/barney && fail
test -f $delta_dir/fred || fail

activity="build 372"
$bin/aegis -build 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 376"
delta_dir=`$bin/aegis -cd -c 2`
test -f $delta_dir/aegis.conf && fail
test -f $delta_dir/barney && fail
test -f $delta_dir/fred || fail
test -f $delta_dir/derived1 || fail
test -f $delta_dir/derived2 || fail

activity="diff  384"
$bin/aegis -diff 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 388"
$bin/aegis -ipass 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 392"
test -f $work/proj.dir/branch.1/baseline/aegis.conf && fail
test -f $work/proj.dir/branch.1/baseline/barney && fail
test -f $work/proj.dir/branch.1/baseline/fred || fail
test -f $work/proj.dir/branch.1/baseline/derived1 || fail
test -f $work/proj.dir/branch.1/baseline/derived2 || fail
#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
