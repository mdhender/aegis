#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1997, 1998 Peter Miller;
#	All rights reserved.
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
# MANIFEST: Test the aefind functionality
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
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin=$here/${1-.}/bin

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
activity="new project 97"
$bin/aegis -newpro example -dir $work/proj -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=example.1.0
export AEGIS_PROJECT

#
# change project attributes
#
activity="project attributes 104"
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
activity="new change 119"
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
activity="new developer 133"
$bin/aegis -newdev $USER -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of the change
#
activity="develop begin 140"
$bin/aegis -devbeg 10 -dir $work/chan -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the new files to the change
#
activity="new file 147"
$bin/aegis -nf $work/chan/config -c 10 -nl -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/chan/config << 'TheEnd'
build_command = "exit 0";
link_integration_directory = true;

history_get_command =
	"co -u'$e' -p $h,v > $o";
history_create_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_put_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_query_command =
	"rlog -r $h,v | awk '/^head:/ {print $$2}'";

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";

diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
TheEnd
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 174"
$bin/aegis -build -nl -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 181"
$bin/aegis -diff -v -nl -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 188"
$bin/aegis -dev_end -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new reviewer
#
activity="new reviewer 195"
$bin/aegis -newrev $USER -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 202"
$bin/aegis -review_pass -chan 10 -proj example.1.0 -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add an integrator
#
activity="new integrator 209"
$bin/aegis -newint $USER -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 216"
$bin/aegis -intbeg 10 -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 223"
$bin/aegis -build -nl -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate diff
#
activity="diff 223"
$bin/aegis -diff -nl -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 233"
$bin/aegis -intpass -nl -v -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create the second and subsequent changes
#
activity="new change 240"
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
activity="develop begin 253"
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
$bin/aefind $work/chan -print -lib $work/lib > test.out 2> log
if test $? -ne 0 ; then cat log; fail; fi

cat > test.ok << 'fubar'
.
config
config,D
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
