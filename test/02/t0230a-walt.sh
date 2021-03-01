#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2005 Walter Franzini;
#	All rights reserved.
#       Copyright (C) 2007 Peter Miller
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: Test the aedist -rec functionality
#
#   This test was provided by Ralph Smith.
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
	echo "FAILED test of the $TEST_FUNC functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work # WHILE DEBUGGING DO: echo work dir is $work
	exit 1
}
no_result()
{
	set +x
        echo "NO RESULT when testing the $TEST_FUNC functionality " \
            "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
warning()
{
	set +x
        echo "WARNING when testing the $TEST_FUNC functionality " \
            "($activity)" 1>&2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 99"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

#
# use the built-in error messages
#
#AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
#export AEGIS_MESSAGE_LIBRARY
#unset LANG
#unset LANGUAGE

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		-e 's/uuid = ".*"/uuid = "UUID"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

#
# test the aedist_branch functionality
#
# Part 1: set up trunk
#
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="new project 139"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="project attributes 147"
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
if test $? -ne 0 ; then cat log; no_result; fi;

activity="staff 160"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Part 2: build nontrivial project branch
#
# We do not use the trunk because it's special, it does not have a
# parent, and this condition make it special also in the code.
#
activity="new branch 174"
$bin/aegis -nbr -p $AEGIS_PROJECT 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

proj=$AEGIS_PROJECT

activity="new change 183"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 193"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 197"
$bin/aegis -nf $work/${proj}.C010/config $work/${proj}.C010/fred \
	$work/${proj}.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/${proj}.C010/config << 'fubar'
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
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/${proj}.C010/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/${proj}.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="build 224"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 228"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 232"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 236"
$bin/aegis -ib -mini 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the integration directory content 240"
delta_dir=`$bin/aegis -cd`
test -f $delta_dir/config || no_result
test -f $delta_dir/fred || no_result
test -f $delta_dir/barney || no_result

activity="integrate build 246"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 250"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the integration directory content 254"
delta_dir=`$bin/aegis -cd`
test -f $delta_dir/config || no_result
test -f $delta_dir/fred || no_result
test -f $delta_dir/barney || no_result

activity="integrate pass 260"
$bin/aegis -ipass -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 264"
bl_dir=$work/proj.dir/branch.1/baseline
test -f $bl_dir/config || no_result
test -f $bl_dir/fred || no_result
test -f $bl_dir/barney || no_result

#
# Part 3: start the source branch; export intermediate and later versions
#

activity="new branch 274"
$bin/aegis -nbr -p $proj 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=test.1.2
export AEGIS_PROJECT

proj=$AEGIS_PROJECT

activity="new change 283"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf 1 -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 293"
$bin/aegis -db 1 -dir $work/$proj.C001 >  log 2>&1
if test $? -ne 0; then cat log; no_result; fi

mkdir $work/$proj.C001/sub
if test $? -ne 0; then cat log; no_result; fi

$bin/aegis -nf $work/$proj.C001/sub/abel $work/$proj.C001/sub/baker \
  -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

echo 'initial content' >> $work/$proj.C001/sub/abel
if test $? -ne 0; then no_result; fi

echo 'initial content' >> $work/$proj.C001/sub/baker
if test $? -ne 0; then no_result; fi

activity="build 310"
$bin/aegis -build 1  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 314"
$bin/aegis -diff 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="dev_end 318"
$bin/aegis -dev_end 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="ibegin 322"
$bin/aegis -ib  1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check dir content 326"
delta_dir=`$bin/aegis -cd -c 1`
test -f $delta_dir/sub/abel || no_result
test -f $delta_dir/sub/baker || no_result

activity="build 331"
$bin/aegis -build 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 335"
delta_dir=`$bin/aegis -cd -c 1`
test -f $delta_dir/sub/abel || no_result
test -f $delta_dir/sub/baker || no_result
test -f $delta_dir/derived1 || no_result
test -f $delta_dir/derived2 || no_result

activity="diff  342"
$bin/aegis -diff 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 346"
$bin/aegis -ipass 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 350"
bl_dir=$work/proj.dir/branch.1/branch.2/baseline
test -f $bl_dir/sub/abel || no_result
test -f $bl_dir/sub/baker || no_result
test -f $bl_dir/derived1 || no_result
test -f $bl_dir/derived2 || no_result

activity="send the branch 357"
$bin/aedist -send -p test.1 -c 2 -es -ndh -cte=none -nocomp -o c1.ae -v \
  > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="new change 362"
cat > caf <<EOF
brief_description = "two";
cause = internal_enhancement;
test_baseline_exempt = true;
EOF
if test $? -ne 0; then no_result; fi

$bin/aegis -nc -p $AEGIS_PROJECT -f caf 20 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 373"
$bin/aegis -db 20 -dir $work/$proj.C020 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -nf $work/$proj.C020/wilma  -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

echo 'initial content' >> $work/$proj.C020/wilma
if test $? -ne 0; then no_result; fi


activity="diff 384"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 388"
$bin/aegis -build -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop_end 392"
$bin/aegis -dev_end 20 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 396"
$bin/aegis -ib 20 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 400"
$bin/aegis -build 20 -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 404"
$bin/aegis -diff 20 -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 408"
$bin/aegis -ipass 20 -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 412"
bl_dir=$work/proj.dir/branch.1/baseline
test -f $bl_dir/derived1 || no_result
test -f $bl_dir/derived2 || no_result

activity="new change 417"
cat > caf << 'fubar'
brief_description = "three";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf 2 -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 427"
$bin/aegis -db 2 -dir $work/$proj.C002 >  log 2>&1
if test $? -ne 0; then cat log; no_result; fi

$bin/aegis -mv -p $proj $work/$proj.C002/sub/abel \
  $work/$proj.C002/charlie -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

$bin/aegis -cp -p $proj $work/$proj.C002/sub/baker -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

echo 'append something' >> $work/$proj.C002/charlie
if test $? -ne 0; then no_result; fi

echo 'append something' >> $work/$proj.C002/sub/baker
if test $? -ne 0; then no_result; fi

activity="build 444"
$bin/aegis -build 2  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 448"
$bin/aegis -diff 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="dev_end 452"
$bin/aegis -dev_end 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="ibegin 456"
$bin/aegis -ib  2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check dir content 460"
delta_dir=`$bin/aegis -cd -c 2`
test -f $delta_dir/charlie || no_result

activity="build 464"
$bin/aegis -build 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 468"
delta_dir=`$bin/aegis -cd -c 2`
test -f $delta_dir/charlie || no_result
test -f $delta_dir/derived1 || no_result
test -f $delta_dir/derived2 || no_result

activity="diff  474"
$bin/aegis -diff 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 478"
$bin/aegis -ipass 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 482"
bl_dir=$work/proj.dir/branch.1/branch.2/baseline
test -f $bl_dir/charlie || no_result
test -f $bl_dir/derived1 || no_result
test -f $bl_dir/derived2 || no_result

activity="send the branch 488"
$bin/aedist -send -p test.1 -c 2 -es -ndh -cte=none -nocomp -o c2.ae -v \
  > log 2>&1
if test $? -ne 0; then cat log; fail; fi

#
# Part 4: start a new branch (stand-in for remote repo)
#         attempt to import the two changesets from earlier branch

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

proj=$AEGIS_PROJECT
activity="new branch 501"
$bin/aegis -nbr -p $proj 3 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=test.1.3
export AEGIS_PROJECT

proj=$AEGIS_PROJECT

activity="receive the branch 510"
$bin/aedist -rec -p $AEGIS_PROJECT -c 1 -no_troj -f c1.ae -v > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="ibegin 514"
$bin/aegis -ib  1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check dir content 518"
delta_dir=`$bin/aegis -cd -c 1`
test -f $delta_dir/sub/abel || no_result

activity="build 522"
$bin/aegis -build 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 526"
delta_dir=`$bin/aegis -cd -c 1`
test -f $delta_dir/sub/abel || no_result
test -f $delta_dir/derived1 || no_result
test -f $delta_dir/derived2 || no_result
test -f $delta_dir/sub/baker || no_result

activity="diff  533"
$bin/aegis -diff 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 537"
$bin/aegis -ipass 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 541"
bl_dir=$work/proj.dir/branch.1/branch.3/baseline
test -f $bl_dir/sub/abel || no_result
test -f $bl_dir/derived1 || no_result
test -f $bl_dir/derived2 || no_result

activity="receive the branch 547"
$bin/aedist -rec -p $AEGIS_PROJECT -c 2 -no_troj -f c2.ae \
        -v > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="ibegin 552"
$bin/aegis -ib  2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check dir content 556"
delta_dir=`$bin/aegis -cd -c 2`
test -f $delta_dir/charlie || no_result

activity="build 560"
$bin/aegis -build 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 564"
delta_dir=`$bin/aegis -cd -c 2`
test -f $delta_dir/charlie || no_result
test -f $delta_dir/derived1 || no_result
test -f $delta_dir/derived2 || no_result

activity="diff  570"
$bin/aegis -diff 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 574"
$bin/aegis -ipass 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi;

activity="check the baseline content 578"
bl_dir=$work/proj.dir/branch.1/branch.3/baseline
test -f $bl_dir/charlie || no_result
test -f $bl_dir/derived1 || no_result
test -f $bl_dir/derived2 || no_result


#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
