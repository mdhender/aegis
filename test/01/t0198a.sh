#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004-2008 Peter Miller
#	Copyright (C) 2007 Walter Franzini
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
#	along with this program.  If not, see
#	<http://www.gnu.org/licenses/>,
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

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

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
	echo "FAILED test of the dev dir style functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the dev dir style functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 98"
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

#
# test the dev dir style functionality
#
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="new project 135"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT


activity="project attributes 144"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 157"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 165"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 175"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 179"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/fred \
	$work/test.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "date > derived1 && date > derived2";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
development_directory_style =
{
    source_file_link = true;
    source_file_symlink = true;
    source_file_copy = true;
};
integration_directory_style =
{
    source_file_link = true;
    source_file_symlink = true;
    source_file_copy = true;
};
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="register derived2 218"
$bin/aegis -nf -build $work/test.C010/derived2 -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="build 222"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 226"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 230"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 234"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 238"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 242"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 246"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch 250"
$bin/aegis -nbr -p $AEGIS_PROJECT 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=${AEGIS_PROJECT}.1
export AEGIS_PROJECT

#
# The first change on the branch edits barney,
# and leaves fred alone.
#
activity="new change 261"
cat > caf << 'fubar'
brief_description = "the second change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 270"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# make sure there are hard links from dev dir to the baseline.
#
test -r $work/test.1.C010/derived1 && fail
#
# derived2 should NOT be there, because it's derived even as a
# registered file.
#
test -r $work/test.1.C010/derived2 && fail
test -r $work/test.1.C010/barney || fail
test -r $work/test.1.C010/aegis.conf || fail
test -r $work/test.1.C010/fred || fail
test -w $work/test.1.C010/barney && fail
test -w $work/test.1.C010/aegis.conf && fail
test -w $work/test.1.C010/fred && fail

activity="copy file 290"
$bin/aegis -cp $work/test.1.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

test -w $work/test.1.C010/barney || fail

echo second > $work/test.1.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="build 299"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

test -w $work/test.1.C010/derived1 || fail
test -w $work/test.1.C010/derived2 || fail

activity="diff 306"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 310"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 314"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="verify integration directory contents 318"
test -r $work/proj.dir/branch.1/delta*/barney || fail
test -w $work/proj.dir/branch.1/delta*/barney && fail
test -r $work/proj.dir/branch.1/delta*/aegis.conf || fail
test -w $work/proj.dir/branch.1/delta*/aegis.conf && fail
test -r $work/proj.dir/branch.1/delta*/derived1 && fail
test -r $work/proj.dir/branch.1/delta*/derived2 && fail
test -r $work/proj.dir/branch.1/delta*/fred || fail
test -w $work/proj.dir/branch.1/delta*/fred && fail
# make sure basline and delta are different
cmp proj.dir/branch.1/delta*/barney proj.dir/branch.1/baseline/barney \
	> /dev/null 2>&1 && fail

activity="integrate build 331"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

test -r $work/proj.dir/branch.1/delta*/derived1 || fail
test -r $work/proj.dir/branch.1/delta*/derived2 || fail
test -r $work/proj.dir/branch.1/delta*/barney || fail
test -w $work/proj.dir/branch.1/delta*/barney && fail

activity="integrate diff 340"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 344"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

test -r $work/proj.dir/branch.1/baseline/barney || fail
test -w $work/proj.dir/branch.1/baseline/barney && fail

activity="new change 351"
cat > caf << 'fubar'
brief_description = "the third change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 360"
$bin/aegis -db 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

test -r $work/test.1.C011/barney || fail
test -w $work/test.1.C011/barney && fail
test -r $work/test.1.C011/aegis.conf || fail
test -w $work/test.1.C011/aegis.conf && fail
test -r $work/test.1.C011/derived1 && fail
test -r $work/test.1.C011/derived2 && fail
test -r $work/test.1.C011/fred || fail
test -w $work/test.1.C011/fred && fail
cmp $work/test.1.C011/barney $work/proj.dir/branch.1/baseline/barney \
	> /dev/null 2>&1 || fail
cmp $work/test.1.C011/fred $work/proj.dir/baseline/fred \
	> /dev/null 2>&1 || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
