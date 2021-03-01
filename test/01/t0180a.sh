#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2003-2008 Peter Miller
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
	echo "FAILED test of the aet -reg functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aet -reg functionality" \
		"($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 101"
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
# test the aet -reg functionality
#

#
# This shell reproduces a bug in aegis 4.11, whereby a file which has been
# removed and then reinstated causes test_main() to not be able to "locate"
# files in the baseline, that are alphabetically later than the
# removed/reinstated file.
#
# Someone wanting to run this script should check theProject and
# theLib names specified below, as well as the projectDir/libDir variables
# to ensure that they are OK for their system.
#
# This script will write a couple of temporary files to the user's home
# directory (they are deleted at the end).
#
# Basically, what this script does is:
# 1. Create a project with a 1.0 branch.
# 2. Create a changeset that adds a aegis.conf file.  Develop and integrate.
# 3. Create a changeset that adds a test and a hosttest source file.  D & I.
# 4. Roll in the 1.0 branch.
# 5. Create a 1.1 branch.
# 6. Create a changeset that deletes the hosttest.  D & I.
# 7. Create a changeset that "aenf's" the same hosttest.  Develop, but
#    integration "aet -reg" does not work (cannot find the test).
#
# I've found that if the hosttest is created, removed and reinstated all
# within the same branch, this bug does not happen (the branch rollin
# described above is required).
#
# This shell script is a bit verbose -- I could have put more effort into
# putting duplicated code into functions.  Oh well.
#

# Set up some variables for use below
theProject=foo
projectDir=$work/$theProject
libDir=$work/lib
chanDir=$work/chan-dev-dir

AEGIS_PATH=$AEGIS_PATH:$libDir
export AEGIS_PATH

activity="create project 161"
# Write the project attributes file
cat > projAttributes << 'EOF'
description="Test project";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
developers_may_create_changes = true;
umask = 022;
default_test_exemption = false;
minimum_change_number = 10;
reuse_change_numbers = true;
minimum_branch_number = 1;
skip_unlucky = false;
compress_database = false;
develop_end_action = goto_awaiting_integration;
protect_development_directory = false;
EOF
if test $? -ne 0 ; then no_result; fi

# Create the project
$bin/aegis -npr $theProject -dir $projectDir -LIB $libDir -file projAttributes \
	-v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

AEGIS_PROJECT=$theProject.1.0
export AEGIS_PROJECT

# Add staff
activity="add staff 190"
$bin/aegis -nd $USER -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -nrv $USER -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -ni $USER -v -p $theProject.1.0 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -ni $USER -v -p $theProject.1 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

###### Create the first change  -- adds aegis.conf file ######

# Attributes common to all changesets
cat > commonChangeAttributes << 'EOF'
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
EOF
if test $? -ne 0 ; then no_result; fi

activity="new change 214"
cat > changeAttributes << 'EOF'
description = "Change 1 - create aegis.conf";
brief_description = "Change 1 - create aegis.conf";
EOF
if test $? -ne 0 ; then no_result; fi

# Add the common attibutes
cat commonChangeAttributes >> changeAttributes
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 10 -file changeAttributes -p $theProject.1.0 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop begin 228"
$bin/aegis -db 10 -v -dir $chanDir > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# Add the aegis.conf file
activity="new file 233"
$bin/aegis -nf $chanDir/aegis.conf -v -c 10 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > $chanDir/aegis.conf << 'EOF'
build_command = "exit 0";
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command =
    "set +e; "
    "diff ${quote $original} ${quote $input} > ${quote $output}; "
    "test $? -le 1";
merge_command =
	"fmerge $original $MostRecent $input -o $output -c /dev/null";
posix_filename_charset = true;
maximum_filename_length = 255;
change_file_command = "rm -f etc/cook/change.[0-9]* etc/cook/project.[0-9]*";
project_file_command = "rm -f etc/cook/project.[0-9]*";
integrate_begin_command = "rm -f etc/cook/project.[0-9]* \
etc/version.h etc/statistics/*.stats";
develop_begin_command = "ln -s $bl bl; ln -s bl/.indent.pro .indent.pro";
test_command = "$shell $filename";
new_test_filename = "test/${zpad $hundred 2}/t${zpad $number 4}.sh";
EOF
if test $? -ne 0 ; then no_result; fi

# build
activity="build 266"
$bin/aegis -build -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# diff
activity="diff 271"
$bin/aegis -diff -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# develop end
activity="develop end 276"
$bin/aegis -de -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# integrate
activity="integrate begin 281"
$bin/aegis -ib -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate build 285"
$bin/aegis -build -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate diff 289"
$bin/aegis -diff -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 293"
$bin/aegis -ipass -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi


###### Create the second change -- add a test and a hosttest source file ######

activity="new change 300"
cat > changeAttributes << 'EOF'
description = "Change 2 - add test and hosttest source";
brief_description = "Change 2";
EOF
if test $? -ne 0 ; then no_result; fi

# Add the common attributes
cat commonChangeAttributes >> changeAttributes
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 11 -file changeAttributes -p $theProject.1.0 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop begin 314"
$bin/aegis -db 11 -v -dir $chanDir > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="new test 318"
$bin/aegis -nt $chanDir/test/00/t0001a.sh -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > $chanDir/test/00/t0001a.sh << 'EOF'
#!/bin/sh
exit 0
EOF
if test $? -ne 0 ; then no_result; fi

activity="new file 328"
$bin/aegis -nf $chanDir/hosttest/0001/main.cc -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > $chanDir/hosttest/0001/main.cc << EOF
Does not matter what is in here.
EOF
if test $? -ne 0 ; then no_result; fi

# turn off "test -bl"
activity="change attributes 338"
$bin/aegis -ca -file changeAttributes -c 11 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# build
activity="build 345"
$bin/aegis -build -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# diff
activity="diff 350"
$bin/aegis -diff -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# test
activity="test 355"
$bin/aegis -test -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# develop end
activity="develop end 360"
$bin/aegis -de -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# integrate
activity="integrate begin 365"
$bin/aegis -ib -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate build 369"
$bin/aegis -build -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate diff 373"
$bin/aegis -diff -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate test 377"
$bin/aegis -test -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 381"
$bin/aegis -ipass -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# Roll in the branch
activity="branch develop end 386"
$bin/aegis -de -c 0 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="branch integrate begin 390"
$bin/aegis -ib -c 0 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="branch integrate build 394"
$bin/aegis -build -c 0 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="branch integrate diff 398"
$bin/aegis -diff -c 0 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="branch integrate pass 402"
$bin/aegis -ipass -c 0 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi


###### Create a new branch ######

activity="new branch 409"
$bin/aegis -nbr 1 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="branch staff 413"
$bin/aegis -nd $USER -p $theProject.1.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

AEGIS_PROJECT=$theProject.1.1
export AEGIS_PROJECT

###### Create a new change -- removes the hosttest ######

activity="new change 422"
cat > changeAttributes << 'EOF'
description = "Change 1 -- removes the hosttest";
brief_description = "Change 1";
EOF
if test $? -ne 0 ; then no_result; fi

# Add the common attributes
sed 's/regression_test_exempt = true/regression_test_exempt = false/' \
	commonChangeAttributes >> changeAttributes
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 10 -file changeAttributes -p $theProject.1.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop begin 437"
$bin/aegis -db 10 -v -dir $chanDir > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="remove file 441"
$bin/aegis -rm $chanDir/hosttest/0001/main.cc > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# build
activity="build 448"
$bin/aegis -build -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# diff
activity="diff 453"
$bin/aegis -diff -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# test -reg
activity="test -reg 458"
$bin/aegis -test -reg -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# develop end
activity="develop end 463"
$bin/aegis -de -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# integrate
activity="integrate begin 468"
$bin/aegis -ib -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate build 472"
$bin/aegis -build -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate diff 476"
$bin/aegis -diff -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate test reg 480"
$bin/aegis -test -reg -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 484"
$bin/aegis -ipass -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

###### Create a new change -- reinstates the removed hosttest file ######

activity="new change 490"
cat > changeAttributes << EOF
description = "Change 2 - reinstates the removed hosttest";
brief_description = "Change 2";
EOF
if test $? -ne 0 ; then no_result; fi

# Add the common attributes
sed 's/regression_test_exempt = true/regression_test_exempt = false/' \
	commonChangeAttributes >> changeAttributes
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 11 -file changeAttributes -p $theProject.1.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop begin 505"
$bin/aegis -db 11 -v -dir $chanDir > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="new file 509"
$bin/aegis -nf $chanDir/hosttest/0001/main.cc -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# Write to the hosttest
cat > $chanDir/hosttest/0001/main.cc << EOF
Reinstated hosttest
EOF
if test $? -ne 0 ; then no_result; fi

# build
activity="build 522"
$bin/aegis -build -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# diff
activity="diff 527"
$bin/aegis -diff -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# test -reg
activity="test reg 532"
$bin/aegis -test -reg -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

action="check project file state"
cat > ok << 'EOF'
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = create;
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
		file_name = "hosttest/0001/main.cc";
		uuid = "UUID";
		action = create;
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
		test =
		[
			"test/00/t0001a.sh",
		];
	},
	{
		file_name = "test/00/t0001a.sh";
		uuid = "UUID";
		action = create;
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
		usage = test;
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
EOF
if test $? -ne 0 ; then no_result; fi

check_it ok $projectDir/info/change/0/001.fs

cat > ok << 'EOF'
src =
[
	{
		file_name = "hosttest/0001/main.cc";
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
		usage = source;
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		deleted_by = 10;
		test =
		[
			"test/00/t0001a.sh",
		];
	},
];
EOF
check_it ok $projectDir/info/change/0/001.branch/0/001.fs

# develop end
activity="develop end 669"
$bin/aegis -de -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

action="check project file state"
cat > ok << 'EOF'
src =
[
	{
		file_name = "hosttest/0001/main.cc";
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
		usage = source;
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		locked_by = 11;
		deleted_by = 10;
		test =
		[
			"test/00/t0001a.sh",
		];
	},
];
EOF
if test $? -ne 0 ; then no_result; fi

check_it ok $projectDir/info/change/0/001.branch/0/001.fs

# integrate
activity="integrate begin 712"
$bin/aegis -ib -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate build 716"
$bin/aegis -build -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate diff 720"
$bin/aegis -diff -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

#### The test -reg command will fail, if the bug is present. ####

activity="integrate test reg 726"
$bin/aegis -test -reg -c 11 -v \
	--trace aet change file nth find list_get \
	> LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
