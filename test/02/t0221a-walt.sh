#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004 Walter Franzini;
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
# MANIFEST: Test the change_pconf_get functionality
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

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

#
# set the path, so that the aegis command that aepatch/aedist invokes
# is from the same test set as the aepatch/aedist command itself.
#
PATH=${bin}:$PATH
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
	echo "FAILED test of the change_pconf_get functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
        echo "NO RESULT when testing the change_pconf_get functionality " \
            "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 92"
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
		-e 's/uuid = ".*"/uuid = "UUID"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

#
# test the change_pconf_get functionality
#
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="new project 129"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT


activity="project attributes 138"
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

activity="staff 151"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# We do not use the trunk because it's special, it does not have a
# parent, and this condition make it special also in the code.
#
activity="new branch 163"
$bin/aegis -nbr -p $AEGIS_PROJECT 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

proj=$AEGIS_PROJECT

activity="new change 172"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 182"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 186"
$bin/aegis -nf $work/${proj}.C010/config $work/${proj}.C010/fred \
	$work/${proj}.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/${proj}.C010/config << 'fubar'
build_command = "date > derived1 && date > derived2";
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
link_integration_directory = true;
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/${proj}.C010/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/${proj}.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="build 213"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 217"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 221"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 225"
$bin/aegis -ib -mini 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 229"
delta_dir=`$bin/aegis -cd`
test -f $delta_dir/config || no_result
test -f $delta_dir/fred || no_result
test -f $delta_dir/barney || no_result

activity="integrate build 235"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 239"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 243"
delta_dir=`$bin/aegis -cd`
test -f $delta_dir/config || no_result
test -f $delta_dir/fred || no_result
test -f $delta_dir/barney || no_result

activity="integrate pass 249"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 253"
bl_dir=$work/proj.dir/branch.1/baseline
test -f $bl_dir/config || no_result
test -f $bl_dir/fred || no_result
test -f $bl_dir/barney || no_result


activity="new branch 260"
$bin/aegis -nbr -p $proj 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=test.1.1
export AEGIS_PROJECT

proj=$AEGIS_PROJECT

activity="new change 269"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf 1 -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 279"
$bin/aegis -db 1 -dir $work/$proj.C001 >  log 2>&1
if test $? -ne 0; then cat log; no_result; fi

$bin/aegis -cp 1 $work/$proj.C001/fred -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

echo 'append something' >> $work/$proj.C001/fred
if test $? -ne 0; then no_result; fi

activity="create a new config file 289"
$bin/aegis -nf -config $work/$proj.C001/test.conf -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > $work/$proj.C001/test.conf <<'EOF'
/*
 * we are not testing tests, we only need a config file somewhere.
 */
new_test_filename =
        "test/${zpad $hundred 2}/"
        "t${zpad $number 4}${left $type 1}.sh";
EOF
if test $? -ne 0; then no_result; fi

activity="build 303"
$bin/aegis -build 1  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 307"
$bin/aegis -diff 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="dev_end 311"
$bin/aegis -dev_end 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="ibegin 315"
$bin/aegis -ib  1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check dir content 319"
delta_dir=`$bin/aegis -cd -c 1`
test -f $delta_dir/fred || no_result
test -f $delta_dir/test.conf || no_result

activity="build 324"
$bin/aegis -build 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 328"
delta_dir=`$bin/aegis -cd -c 1`
test -f $delta_dir/fred || no_result
test -f $delta_dir/derived1 || no_result
test -f $delta_dir/derived2 || no_result

activity="diff  334"
$bin/aegis -diff 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 338"
$bin/aegis -ipass 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 342"
bl_dir=$work/proj.dir/branch.1/branch.1/baseline
test -f $bl_dir/fred || no_result
test -f $bl_dir/test.conf || no_result
test -f $bl_dir/derived1 || no_result
test -f $bl_dir/derived2 || no_result

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
	{
		file_name = "test.conf";
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

check_it ok $work/proj.dir/info/change/0/001.branch/0/001.fs

activity="close branch.1 349"
AEGIS_PROJECT=test.1
export AEGIS_PROJECT
$bin/aegis -dev_end -p $AEGIS_PROJECT 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ib -p $AEGIS_PROJECT 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -build 1 -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -diff 1 -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ipass -p $AEGIS_PROJECT 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 363"
bl_dir=$work/proj.dir/branch.1/baseline
test -f $bl_dir/fred || no_result
test -f $bl_dir/test.conf || no_result
test -f $bl_dir/derived1 || no_result
test -f $bl_dir/derived2 || no_result


activity="new change 371"
cat > caf <<EOF
brief_description = "two";
cause = internal_enhancement;
test_baseline_exempt = true;
EOF
if test $? -ne 0; then no_result; fi

$bin/aegis -nc -p $AEGIS_PROJECT -f caf 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 382"
$bin/aegis -db 2 -dir $work/$proj.C002 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="remove config file 386"
$bin/aegis -rm $work/$proj.C002/test.conf -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 390"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 394"
$bin/aegis -build -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop_end 398"
$bin/aegis -dev_end 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 402"
$bin/aegis -ib 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 406"
$bin/aegis -build 2 -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 410"
$bin/aegis -diff 2 -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 414"
$bin/aegis -ipass 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 418"
bl_dir=$work/proj.dir/branch.1/baseline
test -f $bl_dir/test.conf && no_result
test -f $bl_dir/derived1 || no_result
test -f $bl_dir/derived2 || no_result

cat > ok << 'fubar'
src =
[
	{
		file_name = "barney";
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
		diff_file_fp =
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
	{
		file_name = "test.conf";
		uuid = "UUID";
		action = remove;
		edit =
		{
			revision = "1.2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.2";
			encoding = none;
		};
		usage = config;
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		deleted_by = 2;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi

check_it ok $work/proj.dir/info/change/0/001.fs

activity="send the change 424"
$bin/aedist -send -p  test.1.1 -c 1 -o c1.ae -v > log 2>&1
if test $? -ne 0; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
