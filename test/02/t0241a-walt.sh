#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2006, 2008 Walter Franzini
#	Copyright (C) 2006-2008 Peter Miller
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

AEGIS_DATADIR=$here/lib
export AEGIS_DATADIR

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
	echo "FAILED test of the aeipass ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aeipass ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/uuid = ".*"/uuid = "UUID"/' \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

trap \"no_result\" 1 2 3 15

activity="create test directory 114"
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
if test "$CXX" != "" -a "$CXX" != "c++"
then
	cat >> $work/c++ << fubar
#!/bin/sh
exec $CXX \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx $work/c++
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# test the aeipass functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 161"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 168"
cat > tmp << 'end'
description = "A bogus project created to test the "
    "aeipass functionality.";
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
activity="staff 185"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch 193"
$bin/aegis -p $AEGIS_PROJECT -nbr 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=example.1
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 203"
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
activity="develop begin 215"
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 222"
$bin/aegis -nf  $workchan/bogus1 -nl \
	--uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/aegis.conf -nl \
	--uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus1 << 'end'
bogus1, line 1
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
link_integration_directory = true;
history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;
diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
merge_command = "exit 0 # $input $output $orig $most_recent";
patch_diff_command = "set +e; diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 254"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 261"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 268"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 275"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 279"
$bin/aegis -diff 1 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# integrate build
#
activity="build 286"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 293"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Hack to [p]fstate files to simulate a change integrated with an
# Aegis version lacking support for file's UUIDs.
#
grep -v uuid $workproj/info/change/0/001.branch/0/001.pfs > $work/pfs
if test $? -ne 0; then no_result; fi

grep -v uuid $workproj/info/change/0/001.branch/0/001.fs > $work/fs
if test $? -ne 0; then no_result; fi

cp $work/pfs $workproj/info/change/0/001.branch/0/001.pfs
if test $? -ne 0; then no_result; fi

cp $work/fs  $workproj/info/change/0/001.branch/0/001.fs
if test $? -ne 0; then no_result; fi

#
# Now we close the branch.
#
AEGIS_PROJECT=example
export AEGIS_PROJECT

activity="develop end the branch 319"
$bin/aegis -dev_end 1 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="ibegin 323"
$bin/aegis -ibegin 1 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="ipass the branch 327"
$bin/aegis -ipass 1 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
src =
[
	{
		file_name = "aegis.conf";
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
	},
	{
		file_name = "bogus1";
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
	},
];
EOF
if test $? -ne 0; then no_result; fi

check_it $work/ok $workproj/info/change/0/001.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
