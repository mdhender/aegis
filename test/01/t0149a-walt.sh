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
# MANIFEST: Test the aedist functionality
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
	echo "FAILED test of the aedist functionality (${activity})" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aedist functionality (${activity})" 1>&2
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
# test 'aedist -r' removing non existent files
#
#
# make a new project
#
AEGIS_PROJECT=foo
AEGIS_PATH=$work/lib
workchan=$work/chan
workproj=$work/proj
tmp=$work/tmp
export AEGIS_PROJECT AEGIS_PATH
activity="new project 130"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj \
    -lib $work/lib -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $tmp << 'end'
description = "A bogus project created to test the aedis functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -p $AEGIS_PROJECT -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


#
# add the staff
#
activity="staff 150"
$bin/aegis -nd $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 161"
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
# begin development of a change
#
$bin/aegis -db 10 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/config << 'end'
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
create_symlinks_before_build = true;
remove_symlinks_after_integration_build = false;
end
if test $? -ne 0 ; then no_result; fi

activity="New file 200"
$bin/aegis -nf $workchan/config > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="New file 204"
$bin/aegis -nf $workchan/source-file0 > log 2>& 1
if test $? -ne 0 ; then cat log; no_result; fi

activity="New file 208"
$bin/aegis -nf $workchan/source-file1 > log 2>& 1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Build 212"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Diff 216"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Develop end 220"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Review Pass 224"
$bin/aegis -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Begin 228"
$bin/aegis -ib  10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Build 232"
$bin/aegis -b 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Pass 236"
$bin/aegis -ipass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# the second change
#

activity="New change 244"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true ;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 11 -f $tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -db 11 -p $AEGIS_PROJECT -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="move file 260"
$bin/aegis -mv -c 11 -p $AEGIS_PROJECT $workchan/source-file0 \
    $workchan/source-file2 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="second change build 265"
$bin/aegis -build -c 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="second change diff 269"
$bin/aegis -diff -c 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="second change develop end 273"
$bin/aegis -de -c 11 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 277"
$bin/aegis -rpass -c 11 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 281"
$bin/aegis -ib -c 11 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -build -c 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -diff -c 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -ipass -c 11 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create the patch
#
activity="generate the patch 297"
$bin/aedist -s -p $AEGIS_PROJECT -bl -es -o $work/$AEGIS_PROJECT.ae > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# cleanup
#
activity="cleanup 304"
rm -rf $workchan
rm -rf $workproj
rm -rf $work/lib

#
# create the destination project
#

activity="new project 313"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj \
    -lib $work/lib -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $tmp << 'end'
description = "A bogus project created to test the aedis functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi

$bin/aegis -pa -p $AEGIS_PROJECT -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


#
# add the staff
#
activity="staff 334"
$bin/aegis -nd $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="receive the change 342"
$bin/aedist -r -p $AEGIS_PROJECT -f $work/$AEGIS_PROJECT.ae \
    -dir $workchan -verb > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
