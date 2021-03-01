#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996-1998, 2000, 2001, 2004 Peter Miller;
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
# MANIFEST: Test the cross-branch merge functionality
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
	echo "FAILED test of the cross-branch merge functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of the cross-branch merge functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		-e 's/uuid = ".*"/uuid = "UUID"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

activity="working directory 86"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

AEGIS_PATH=$work/lib
export AEGIS_PATH
AEGIS_PROJECT=test
export AEGIS_PROJECT

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the cross-branch merge functionality
#
activity="new project 124"
$bin/aegis -npr $AEGIS_PROJECT -vers "" > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 128"
cat > paf << 'fubar'
developer_may_review = true;
reviewer_may_integrate = true;
developer_may_integrate = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -p $AEGIS_PROJECT -file paf > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 139"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# the first change established the files and their versions
#
activity="new change 150"
cat > caf << 'fubar'
brief_description = "ten";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 159"
$bin/aegis -db 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 163"
$bin/aegis -nf $work/test.C010/config $work/test.C010/fred > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/test.C010/config << 'fubar'
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

cat > $work/test.C010/fred << 'fubar'
one
two
three
four
five
fubar
if test $? -ne 0 ; then no_result; fi

activity="build 192"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 196"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 200"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 204"
$bin/aegis -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 208"
$bin/aegis -ib 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 212"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 216"
$bin/aegis -ipass > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# the branch is so that it can get out-of-date,
# necessitating the cross-branch merge
# between the trunk and the branch
#
activity="new branch 225"
$bin/aegis -nbr -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="change attributes 229"
cat > caf << 'fubar'
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -ca -f caf -p $AEGIS_PROJECT -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

activity="new change 243"
cat > caf << 'fubar'
brief_description = "one point ten";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 252"
$bin/aegis -db 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 256"
$bin/aegis -cp $work/test.1.C010/fred > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/test.1.C010/fred << 'fubar'
one
two
three
four
five
six
fubar
if test $? -ne 0 ; then no_result; fi

activity="build 270"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 274"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 278"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 282"
$bin/aegis -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 286"
$bin/aegis -ib 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 290"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 294"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 298"
$bin/aegis -ipass > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Now perform another change on the trunk.  This will make ``fred'' on
# the trunk different to ``fred'' on the branch.
#

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="new change 310"
cat > caf << 'fubar'
brief_description = "eleven";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 319"
$bin/aegis -db 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 323"
$bin/aegis -cp $work/test.C011/fred -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/test.C011/fred << 'fubar'
three
four
five
fubar
if test $? -ne 0 ; then no_result; fi

activity="build 334"
$bin/aegis -b -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 338"
$bin/aegis -diff -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 342"
$bin/aegis -de -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 346"
$bin/aegis -rpass 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 350"
$bin/aegis -ib 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 354"
$bin/aegis -b -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 358"
$bin/aegis -ipass -c 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# At this point the branch is out-of-date with respect to the trunk.
# Make sure that aede on the branch fails.
#
activity="check project file state 366"
cat > ok << 'fubar'
src =
[
	{
		file_name = "config";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = config;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "fred";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.3";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.3";
			encoding = none;
		};
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/trunk.fs

activity="check branch file state 401"
cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "1.2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
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
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/001.fs

activity="develop end [fail] 429"
$bin/aegis -de 1 -v > log 2>&1
if test $? -ne 1 ; then cat log; no_result; fi

#
# create a new change on the branch specifically to perform a cross
# branch merge.
#

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

activity="new change 441"
cat > caf << 'fubar'
brief_description = "one point ten";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 450"
$bin/aegis -db 11 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 454"
$bin/aegis -cp $work/test.1.C011/fred > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="cross branch merge 458"
$bin/aegis -diff -branch '' -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

mv $work/test.1.C011/fred,D $work/test.1.C011/fred
if test $? -ne 0 ; then no_result; fi

activity="check change file state 465"
cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "1.2";
			encoding = none;
		};
		edit_origin_new =
		{
			revision = "1.3";
			encoding = none;
		};
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/001.branch/0/011.fs

activity="build 481"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 485"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 489"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 493"
$bin/aegis -rpass 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 497"
$bin/aegis -ib 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 501"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 505"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 509"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check branch file state 513"
cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "1.4";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.3";
			encoding = none;
		};
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
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/001.fs

#
# now the branch should end development cleanly
#
AEGIS_PROJECT=test
export AEGIS_PROJECT

$bin/aegis -de -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -rpass -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -ib -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
