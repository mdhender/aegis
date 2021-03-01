#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004 Walter Franzini;
#	All rights reserved.
#       Copyright (C) 2007, 2008 Peter Miller
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
activity="new project 115"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj \
    -lib $work/lib -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
#
# change project attributes
#
activity="project attributes 122"
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
activity="new change 137"
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
activity="staff 153"
$bin/aegis -nd $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 164"
$bin/aegis -db 10 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "rm -f non-source-file && date > non-source-file && exit 0";
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
create_symlinks_before_build = true;
remove_symlinks_after_integration_build = false;
end
if test $? -ne 0 ; then no_result; fi

activity="New file 187"
$bin/aegis -nf $workchan/aegis.conf > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="New file build 191"
$bin/aegis -nf -build $workchan/non-source-file > log 2>& 1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Build 195"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Diff 199"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Develop end 203"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Review Pass 207"
$bin/aegis -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Begin 211"
$bin/aegis -ib  10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Build 215"
$bin/aegis -b 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Pass 219"
$bin/aegis -ipass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Open a new branch
#
activity="new branch 226"
$bin/aegis -nbr  1 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=blurb.1

activity="branch change 232"
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

activity="develop begin 245"
$bin/aegis -db -c 10 -p $AEGIS_PROJECT -dir $workchan -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 249"
$bin/aegis -cp -c 10 $workchan/aegis.conf -p $AEGIS_PROJECT -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
echo '/* */' >> $workchan/config
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop build 255"
$bin/aegis -build -p $AEGIS_PROJECT -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop diff 259"
$bin/aegis -diff -p $AEGIS_PROJECT -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 263"
$bin/aegis -p $AEGIS_PROJECT -de 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 267"
$bin/aegis -p $AEGIS_PROJECT -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 271"
$bin/aegis -p $AEGIS_PROJECT -ib 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 275"
$bin/aegis -p $AEGIS_PROJECT -build 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 279"
$bin/aegis -p $AEGIS_PROJECT -diff 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 283"
$bin/aegis -p $AEGIS_PROJECT -ipass 10  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# The second change
activity="new change 288"
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

activity="develop begin 302"
$bin/aegis -db 11 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $tmp <<EOF
This is a dummy file
EOF
$bin/aegis -nf 11 $workchan/dummy_file > log 2>&1
cp $tmp $workchan/dummy_file

activity="develop build 312"
$bin/aegis -b 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop diff 316"
$bin/aegis -diff 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 320"
$bin/aegis -de 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 324"
$bin/aegis -rpass 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 328"
$bin/aegis -ib 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 332"
$bin/aegis -b 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 336"
$bin/aegis -diff 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 340"
$bin/aegis -ipass 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 344"
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

activity="develop begin 358"
$bin/aegis -db 11 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 362"
$bin/aegis -cp 11 $workchan/dummy_file > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo 'DUMMY LINE' >> $workchan/dummy_file

activity="develop build 368"
$bin/aegis -build 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop diff 372"
$bin/aegis -diff 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 376"
$bin/aegis -de 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 380"
$bin/aegis -rpass 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 384"
$bin/aegis -ib 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 388"
$bin/aegis -build 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 392"
$bin/aegis -diff 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 396"
$bin/aegis -ipass 11 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 400"
AEGIS_PROJECT=blurb
$bin/aegis -de 1 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="review pass 405"
$bin/aegis -rpass 1 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="integrate begin 409"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="integrate build 413"
$bin/aegis -build 1 -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="integrate diff 417"
$bin/aegis -diff 1 -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="integrate pass 421"
$bin/aegis -ipass 1 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
