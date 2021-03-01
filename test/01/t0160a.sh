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
	log_file_preference = never; \
	default_project_directory = \"$work\"; \
	default_development_directory = \"$work\";"
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

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

no_result()
{
	set +x
	echo 'NO RESULT for test of the aegis.confdir functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo 'FAILED test of the aegis.confdir functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	echo PASSED 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
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

AEGIS_PROJECT=test
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

#
# test the aegis.confdir functionality
#
activity="new project 121"
$bin/aegis -npr test -version '' -v -dir $work/proj.dir > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 125"
cat > paf << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 136"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 144"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 154"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 158"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/config.d/build \
	$work/test.C010/config.d/history $work/test.C010/config.d/misc \
	$work/test.C010/fred $work/test.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/test.C010/aegis.conf << 'fubar'
configuration_directory = "config.d";
fubar
if test $? -ne 0 ; then no_result; fi

cat > $work/test.C010/config.d/build << 'fubar'
build_command = "exit 0";
link_integration_directory = true;
fubar
if test $? -ne 0 ; then no_result; fi

cat > $work/test.C010/config.d/history << 'fubar'
history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;
fubar
if test $? -ne 0 ; then no_result; fi

cat > $work/test.C010/config.d/misc << 'fubar'
diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="build 200"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 204"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 208"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 212"
$bin/aegis -rpass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 216"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 220"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 224"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 228"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# now create a branch
#
activity="new branch 235"
$bin/aegis -nbr -p test 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

#
# The first change on the branch
# override the build rule
#
activity="new change 246"
cat > caf << 'fubar'
brief_description = "the second change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 255"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 259"
$bin/aegis -cp $work/test.1.C010/config.d/build -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/test.1.C010/config.d/build << 'fubar'
build_command = "rm -rf wilma; date > wilma; exit 0";
fubar
if test $? -ne 0 ; then no_result; fi

activity="build 268"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

test -f $work/test.1.C010/wilma || fail

activity="diff 274"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 278"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 282"
$bin/aegis -rpass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 286"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 290"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

test -f $work/test.1.C010/wilma || fail

activity="integrate diff 296"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 300"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
