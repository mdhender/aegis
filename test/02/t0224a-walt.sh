#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2006-2008 Peter Miller
#	Copyright (C) 2005, 2006 Walter Franzini;
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
	log_file_preference = never; \
	default_development_directory = \"$work\";"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

# This tells aeintegratq that it is being used by a test.
AEGIS_TEST_DIR=$work
export AEGIS_TEST_DIR

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
	echo "FAILED test of the aedist functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aedist functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 106"
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
# If the C++ compiler is called something other than "c++", as
# discovered by the configure script, create a shell script called
# "c++" which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
#
if test "$CXX" != "c++"
then
	cat >> $work/c++ << fubar
#!/bin/sh
exec ${CXX-g++} \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx $work/c++
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# test the aedist functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 153"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 160"
cat > tmp << 'end'
description = "A bogus project created to test the aemv/merge "
    "functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 177"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

activity="new branch 187"
$bin/aegis -nbr -p $AEGIS_PROJECT 10 -v > log 2>&1

AEGIS_PROJECT=${AEGIS_PROJECT}.10
export AEGIS_PROJECT

# 11111111111111111111111111111111111111111111111111111111111111111111111111

#
# create a new change
#
activity="new change 198"
cat > tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 210"
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 217"
$bin/aegis -nf  $workchan/bogus1 -nl \
	--uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/bogus2 -nl \
	--uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/aegis.conf -nl \
	--uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus1 << 'end'
bogus1, line 1
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/bogus2 << 'end'
line one
line two
line three
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "echo build not required";
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
merge_command = "exit 1 # $input $output $orig $most_recent";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 259"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 266"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -change_attr --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd0 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 276"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 283"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 287"
$bin/aegis -diff 1 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# integrate build
#
activity="build 294"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 301"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# 22222222222222222222222222222222222222222222222222222222222222222222222222

#
# create a new change
#
activity="new change 310"
cat > tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 322"
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify bogus1 326"
$bin/aegis -cp 2 $workchan/bogus1 -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat >> $workchan/bogus1 <<EOF
append some text
EOF
if test $? -ne 0; then no_result; fi

activity="modify bogus2 335"
$bin/aegis -cp 2 $workchan/bogus2 -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat >> $workchan/bogus2 <<EOF
append some text
EOF
if test $? -ne 0; then no_result; fi

#
# Copy aegis conf, this will stop aedist -rec before running the
# merge_command.
#
activity="copy aegis.conf 348"
$bin/aegis -cp 2 $workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="diff the change 352"
$bin/aegis -diff 2 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="build the change 356"
$bin/aegis -build 2 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="set change's UUID 360"
$bin/aegis -change_attr 2 --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd6 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 364"
$bin/aegis -dev_end 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate begin 368"
$bin/aegis -ibegin 2  -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="diff the change 372"
$bin/aegis -diff 2 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="build the change 376"
$bin/aegis -build 2 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate pass 380"
$bin/aegis -ipass 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

# 33333333333333333333333333333333333333333333333333333333333333333333333333

#
# create a new change
#
activity="new change 389"
cat > tmp << 'end'
brief_description = "The third change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 401"
$bin/aegis -db 3 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify bogus1 405"
$bin/aegis -cp 3 $workchan/bogus1 -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > $workchan/bogus1 <<EOF
the penguin
crows
at midnight
EOF
if test $? -ne 0; then no_result; fi

activity="modify bogus2 416"
$bin/aegis -cp 3 $workchan/bogus2 -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > $workchan/bogus2 <<EOF
less
is
more
EOF
if test $? -ne 0; then no_result; fi

#
# Copy aegis conf, this will stop aedist -rec before running the
# merge_command.
#
activity="copy aegis.conf 431"
$bin/aegis -cp 3 $workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="diff the change 435"
$bin/aegis -diff 3 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="build the change 439"
$bin/aegis -build 3 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="set change's UUID 443"
$bin/aegis -change_attr 3 --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd7 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 447"
$bin/aegis -dev_end 3 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate begin 451"
$bin/aegis -ibegin 3  -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="diff the change 455"
$bin/aegis -diff 3 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="build the change 459"
$bin/aegis -build 3 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate pass 463"
$bin/aegis -ipass 3 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

# 44444444444444444444444444444444444444444444444444444444444444444444444444

activity="send the change 469"
$bin/aedist -send -c 2 -ndh -cte=none -nocomp -o c2.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="test the archive 473"
mkdir tmp.d > log 2>&1
if test $? -ne 0; then cat log; no_result; fi
$bin/test_cpio --extract --change_dir $work/tmp.d --file c2.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="change-set 479"
cat > ok <<EOF
brief_description = "The second change";
description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
state = awaiting_development;
uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd6";
src =
[
	{
		file_name = "aegis.conf";
		uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3";
		action = modify;
		usage = config;
		attribute =
		[
			{
				name = "edit-origin-UUID";
				value = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd0";
			},
		];
	},
	{
		file_name = "bogus1";
		uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
		action = modify;
		usage = source;
		attribute =
		[
			{
				name = "edit-origin-UUID";
				value = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd0";
			},
		];
	},
	{
		file_name = "bogus2";
		uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
		action = modify;
		usage = source;
		attribute =
		[
			{
				name = "edit-origin-UUID";
				value = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd0";
			},
		];
	},
];
EOF
if test $? -ne 0; then no_result; fi

diff ok tmp.d/etc/change-set
if test $? -ne 0; then cat log; fail; fi

#
# Receive the change, make the trojan check mandatory.
#
activity="receive the change 540"
$bin/aedist -rec -c 4 -f c2.ae -trojan -ignore-uuid > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="fstate check 544"
cat > ok <<EOF
src =
[
	{
		file_name = "aegis.conf";
		uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3";
		action = modify;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = config;
	},
	{
		file_name = "bogus1";
		uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
		action = modify;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
	},
	{
		file_name = "bogus2";
		uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
		action = modify;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
	},
];
EOF
if test $? -ne 0; then no_result; fi

diff ok $workproj/info/change/0/010.branch/0/004.fs
if test $? -ne 0; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
