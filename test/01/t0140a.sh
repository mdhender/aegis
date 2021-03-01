#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2003 Peter Miller;
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
# MANIFEST: Test the test exemption (aecpu) functionality
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
AEGIS_PATH=$work/lib
export AEGIS_PATH

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
# If the C compiler is called something other than ``cc'', as discovered
# by the configure script, create a shell script called ``cc'' which
# invokes the correct C compiler.  Make sure the current directory is in
# the path, so that it will be invoked.
#
if test "$CC" != "" -a "$CC" != "cc"
then
	cat >> $work/cc << fubar
#!/bin/sh
exec $CC \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx $work/cc
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# test the test exemption functionality
#
projdir=$work/projdir
export projdir

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="new project 125"
$bin/aegis -npr $AEGIS_PROJECT -version - -dir $projdir > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 130"
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

activity="new change 167"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -p $AEGIS_PROJECT -f caf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

changedir=$projdir/changedir
export changedir

activity="develop begin 176"
$bin/aegis -db 10 -v  -dir $changedir > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 180"
$bin/aegis -nf $changedir/config $changedir/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $changedir/config << 'fubar'
build_command = "exit 0";
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
fubar
if test $? -ne 0 ; then no_result; fi
echo hello > $changedir/fred
if test $? -ne 0 ; then no_result; fi

activity="new test 190"
$bin/aegis -nt -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $changedir/test/00/t0001a.sh << 'fubar'
here=`pwd`;
if [ $here = $projdir/baseline ] ; then
    exit 1 ;
else
    exit 0 ;
fi
fubar
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 201"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 205"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 205"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 209"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 213"
$bin/aegis -rpass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 217"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 221"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 225"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test 241"
$bin/aegis -test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 245"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Next change
#
activity="new change 253"
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

activity="develop begin 262"
$bin/aegis -db 11 -v  -dir $changedir > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 265"
$bin/aegis -nf -baserel foo -v  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
echo Hi There > $changedir/foo
if test $? -ne 0 ; then no_result; fi

activity="copy file 267"
$bin/aegis -cp -baserel test/00/t0001a.sh  -v  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
echo \# comment  >> $changedir/test/00/t0001a.sh
if test $? -ne 0 ; then no_result; fi

activity="diff 275"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 287"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop end should fail (test -reg required)
activity="develop end 293"
$bin/aegis -de -v > log 2>&1
if test $? -eq 0 ; then cat log; fail; fi

grep 'test required' log > /dev/null
if test $? -ne 0 ; then cat log; fail; fi

activity="copy undo 302"
$bin/aegis -cpu $changedir/test/00/t0001a.sh -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 304"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# develop end should be OK now
activity="develop end 308"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

pass