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
	echo "FAILED test of the aefind functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aefind functionality ($activity)" 1>&2
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
activity="new project 99"
$bin/aegis -newpro example -dir $work/proj -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=example.1.0
export AEGIS_PROJECT

#
# change project attributes
#
activity="project attributes 109"
cat > $work/tmp << 'TheEnd'
description = "test of aefind";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -proatt -f $work/tmp -proj example -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -proatt -f $work/tmp -proj example.1 -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -proatt -f $work/tmp -proj example.1.0 -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 128"
cat > $work/tmp << 'TheEnd'
brief_description = "first";
description = "The first change";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc -f $work/tmp -project example.1.0 -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new developer
#
activity="new developer 144"
$bin/aegis -newdev $USER -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of the change
#
activity="develop begin 151"
$bin/aegis -devbeg 10 -dir $work/chan -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the new files to the change
#
activity="new file 158"
$bin/aegis -nf $work/chan/aegis.conf -c 10 -nl -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/chan/aegis.conf << 'TheEnd'
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
# build the change
#
activity="build 185"
$bin/aegis -build -nl -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 192"
$bin/aegis -diff -v -nl -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 199"
$bin/aegis -dev_end -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new reviewer
#
activity="new reviewer 206"
$bin/aegis -newrev $USER -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 213"
$bin/aegis -review_pass -chan 10 -proj example.1.0 -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add an integrator
#
activity="new integrator 220"
$bin/aegis -newint $USER -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 227"
$bin/aegis -intbeg 10 -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 234"
$bin/aegis -build -nl -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate diff
#
activity="diff 241"
$bin/aegis -diff -nl -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 248"
$bin/aegis -intpass -nl -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create the second and subsequent changes
#
activity="new change 255"
cat > $work/tmp << 'fubar'
brief_description = "two";
description = "The second change.";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f $work/tmp -project example.1.0 -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of the change
#
activity="develop begin 270"
$bin/aegis -devbeg 11 -dir $work/chan -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create an new file
#
$bin/aegis -nf $work/chan/fred -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# run a simple find
#
$bin/aefind $work/chan -print -lib $work/lib -noresolve > test.out 2> log
if test $? -ne 0 ; then cat log; fail; fi

cat > test.ok << 'fubar'
.
aegis.conf
aegis.conf,D
fred
fubar
if test $? -ne 0 ; then no_result; fi

sort test.out > test.out2
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out2
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
