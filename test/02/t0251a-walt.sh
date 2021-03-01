#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2007, 2008 Peter Miller
#	Copyright (C) 2007, 2008 Walter Franzini
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
#	along with this program; if not, see
#	<http://www.gnu.org/licenses/>.
#
#   This test was provided by Ralph Smith.
#   It addresses a bug whereby a file, which is created and renamed within
#   a single branch, is not picked up by 'aedist -send -es' from
#   subsequent branches when the first branch has been integrated.
#
#   While we are at it, we check the integrity of aetar output for
#   such projects, and make sure that such aedist archives can be
#   received properly.
#
#   The tests are combined because it is time consuming to set up the
#   repository.
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
	log_file_preference = never; \
	default_development_directory = \"$work\"; "
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

# This tells aeintegratq that it is being used by a test.
AEGIS_TEST_DIR=$work
export AEGIS_TEST_DIR

here=`pwd`
if test $? -ne 0 ; then exit 2; fi


parch=
test "$1" != "" && parch="$1/"
bin="$here/${parch}bin"
if test "$EXEC_SEARCH_PATH" != ""
then
    tpath=
    hold="$IFS"
    IFS=":$IFS"
    for tpath2 in $EXEC_SEARCH_PATH
    do
	tpath=${tpath}${tpath2}/${parch}bin:
    done
    IFS="$hold"
    PATH=${tpath}${PATH}
else
    PATH=${bin}:${PATH}
fi
export PATH

AEGIS_DATADIR=$here/lib
export AEGIS_DATADIR

#
# set the path, so that the aegis command that aepatch/aedist invokes
# is from the same test set as the aepatch/aedist command itself.
#
PATH=${bin}:$PATH
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
	rm -rf $work
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

activity="create test directory 134"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi
workchan=$work/change-dir

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

activity="new project 175"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="project attributes 183"
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

activity="staff 196"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 204"
cat > caf << 'fubar'
brief_description = "zero";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 214"
$bin/aegis -db 10 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 218"
$bin/aegis -nf $workchan/aegis.conf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/aegis.conf << 'fubar'
build_command = "date > derived1 && date > derived2";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_create_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
if test $? -ne 0 ; then no_result; fi

activity="build 241"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 245"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 249"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

actitity="integrate begin"
$bin/aegis -ib -c 10 -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="integrate 257"
$bin/aefinish -p $AEGIS_PROJECT -c 10 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

#
# Part 2: build nontrivial project branch
#
# create a file, and rename it in the same branch.
#
# We do not use the trunk because it's special, it does not have a
# parent, and this condition make it special also in the code.
#
activity="new branch 269"
$bin/aegis -nbr -p $AEGIS_PROJECT 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

proj=$AEGIS_PROJECT

activity="new change 278"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 10 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

workchan=$work/change-dir-10
activity="develop begin 289"
$bin/aegis -db 10 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 293"
$bin/aegis -nf  $workchan/fred $workchan/barney $workchan/betty \
  -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo fred-content > $workchan/fred
if test $? -ne 0 ; then no_result; fi

cp $workchan/fred fred.saved
if test $? -ne 0 ; then no_result; fi

echo barney-content > $workchan/barney
if test $? -ne 0 ; then no_result; fi

echo betty-content > $workchan/betty
if test $? -ne 0 ; then no_result; fi

activity="finish 310"
$bin/aefinish -c 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 314"
$bin/aegis -ib -c 10 -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="integrate 318"
$bin/aefinish -p $AEGIS_PROJECT -c 10 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 20 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

workchan=$work/change-dir-20
activity="develop begin 327"
$bin/aegis -db 20 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="rename 331"
$bin/aegis -mv  $workchan/barney $workchan/dino -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo dino-content >> $workchan/dino
if test $? -ne 0 ; then no_result; fi

cp $workchan/dino dino.saved
if test $? -ne 0 ; then no_result; fi

activity="rename 341"
$bin/aegis -mv  $workchan/betty $workchan/hoppy -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="finish 345"
$bin/aefinish -c 20 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 349"
$bin/aegis -ib -c 20 -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="integrate 353"
$bin/aefinish -p $AEGIS_PROJECT -c 20 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="new change 357"
$bin/aegis -nc 30 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 361"
$bin/aegis -db 30 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify file 365"
$bin/aegis -cp -c 30 $workchan/hoppy
if test $? -ne 0 ; then cat log; no_result; fi

echo hoppy-content >> $workchan/hoppy
if test $? -ne 0 ; then no_result; fi

cp $workchan/hoppy hoppy.saved
if test $? -ne 0 ; then no_result; fi

activity="finish 375"
$bin/aefinish -c 30 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 379"
$bin/aegis -ib -c 30 -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="integrate 383"
$bin/aefinish -p $AEGIS_PROJECT -c 30 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="aedist 387"
$bin/aedist -send -p $AEGIS_PROJECT -bl -ndh -naa -comp_alg=none \
	-o dist0.ae > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="branch develop end 392"
$bin/aegis -de -p test -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 396"
$bin/aegis -ib -p test -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="branch integrate 400"
$bin/aefinish -p test -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="check project content 404"
cat > ok <<EOF
aegis.conf
dino
fred
hoppy
EOF

$bin/aelpf -p test > $work/proj.list
if test $? -ne 0; then no_result; fi

diff ok proj.list
if test $? -ne 0; then no_result; fi

#
# Part 3: start new branch, from which we export
#
activity="new branch 421"
$bin/aegis -nbr -p test 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=test.2
export AEGIS_PROJECT

proj=$AEGIS_PROJECT

# make the new branch nontrivial
activity="new change 431"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf 1 -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

workchan=$work/change-dir-1
activity="develop begin 442"
$bin/aegis -db 1 -dir $workchan >  log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="create file 446"
$bin/aegis -nf $workchan/pebbles -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo two > $workchan/pebbles
if test $? -ne 0 ; then no_result; fi

activity="move files 453"
$bin/aegis -mv $workchan/fred $workchan/wilma -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify files 457"
$bin/aegis -cp $workchan/hoppy -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo more-hoppy-content >> $workchan/hoppy
if test $? -ne 0 ; then no_result; fi

cp $workchan/hoppy new.hoppy.saved
if test $? -ne 0 ; then no_result; fi

activity="finish 467"
$bin/aefinish -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 471"
$bin/aegis -ib -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate 475"
$bin/aefinish -p $AEGIS_PROJECT -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check project content 479"
cat > ok <<EOF
aegis.conf
dino
hoppy
pebbles
wilma
EOF
if test $? -ne 0; then no_result; fi

$bin/aelpf -p $AEGIS_PROJECT > proj.list
if test $? -ne 0; then no_result; fi

diff ok proj.list
if test $? -ne 0; then no_result; fi

activity="aetar 495"
$bin/aetar -send -p $AEGIS_PROJECT -bl -es -comp_alg=none -app tarred \
 > dist1.tar
if test $? -ne 0 ; then cat log; no_result; fi

activity="check aetar content 500"
cat > ok <<EOF
tarred/aegis.conf
tarred/dino
tarred/hoppy
tarred/pebbles
tarred/wilma
EOF
if test $? -ne 0; then no_result; fi

tar tf dist1.tar > proj.list
if test $? -ne 0; then no_result; fi

diff ok proj.list
if test $? -ne 0; then fail; fi

activity="open aetar file 516"
tar xf dist1.tar
if test $? -ne 0 ; then cat log; no_result; fi

activity="check file 520"
cmp -s tarred/dino dino.saved > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check file 524"
cmp -s tarred/hoppy new.hoppy.saved > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check file 528"
cmp -s tarred/wilma fred.saved > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="aedist 532"
$bin/aedist -send -p $AEGIS_PROJECT -bl -es -ndh -naa -comp_alg=none \
	-o dist1.ae > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="aedist 537"
$bin/aedist -send -p test -c 2 -es -ndh -naa -comp_alg=none \
	-o dist2.ae > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the archive content 542"
$bin/test_cpio -list -file $work/dist1.ae > $work/content 2>&1
if test $? -ne 0; then no_result; fi

cat > $work/ok <<EOF
etc/project-name
etc/change-number
etc/change-set
src/aegis.conf
src/dino
src/hoppy
src/pebbles
src/wilma
EOF

cmp $work/ok $work/content
if test $? -ne 0; then fail; fi

activity="open aedist file 560"
$bin/test_cpio -extract -cd $work -file $work/dist1.ae > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check file 564"
cmp -s src/dino dino.saved > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check file 568"
cmp -s src/hoppy new.hoppy.saved > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check file 572"
cmp -s src/wilma fred.saved > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

# Part 4: import the changeset into a new project

activity="new project 578"
$bin/aegis -npr testx -version - -v -dir $work/projx.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=testx

activity="aepa 585"
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi;

activity="staff 589"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# avoid collisions
workchan=$work/change-dirx

activity="aedist -rec 600"
$bin/aedist -rec -p $AEGIS_PROJECT -c 10 -f dist0.ae \
  -notroj -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="integrate begin 605"
$bin/aegis -ib -c 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate 609"
$bin/aefinish -p $AEGIS_PROJECT -c 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# acid test: can we receive an ES changeset?
activity="aedist -rec 614"
$bin/aedist -rec -p $AEGIS_PROJECT -c 11 -f dist1.ae \
  -troj -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check file 619"
cmp -s $workchan/dino dino.saved > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check file 623"
cmp -s $workchan/hoppy new.hoppy.saved > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="aedbu 627"
$bin/aegis -dbu -p $AEGIS_PROJECT -c 11 \
  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# and again, with a branch changeset
activity="aedist -rec 633"
$bin/aedist -rec -p $AEGIS_PROJECT -c 12 -f dist2.ae \
  -troj -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check file 638"
cmp -s $workchan/dino dino.saved > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check file 642"
cmp -s $workchan/hoppy new.hoppy.saved > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
