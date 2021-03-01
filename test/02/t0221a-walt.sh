#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004 Walter Franzini;
#	All rights reserved.
#       Copyright (C) 2007, 2008 Peter Miller
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

activity="new project 130"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT


activity="project attributes 139"
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

activity="staff 152"
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
activity="new branch 164"
$bin/aegis -nbr -p $AEGIS_PROJECT 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

proj=$AEGIS_PROJECT

activity="new change 173"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 183"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 187"
$bin/aegis -nf $work/${proj}.C010/aegis.conf $work/${proj}.C010/fred \
	$work/${proj}.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/${proj}.C010/aegis.conf << 'fubar'
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
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/${proj}.C010/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/${proj}.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="build 214"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 218"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 222"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 226"
$bin/aegis -ib -mini 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 230"
delta_dir=`$bin/aegis -cd`
test -f $delta_dir/aegis.conf || no_result
test -f $delta_dir/fred || no_result
test -f $delta_dir/barney || no_result

activity="integrate build 236"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 240"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 244"
delta_dir=`$bin/aegis -cd`
test -f $delta_dir/aegis.conf || no_result
test -f $delta_dir/fred || no_result
test -f $delta_dir/barney || no_result

activity="integrate pass 250"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 254"
bl_dir=$work/proj.dir/branch.1/baseline
test -f $bl_dir/aegis.conf || no_result
test -f $bl_dir/fred || no_result
test -f $bl_dir/barney || no_result


activity="new branch 261"
$bin/aegis -nbr -p $proj 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=test.1.1
export AEGIS_PROJECT

proj=$AEGIS_PROJECT

activity="new change 270"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf 1 -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 280"
$bin/aegis -db 1 -dir $work/$proj.C001 >  log 2>&1
if test $? -ne 0; then cat log; no_result; fi

$bin/aegis -cp 1 $work/$proj.C001/fred -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

echo 'append something' >> $work/$proj.C001/fred
if test $? -ne 0; then no_result; fi

activity="create a new aegis.conf file 290"
$bin/aegis -nf -config $work/$proj.C001/test.conf -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > $work/$proj.C001/test.conf <<'EOF'
/*
 * we are not testing tests, we only need a aegis.conf file somewhere.
 */
new_test_filename =
        "test/${zpad $hundred 2}/"
        "t${zpad $number 4}${left $type 1}.sh";
EOF
if test $? -ne 0; then no_result; fi

activity="build 304"
$bin/aegis -build 1  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 308"
$bin/aegis -diff 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="dev_end 312"
$bin/aegis -dev_end 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="ibegin 316"
$bin/aegis -ib  1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check dir content 320"
delta_dir=`$bin/aegis -cd -c 1`
test -f $delta_dir/fred || no_result
test -f $delta_dir/test.conf || no_result

activity="build 325"
$bin/aegis -build 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 329"
delta_dir=`$bin/aegis -cd -c 1`
test -f $delta_dir/fred || no_result
test -f $delta_dir/derived1 || no_result
test -f $delta_dir/derived2 || no_result

activity="diff  335"
$bin/aegis -diff 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 339"
$bin/aegis -ipass 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 343"
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
			revision = "2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1";
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
			revision = "1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1";
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

activity="close branch.1 415"
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

activity="check the baseline content 429"
bl_dir=$work/proj.dir/branch.1/baseline
test -f $bl_dir/fred || no_result
test -f $bl_dir/test.conf || no_result
test -f $bl_dir/derived1 || no_result
test -f $bl_dir/derived2 || no_result


activity="new change 437"
cat > caf <<EOF
brief_description = "two";
cause = internal_enhancement;
test_baseline_exempt = true;
EOF
if test $? -ne 0; then no_result; fi

$bin/aegis -nc -p $AEGIS_PROJECT -f caf 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 448"
$bin/aegis -db 2 -dir $work/$proj.C002 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="remove aegis.conf file 452"
$bin/aegis -rm $work/$proj.C002/test.conf -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 456"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 460"
$bin/aegis -build -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop_end 464"
$bin/aegis -dev_end 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 468"
$bin/aegis -ib 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 472"
$bin/aegis -build 2 -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 476"
$bin/aegis -diff 2 -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 480"
$bin/aegis -ipass 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the baseline content 484"
bl_dir=$work/proj.dir/branch.1/baseline
test -f $bl_dir/test.conf && no_result
test -f $bl_dir/derived1 || no_result
test -f $bl_dir/derived2 || no_result

cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1";
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
		file_name = "barney";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1";
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
		file_name = "fred";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "3";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1";
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
			revision = "2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "2";
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

activity="send the change 606"
$bin/aedist -send -p  test.1.1 -c 1 -o c1.ae -v > log 2>&1
if test $? -ne 0; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
