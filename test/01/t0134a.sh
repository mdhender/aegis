#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2003-2008 Peter Miller
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
AEGIS_PATH=$work/lib
export AEGIS_PATH


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
	echo 'FAILED test of the test_exemption functionality' \
		"($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the test_exemption functionality' \
		"($activity)" 1>&2
	echo $activity
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
# test the test exemption functionality
#

projdir=$work/projdir
export projdir

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="new project 115"
$bin/aegis -npr $AEGIS_PROJECT -version - -dir $projdir > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 119"
cat > paf << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = false;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nd $USER
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nrv $USER
if test $? -ne 0 ; then no_result; fi

$bin/aegis -ni $USER
if test $? -ne 0 ; then no_result; fi

activity="new change 139"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -p $AEGIS_PROJECT -f caf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

changedir=$projdir/changedir
export changedir

activity="develop begin 151"
$bin/aegis -db 10 -v  -dir $changedir > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 155"
$bin/aegis -nf $changedir/aegis.conf $changedir/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $changedir/aegis.conf << 'fubar'
build_command = "exit 0";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
fubar
if test $? -ne 0 ; then no_result; fi
echo hello > $changedir/fred
if test $? -ne 0 ; then no_result; fi

activity="new test 176"
$bin/aegis -nt -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $changedir/test/00/t0001a.sh << 'fubar'
if pwd | grep "$projdir/baseline" > /dev/null
then
    exit 1 ;
else
    exit 0 ;
fi
fubar
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 189"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 193"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 197"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 201"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 205"
$bin/aegis -rpass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 209"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 213"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 217"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 221"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 225"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Next change
#
activity="new change 232"
cat > caf << 'fubar'
brief_description = "two";
cause = internal_bug;
regression_test_exempt = false;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -p $AEGIS_PROJECT -f caf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 242"
$bin/aegis -db 11 -v  -dir $changedir > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 246"
$bin/aegis -cp -baserel fred  -v  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $changedir/fred << 'fubar'
Goodbye
fubar
if test $? -ne 0 ; then no_result; fi

activity="new test 254"
$bin/aegis -nt -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $changedir/test/00/t0002a.sh << 'fubar'
if pwd | grep "$projdir/baseline" > /dev/null
then
    exit 1 ;
else
    exit 0 ;
fi
fubar
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 267"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 271"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 275"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="baseline test 279"
$bin/aegis -test -bl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop end should fail (test -reg required)
activity="develop end 284"
$bin/aegis -de -v > log 2>&1
if test $? -eq 0 ; then cat log; no_result; fi

grep 'test -reg required' log > /dev/null
if test $? -ne 0 ; then cat log; no_result; fi

# But now if we aecp the only other test, the regression exemption is granted
activity="copy file 292"
$bin/aegis -cp -baserel test/00/t0001a.sh  -v  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
echo \# comment >> $changedir/test/00/t0001a.sh
if test $? -ne 0 ; then no_result; fi

activity="diff 298"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 302"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 306"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="baseline test 310"
$bin/aegis -test -bl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop end should now be permitted
# no aet -reg is required
activity="develop end 316"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; fail ; fi

activity="develop end undo 320"
$bin/aegis -deu -v -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result ; fi

# aecpu the test
activity="copy undo 325"
$bin/aegis -cpu -baserel test/00/t0001a.sh -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result ; fi

# make sure that regression exemption is now false again
activity="build 330"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 334"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="baseline test 338"
$bin/aegis -test -bl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop end should fail (test -reg required)
activity="develop end 343"
$bin/aegis -de -v > log 2>&1
if test $? -eq 0 ; then cat log; fail; fi

pass
