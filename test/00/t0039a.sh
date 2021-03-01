#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996-1998, 2005-2008 Peter Miller
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
workchan=$work/chan
worklib=$work/lib
workproj=$work/proj
tmp=$work/blah_blah_blah
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
AEGIS_PATH=$worklib
export AEGIS_PATH
AEGIS_PROJECT=testcorr
export AEGIS_PROJECT

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
	echo 'FAILED test of the test correlation functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT for test of the test correlation functionality' 1>&2
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

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# make a new project
#
activity="new project 107"
$bin/aegis -newpro $AEGIS_PROJECT -version "" -dir $workproj -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 114"
cat > $tmp << 'TheEnd'
description = "test the test correlation functionality";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -proatt -f $tmp -proj $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 129"
cat > $tmp << 'TheEnd'
brief_description = "one";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -new_change 1 -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add staff
#
activity="new developer 145"
$bin/aegis -newdev $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="new reviewer 148"
$bin/aegis -newrev $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="new integrator 151"
$bin/aegis -newint $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of the change
#
activity="develop begin 158"
$bin/aegis -devbeg 1 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the new files to the change
#
activity="new file 165"
$bin/aegis -new_file $workchan/aegis.conf $workchan/a -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/aegis.conf << 'TheEnd'
build_command = "exit 0";
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";

diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
TheEnd
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
# This is NOT recommended practice when writing tests :-)
#
activity="new test 194"
$bin/aegis -nt -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

sed -e 's/^X//' > $workchan/test/00/t0001a.sh << 'TheEnd'
Xcp /dev/null ran.test.1
Xexit 0
TheEnd
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 207"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 214"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 221"
$bin/aegis -test -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 228"
$bin/aegis -dev_end -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 235"
$bin/aegis -review_pass -chan 1 -proj $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 242"
$bin/aegis -intbeg 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build and test
#
activity="build 249"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="test 252"
$bin/aegis -test -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 259"
$bin/aegis -intpass -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# create the second change
#
activity="new change 266"
cat > $tmp << 'fubar'
brief_description = "two";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_change 2 -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of the change
#
activity="develop begin 281"
$bin/aegis -devbeg 2 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the new files to the change
#
activity="new file 288"
$bin/aegis -nf $workchan/b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new test
#
activity="new test 295"
$bin/aegis -nt -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

sed -e 's/^X//' > $workchan/test/00/t0002a.sh << 'TheEnd'
Xcp /dev/null ran.test.2
Xexit 0
TheEnd
if test $? -ne 0 ; then no_result; fi


#
# reset the project attributes,
# the aent overrode the test exemptions
#
activity="change attributes 310"
$bin/aegis -ca 2 -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# build the change
#
activity="build 317"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 324"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 331"
$bin/aegis -test -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 338"
$bin/aegis -dev_end -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 345"
$bin/aegis -review_pass -chan 2 -proj $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 352"
$bin/aegis -intbeg 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build and test
#
activity="build 359"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="test 362"
$bin/aegis -test -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 369"
$bin/aegis -intpass -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# create the third change
#
activity="new change 376"
cat > $tmp << 'fubar'
brief_description = "three";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_change 3 -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of the change
#
activity="develop begin 388"
$bin/aegis -devbeg 3 -dir $workchan.3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the new files to the change
#
activity="copy file 395"
$bin/aegis -copy_file $workchan.3/a -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="test 402"
$bin/aegis -test -suggest 1 -suggest_noise 0 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# only test 1 should have been run
#
activity="verify tests run 409"
if test ! -r $workchan.3/ran.test.1; then fail; fi
if test -r $workchan.3/ran.test.2; then fail; fi

#
# Only definite negatives are possible.  The functionality exercised by
# this test appears to work, no other guarantees are made.
#
pass
