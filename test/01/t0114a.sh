#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2002, 2004-2008 Peter Miller
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
worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT
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
	echo "FAILED test of the aeb -minimum  functionality " 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
  echo "NO RESULT when testing the aeb -minimum  functionality ($activity)" 1>&2
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
# test the aeb -minimum functionality
#
#
# make a new project
#
activity="new project 111"
$bin/aegis -npr foo -vers "" -dir $workproj  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 118"
cat > $tmp << 'end'
description = "A bogus project created to test the aeb -minimum functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 132"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true ;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 10 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 148"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 10 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "echo Hello > non-source-file && exit 0";
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

activity="New file 181"
$bin/aegis -nf $workchan/aegis.conf > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Build 185"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Diff 189"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


activity="Develop end 194"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Review Pass 198"
$bin/aegis -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Begin 202"
$bin/aegis -ib  10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Build 206"
$bin/aegis -b 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Pass 210"
$bin/aegis -ipass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


activity="new branch 215"
$bin/aegis -nbr  1 -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


# The second change
activity="new change 221"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true ;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi


activity="new change 232"
$bin/aegis -nc 11 -f $tmp -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Develop Begin 236"
$bin/aegis -db 11 -dir $workchan -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Change Dir 240"
devdir=`$bin/aegis -cd 11  -p foo.1`
if test $? -ne 0 ; then no_result; fi
cd $devdir
if test $? -ne 0 ; then no_result; fi

activity="Copy File 246"
$bin/aegis -cp aegis.conf  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# Change the build command to not create any non source files
activity="Mutate aegis.conf 251"
grep -v 'build_command' aegis.conf > tmpConfig
if test $? -ne 0 ; then cat log; no_result; fi
echo 'build_command = "exit 0";' >> tmpConfig
if test $? -ne 0 ; then cat log; no_result; fi
mv tmpConfig aegis.conf
if test $? -ne 0 ; then cat log; no_result; fi

activity="Build 259"
$bin/aegis -b 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Diff 263"
$bin/aegis -diff 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Develop End 267"
$bin/aegis -de 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Review Pass 271"
$bin/aegis -rpass 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Begin 275"
$bin/aegis -ib -minimum 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Aesub  279"
intdir=`$bin/aesub '$intdir' -c 11 -p foo.1`
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Build 283"
$bin/aegis -b -minimum 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


# Non source files should not be present after a -minimum
test ! -h $intdir/non-source-file
if test $? -ne 0 ; then  fail ; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
