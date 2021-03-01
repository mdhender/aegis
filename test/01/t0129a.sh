#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2001, 2002, 2004-2008 Peter Miller
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
	log_file_preference = never;"
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

activity="working directory 96"
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
activity="new preject 118"
$bin/aegis -npr test -vers - -dir $work/proj -lib $work/lib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="project attributes 125"
cat > paf << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="new developer 136"
$bin/aegis -nd $USER > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi
activity="new reviewer 139"
$bin/aegis -nrv $USER > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi
activity="new integrator 142"
$bin/aegis -ni $USER > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="new branch 146"
$bin/aegis -nbr -p test 1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# The first change creates a file ---------------------------------------------

AEGIS_PROJECT=test.1
export AEGIS_PROJECT

activity="new change 155"
cat > caf << 'fubar'
brief_description = "change the first";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 10 -f caf -p test.1 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop begin 168"
$bin/aegis -db 10 -dir $work/chan > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="new file 172"
$bin/aegis -nf $work/chan/aegis.conf $work/chan/fred > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > $work/chan/aegis.conf << 'fubar'
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

activity="build 192"
$bin/aegis -b > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="diff 196"
$bin/aegis -diff > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop end 200"
$bin/aegis -de > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="review pass 204"
$bin/aegis -rpass 10 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate begin 208"
$bin/aegis -ib 10 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="diff 212"
$bin/aegis -diff > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="build 216"
$bin/aegis -b > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 220"
$bin/aegis -ipass > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# The second change modifies the file ------------------------------------------

activity="new change 226"
cat > caf << 'fubar'
brief_description = "change the second";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 11 -f caf -p test.1 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop begin 239"
$bin/aegis -db 11 -dir $work/chan > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="modify file 243"
$bin/aegis -cp $work/chan/fred > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

echo yuck > $work/chan/fred
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="build 250"
$bin/aegis -b > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="diff 254"
$bin/aegis -diff > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop end 258"
$bin/aegis -de > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="review pass 262"
$bin/aegis -rpass 11 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate begin 266"
$bin/aegis -ib 11 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="diff 270"
$bin/aegis -diff > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="build 274"
$bin/aegis -b > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 278"
$bin/aegis -ipass > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# Now develop-end the branch ---------------------------------------------------

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="develop end branch 287"
$bin/aegis -de -p test 1 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# Verify branch file state ----------------------------------------------------

activity="verify branch file state 293"

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
		file_name = "fred";
		uuid = "UUID";
		action = create;
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
];
fubar
if test $? -ne 0 ; then cat LOG; no_result; fi

check_it ok $work/proj/info/change/0/001.fs

# Verify trunk file state ----------------------------------------------------

activity="verify trunk file state 362"

cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = transparent;
		usage = config;
		locked_by = 1;
		about_to_be_created_by = 1;
	},
	{
		file_name = "fred";
		uuid = "UUID";
		action = transparent;
		usage = source;
		locked_by = 1;
		about_to_be_created_by = 1;
	},
];
fubar
if test $? -ne 0 ; then cat LOG; no_result; fi

check_it ok $work/proj/info/trunk.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
