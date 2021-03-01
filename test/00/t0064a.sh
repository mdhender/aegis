#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1997, 1998 Peter Miller;
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
	echo "FAILED test of the aede functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of the aede functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="working directory 77"
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

AEGIS_PATH=$work/lib
export AEGIS_PATH

#
# test the aede functionality
#
activity="new preject 100"
$bin/aegis -npr test -vers '' -dir $work/proj -lib $work/lib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="project attributes 107"
cat > paf << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="new developer 118"
$bin/aegis -nd $USER > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi
activity="new reviewer 121"
$bin/aegis -nrv $USER > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi
activity="new integrator 124"
$bin/aegis -ni $USER > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# The first change creates a file ---------------------------------------------

activity="new change 130"
cat > caf << 'fubar'
brief_description = "change the first";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc -f caf -p test > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop begin 143"
$bin/aegis -db 10 -dir $work/chan > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="new file 147"
$bin/aegis -nf $work/chan/config $work/chan/fred > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > $work/chan/config << 'fubar'
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

activity="build 167"
$bin/aegis -b > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="diff 171"
$bin/aegis -diff > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop end 175"
$bin/aegis -de > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="review pass 179"
$bin/aegis -rpass 10 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate begin 183"
$bin/aegis -ib 10 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="build 187"
$bin/aegis -b > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 191"
$bin/aegis -ipass > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# The second change deletes the file -------------------------------------------

activity="new change 197"
cat > caf << 'fubar'
brief_description = "change the second";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc -f caf -p test > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop begin 210"
$bin/aegis -db 11 -dir $work/chan > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="remove file 214"
$bin/aegis -rm $work/chan/fred > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="build 218"
$bin/aegis -b > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="diff 222"
$bin/aegis -diff > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop end 226"
$bin/aegis -de > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="review pass 230"
$bin/aegis -rpass 11 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate begin 234"
$bin/aegis -ib 11 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="build 238"
$bin/aegis -b > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 242"
$bin/aegis -ipass > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# Create a new branch ---------------------------------------------------------

activity="new branch 248"
$bin/aegis -nbr -p test > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

# Change the third creates the file again -------------------------------------

activity="new change 257"
cat > caf << 'fubar'
brief_description = "change the third";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc -f caf -p test.1 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop begin 270"
$bin/aegis -db 10 -dir $work/chan > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="new file 274"
$bin/aegis -nf $work/chan/fred > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="build 278"
$bin/aegis -b > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="diff 282"
$bin/aegis -diff > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop end 286"
$bin/aegis -de > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# Verify that develop end shallowed the file properly -------------------------

activity="verify trunk file state 292"

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
	if test $? -ne 0; then fail; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

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
	},
	{
		file_name = "fred";
		action = remove;
		edit_number = "1.1";
		edit_number_origin = "1.1";
		usage = source;
		deleted_by = 11;
	},
];
fubar
if test $? -ne 0 ; then cat LOG; no_result; fi

check_it ok $work/proj/info/trunk.fs

activity="verify branch file state 339"

cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		action = remove;
		edit_number = "1.1";
		edit_number_origin = "1.1";
		usage = source;
		locked_by = 10;
		about_to_be_created_by = 10;
		about_to_be_copied_by = 10;
		deleted_by = 10;
	},
];
fubar
if test $? -ne 0 ; then cat LOG; no_result; fi

check_it ok $work/proj/info/change/0/001.fs

# Finish the integration ------------------------------------------------------

activity="review pass 363"
$bin/aegis -rpass 10 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate begin 367"
$bin/aegis -ib 10 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="diff 371"
$bin/aegis -diff > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="build 375"
$bin/aegis -b > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 379"
$bin/aegis -ipass > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# Verify branch file state ----------------------------------------------------

activity="verify branch file state 385"

cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		action = create;
		edit_number = "1.2";
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
];
fubar
if test $? -ne 0 ; then cat LOG; no_result; fi

check_it ok $work/proj/info/change/0/001.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
