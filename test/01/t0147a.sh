#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004 Walter Franzini;
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
# MANIFEST: Test the aede functionality
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
	echo 'FAILED test of the aede functionality' \
	    "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the aede functionality' \
	    "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 86"
mkdir $work $work/lib
workproj="$work/foo1.proj"
workchan="$work/foo.chan"
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi
tmp=$work/tmp

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
AEGIS_PATH=$work/lib
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the aede functionality
#
#
# make a new project
#
AEGIS_PROJECT=blurb
AEGIS_PATH=$work/lib
export AEGIS_PROJECT AEGIS_PATH
activity="new project 134"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj \
    -lib $work/lib -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
#
# change project attributes
#
activity="project attributes 141"
cat > $tmp << 'end'
description = "A bogus project created to test the de functionality on a branch.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -p $AEGIS_PROJECT -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 156"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true ;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 10 -f $tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 172"
$bin/aegis -nd $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 183"
$bin/aegis -db 10 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/config << 'end'
build_command = "rm -f non-source-file && date > non-source-file && exit 0";
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
create_symlinks_before_build = true;
remove_symlinks_after_integration_build = false;
end
if test $? -ne 0 ; then no_result; fi

activity="New file 206"
$bin/aegis -nf $workchan/config > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="New file build 210"
$bin/aegis -nf -build $workchan/non-source-file > log 2>& 1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Build 214"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Diff 218"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Develop end 222"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Review Pass 226"
$bin/aegis -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Begin 230"
$bin/aegis -ib  10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Build 234"
$bin/aegis -b 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Pass 238"
$bin/aegis -ipass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Open a new branch
#
activity="new branch 245"
$bin/aegis -nbr  1 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=blurb.1

activity="branch change 251"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true ;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 10 -p $AEGIS_PROJECT -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 264"
$bin/aegis -db -c 10 -p $AEGIS_PROJECT -dir $workchan -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 268"
$bin/aegis -cp -c 10 $workchan/config -p $AEGIS_PROJECT -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
echo '/* */' >> $workchan/config
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop build 274"
$bin/aegis -build -p $AEGIS_PROJECT -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop diff 278"
$bin/aegis -diff -p $AEGIS_PROJECT -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 282"
$bin/aegis -p $AEGIS_PROJECT -de 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 286"
$bin/aegis -p $AEGIS_PROJECT -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 290"
$bin/aegis -p $AEGIS_PROJECT -ib 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 294"
$bin/aegis -p $AEGIS_PROJECT -build 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 298"
$bin/aegis -p $AEGIS_PROJECT -diff 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 302"
$bin/aegis -p $AEGIS_PROJECT -ipass 10  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# The second change
activity="new change 307"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true ;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi
AEGIS_PROJECT=blurb

$bin/aegis -nc 11 -p $AEGIS_PROJECT -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 321"
$bin/aegis -db 11 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $tmp <<EOF
This is a dummy file
EOF
$bin/aegis -nf 11 $workchan/dummy_file > log 2>&1
cp $tmp $workchan/dummy_file

activity="develop build 331"
$bin/aegis -b 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop diff 335"
$bin/aegis -diff 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 339"
$bin/aegis -de 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 343"
$bin/aegis -rpass 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 347"
$bin/aegis -ib 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 351"
$bin/aegis -b 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 355"
$bin/aegis -diff 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 359"
$bin/aegis -ipass 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 363"
cat > $tmp << 'end'
brief_description = "The third change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true ;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi

AEGIS_PROJECT=blurb.1
$bin/aegis -nc 11 -p $AEGIS_PROJECT -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 377"
$bin/aegis -db 11 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 381"
$bin/aegis -cp 11 $workchan/dummy_file > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo 'DUMMY LINE' >> $workchan/dummy_file

activity="develop build 387"
$bin/aegis -build 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop diff 391"
$bin/aegis -diff 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 395"
$bin/aegis -de 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 399"
$bin/aegis -rpass 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 403"
$bin/aegis -ib 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 407"
$bin/aegis -build 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 411"
$bin/aegis -diff 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 415"
$bin/aegis -ipass 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 419"
AEGIS_PROJECT=blurb
$bin/aegis -de 1 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="review pass 424"
$bin/aegis -rpass 1 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="integrate begin 428"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="integrate build 432"
$bin/aegis -build 1 -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="integrate diff 436"
$bin/aegis -diff 1 -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="integrate pass 440"
$bin/aegis -ipass 1 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
