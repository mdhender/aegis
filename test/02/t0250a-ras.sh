#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004-2008 Peter Miller
#       Derived 2006 by Ralph Smith
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
# DESCRIPTION: This test checks that a derived file whose name coincides
#       with a previously removed VC file is propagated by the symlink
#       directory style.
#
# Part 1. general variables and functions
# DEBUG="yes" # undefined or empty for production
STYLE="symlink" # what to test: symlink, link, or copy

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
	default_development_directory = \"$work\";"
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

if [ "$STYLE"="symlink" ]; then
  test_flag="-h"
else
  test_flag="-f"
fi

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
	echo "FAILED test of the tricky symlink functionality\
 ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the tricky symlink functionality\
 ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 130"
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

#
# If the C++ compiler is called something other than "c++", as
# discovered by the configure script, create a shell script called
# "c++" which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
#
if test "$CXX" != "c++"
then
	cat >> $work/c++ << fubar
#!/bin/sh
exec ${CXX-g++} \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx $work/c++
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# test the tricky symlink functionality
#
# Part 2. set up a project
# files of note:
# fred - simple file
# barney - removed, later revived as derived file
# wilma - removed
# betty - converted to derived file in one change
# pebbles - removed, later revived as derived (in branch)
# bammbamm = removed in branch

AEGIS_PATH=$work/lib
export AEGIS_PATH
workchan=$work/change-dir

activity="new project 181"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT


activity="project attributes 190"
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

activity="staff 203"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG"; then echo "## first change:" >/tmp/aelog ; fi

activity="new change 213"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 10 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 223"
$bin/aegis -db 10 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 227"
$bin/aegis -nf $workchan/aegis.conf $workchan/fred $workchan/barney \
	$workchan/wilma $workchan/pebbles $workchan/bammbamm \
	$workchan/betty -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/aegis.conf << 'fubar'
build_command = "date > derived1 && date > derived2 && date > derived3";

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
cat >>$workchan/aegis.conf <<fubar
development_directory_style =
{
    source_file_$STYLE = true;
    derived_file_$STYLE = true;
};
integration_directory_style =
{
    source_file_$STYLE = true;
    derived_file_$STYLE = true;
};
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $workchan/fred
if test $? -ne 0 ; then no_result; fi

echo one > $workchan/barney
if test $? -ne 0 ; then no_result; fi

echo one > $workchan/wilma
if test $? -ne 0 ; then no_result; fi

echo one > $workchan/betty
if test $? -ne 0 ; then no_result; fi

echo one > $workchan/pebbles
if test $? -ne 0 ; then no_result; fi

echo one > $workchan/bammbamm
if test $? -ne 0 ; then no_result; fi

activity="build 279"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 283"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 287"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 291"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 295"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 299"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 303"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
$bin/aegis -list pf >>/tmp/aelog
echo "## BL contents:" >>/tmp/aelog
ls -lR `aesub -bl '$BL'` >>/tmp/aelog
echo "## second change:" >>/tmp/aelog
fi

##########################################
# Part 3. remove a source file
activity="new change 316"
cat > caf << 'fubar'
brief_description = "two";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f caf -p test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 326"
$bin/aegis -db 2 -dir $workchan -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
echo "## DD contents:" >>/tmp/aelog
ls -lR `aesub '$DD'` >>/tmp/aelog
fi

activity="check symlink 335"
if test ! $test_flag $workchan/barney ; then no_result; fi

activity="remove file 338"
$bin/aegis -rm $workchan/barney $workchan/wilma $workchan/betty -v -nl \
  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 343"
$bin/aegis -cp $workchan/aegis.conf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify file 347"
sed 's/derived3/betty/' $workchan/aegis.conf >$workchan/fubar
mv $workchan/fubar $workchan/aegis.conf
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 352"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 356"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 360"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 364"
$bin/aegis -ib 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 368"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 372"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 376"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
$bin/aegis -list pf >>/tmp/aelog
echo "## BL contents:" >>/tmp/aelog
ls -lR `aesub -bl '$BL'` >>/tmp/aelog
echo "## third change:" >>/tmp/aelog
fi

##########################################
# Part 4. produce a derived file with the previously removed name
activity="new change 389"
cat > caf << 'fubar'
brief_description = "three";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f caf -p test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 399"
$bin/aegis -db 3 -dir $workchan -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
echo "## DD contents:" >>/tmp/aelog
ls -lR `aesub '$DD'` >>/tmp/aelog
fi

activity="copy file 408"
$bin/aegis -cp $workchan/aegis.conf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify file 412"
sed 's/derived1/barney/' $workchan/aegis.conf >$workchan/fubar
mv $workchan/fubar $workchan/aegis.conf
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 417"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 421"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 425"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 429"
$bin/aegis -ib 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 433"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 437"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 441"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
$bin/aegis -list pf >>/tmp/aelog
echo "## BL contents:" >>/tmp/aelog
ls -lR `aesub -bl '$BL'` >>/tmp/aelog
echo "## fourth change:" >>/tmp/aelog
fi

# Move to a branch to check the logic more rigorously

activity="new branch 454"
$bin/aegis -nbr 1 -p test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

##########################################
# Part 5. remove a source file
activity="new change 463"
cat > caf << 'fubar'
brief_description = "two";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f caf -p test.1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 473"
$bin/aegis -db 2 -dir $workchan -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
echo "## DD contents:" >>/tmp/aelog
ls -lR `aesub '$DD'` >>/tmp/aelog
fi

activity="check symlink 482"
if test ! $test_flag $workchan/barney ; then fail; fi

activity="remove file 485"
$bin/aegis -rm $workchan/pebbles $workchan/bammbamm -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 489"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 493"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 497"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 501"
$bin/aegis -ib 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 505"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 509"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 513"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
$bin/aegis -list pf >>/tmp/aelog
echo "## BL contents:" >>/tmp/aelog
ls -lR `aesub -bl '$BL'` >>/tmp/aelog
echo "## third change:" >>/tmp/aelog
fi

##########################################
# Part 6. produce a derived file with the previously removed name
activity="new change 526"
cat > caf << 'fubar'
brief_description = "three";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f caf -p test.1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 536"
$bin/aegis -db 3 -dir $workchan -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
echo "## DD contents:" >>/tmp/aelog
ls -lR `aesub '$DD'` >>/tmp/aelog
fi

activity="copy file 545"
$bin/aegis -cp $workchan/aegis.conf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify file 549"
sed 's/derived2/pebbles/' $workchan/aegis.conf >$workchan/fubar
mv $workchan/fubar $workchan/aegis.conf
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 554"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 558"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 562"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 566"
$bin/aegis -ib 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 570"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 574"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 578"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
$bin/aegis -list pf >>/tmp/aelog
echo "## BL contents:" >>/tmp/aelog
ls -lR `aesub -bl '$BL'` >>/tmp/aelog
echo "## fourth change:" >>/tmp/aelog
fi

##########################################
# Part 7. check for availability in subsequent development
activity="new change 591"
cat > caf << 'fubar'
brief_description = "four";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 4 -f caf -p test.1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 601"
$bin/aegis -db 4 -dir $workchan -v -nl \
  -trace libaegis/change/build/symlinks.cc > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
echo "## DD contents:" >>/tmp/aelog
ls -lR `aesub '$DD'` >>/tmp/aelog
fi

activity="check symlinks 611"
if test ! $test_flag $workchan/barney ; then fail; fi
if test $test_flag $workchan/wilma ; then fail; fi
if test ! $test_flag $workchan/betty ; then fail; fi
if test ! $test_flag $workchan/pebbles ; then fail; fi
if test $test_flag $workchan/bammbamm ; then fail; fi

activity="copy file 622" # just to be a nontrivial change
$bin/aegis -cp $workchan/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="mofify file 622"
echo two >> $workchan/fred
if test $? -ne 0 ; then no_result; fi

activity="build 626"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 630"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 634"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 638"
$bin/aegis -ib 4 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 642"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
echo "## IntD contents:" >>/tmp/aelog
ls -lR `aesub '$IntD'` >>/tmp/aelog
fi

activity="find integration dir 651"
INTD=`$bin/aesub '$IntD'`
if test $? -ne 0 ; then cat log; no_result; fi

activity="check links 655"
if test ! -f $INTD/barney ; then fail; fi
if test -f $INTD/wilma ; then fail; fi
if test ! -f $INTD/betty ; then fail; fi
if test ! -f $INTD/pebbles ; then fail; fi
if test -f $INTD/bammbamm ; then fail; fi

activity="integrate begin undo 662"
$bin/aegis -ibu 4 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 666"
$bin/aegis -ib -mini 4 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 670"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

if test "$DEBUG" ; then
echo "## IntD (-mini) contents:" >>/tmp/aelog
ls -lR `aesub '$IntD'` >>/tmp/aelog
fi

activity="find integration dir 679"
INTD=`$bin/aesub '$IntD'`
if test $? -ne 0 ; then cat log; no_result; fi

activity="check links 683"
if test ! -f $INTD/barney ; then fail; fi
if test -f $INTD/wilma ; then fail; fi
if test ! -f $INTD/betty ; then fail; fi
if test ! -f $INTD/pebbles ; then fail; fi
if test -f $INTD/bammbamm ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
