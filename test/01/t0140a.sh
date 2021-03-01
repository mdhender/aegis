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
	echo 'FAILED test of the test_exemption (aecpu) functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the test_exemption functionality' 1>&2
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

activity="new project 111"
$bin/aegis -npr $AEGIS_PROJECT -version - -dir $projdir > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 115"
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

activity="new change 135"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -p $AEGIS_PROJECT -f caf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

changedir=$projdir/changedir
export changedir

activity="develop begin 147"
$bin/aegis -db 10 -v  -dir $changedir > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 151"
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

activity="new test 172"
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

activity="build 185"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 189"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 193"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 197"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 201"
$bin/aegis -rpass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 205"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 209"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 213"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 217"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 221"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Next change
#
activity="new change 228"
cat > caf << 'fubar'
brief_description = "two";
cause = internal_bug;
test_exempt = true;
regression_test_exempt = true;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -p $AEGIS_PROJECT -f caf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 240"
$bin/aegis -db 11 -v  -dir $changedir > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 244"
$bin/aegis -nf -baserel foo -v  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
echo Hi There > $changedir/foo
if test $? -ne 0 ; then no_result; fi

activity="copy file 250"
$bin/aegis -cp -baserel test/00/t0001a.sh  -v  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
echo \# comment  >> $changedir/test/00/t0001a.sh
if test $? -ne 0 ; then no_result; fi

activity="diff 256"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 260"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop end should fail (test -reg required)
activity="develop end 265"
$bin/aegis -de -v > log 2>&1
if test $? -eq 0 ; then cat log; fail; fi

grep 'test required' log > /dev/null
if test $? -ne 0 ; then cat log; fail; fi

activity="copy undo 272"
$bin/aegis -cpu $changedir/test/00/t0001a.sh -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 276"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop end should be OK now
activity="develop end 281"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

pass
