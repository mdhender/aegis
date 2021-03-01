#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996, 1997, 1998 Peter Miller;
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
# MANIFEST: Test the aerm vs aede branch functionality
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
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

AEGIS_PROJECT=test.1.0
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin=$here/${1-.}/bin

no_result()
{
	set +x
	echo "NO RESULT for test of the aerm vs aede branch functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of the aerm vs aede branch functionality ($activity)" 1>&2
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

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/$USER/USER/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

activity="working directory 91"
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
# test the aerm vs aede branch functionality
#
activity="new project 124"
$bin/aegis -npr test -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 128"
cat > paf << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -pa -f paf -v -p test.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# set the branch attributes to something useful
#
cat > caf << 'fubar'
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -ca -f caf -v -p test.1 -c 0 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 153"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nd $USER -v -p test.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v -p test.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v -p test.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 167"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test.1.0 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 176"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 180"
$bin/aegis -nf $work/test.1.0.C010/config $work/test.1.0.C010/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.1.0.C010/config << 'fubar'
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
echo hello > $work/test.1.0.C010/fred
if test $? -ne 0 ; then no_result; fi

activity="build 201"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 205"
$bin/aegis -diff -v > log 2>&1
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

activity="integrate pass 229"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
if test ! -f $work/test/branch.1/branch.0/baseline/config; then no_result; fi
if test ! -f $work/test/branch.1/branch.0/baseline/config,D; then no_result; fi
if test ! -f $work/test/branch.1/branch.0/baseline/fred; then no_result; fi
if test ! -f $work/test/branch.1/branch.0/baseline/fred,D; then no_result; fi

#
# now remove a file
#
activity="new change 240"
cat > caf << 'fubar'
brief_description = "two";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test.1.0 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 249"
$bin/aegis -db 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="remove file 253"
$bin/aegis -rm $work/test.1.0.C011/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check change file state 257"
cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		action = remove;
		edit_number_origin = "1.1";
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/001.branch/0/000.branch/0/011.fs

activity="build 273"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 277"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 281"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="review pass 285"
$bin/aegis -rpass 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 289"
$bin/aegis -ib 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
if test ! -f $work/test/branch.1/branch.0/delta*/config; then no_result; fi
if test ! -f $work/test/branch.1/branch.0/delta*/config,D; then no_result; fi
if test -f $work/test/branch.1/branch.0/delta*/fred; then fail; fi
if test -f $work/test/branch.1/branch.0/delta*/fred,D; then fail; fi

activity="integrate build 297"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 301"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
if test ! -f $work/test/branch.1/branch.0/delta*/fred,D; then fail; fi

activity="check change file state 306"
cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		action = remove;
		edit_number_origin = "1.1";
		usage = source;
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		idiff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/001.branch/0/000.branch/0/011.fs

activity="integrate pass 334"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
if test ! -f $work/test/branch.1/branch.0/baseline/config; then no_result; fi
if test ! -f $work/test/branch.1/branch.0/baseline/config,D; then no_result; fi
if test -f $work/test/branch.1/branch.0/baseline/fred; then fail; fi
if test ! -f $work/test/branch.1/branch.0/baseline/fred,D; then fail; fi

activity="check branch file state 342"
cat > ok << 'fubar'
src =
[
	{
		file_name = "config";
		action = create;
		edit_number = "1.1";
		edit_number_origin = "1.1";
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "fred";
		action = remove;
		edit_number = "1.1";
		edit_number_origin = "1.1";
		usage = source;
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		deleted_by = 11;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/001.branch/0/000.fs

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

activity="develop end 387"
$bin/aegis -de 0 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 391"
$bin/aegis -rpass 0 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 395"
$bin/aegis -ib 0 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 399"
$bin/aegis -b 0 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 403"
$bin/aegis -diff 0 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 407"
$bin/aegis -ipass 0 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check branch file state 411"
cat > ok << 'fubar'
src =
[
	{
		file_name = "config";
		action = create;
		edit_number = "1.2";
		edit_number_origin = "1.2";
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "fred";
		action = remove;
		edit_number = "1.1";
		edit_number_origin = "1.1";
		usage = source;
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		deleted_by = ZERO;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/001.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass