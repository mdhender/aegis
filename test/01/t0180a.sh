#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2003 Peter Miller;
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
# MANIFEST: Test the aet -reg functionality
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

activity="create test directory 100"
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
# 2. Create a changeset that adds a config file.  Develop and integrate.
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

export AEGIS_PATH=$AEGIS_PATH:$libDir

activity="create project 159"
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
activity="add staff 188"
$bin/aegis -nd $USER -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -nrv $USER -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -ni $USER -v -p $theProject.1.0 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -ni $USER -v -p $theProject.1 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

###### Create the first change  -- adds config file ######

# Attributes common to all changesets
cat > commonChangeAttributes << 'EOF'
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
EOF
if test $? -ne 0 ; then no_result; fi

activity="new change 212"
cat > changeAttributes << 'EOF'
description = "Change 1 - create config";
brief_description = "Change 1 - create config";
EOF
if test $? -ne 0 ; then no_result; fi

# Add the common attibutes
cat commonChangeAttributes >> changeAttributes
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 10 -file changeAttributes -p $theProject.1.0 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop begin 226"
$bin/aegis -db 10 -v -dir $chanDir > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# Add the config file
activity="new file 231"
$bin/aegis -nf $chanDir/config -v -c 10 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > $chanDir/config << 'EOF'
build_command = "exit 0";
link_integration_directory = true;
history_create_command = "ci -f -u -m$c -t/dev/null $i $h,v; rcs -U $h,v";
history_get_command = "co -r'$e' -p $h,v > $o";
history_put_command = "ci -f -u -m$c -t/dev/null $i $h,v; rcs -U $h,v";
history_query_command = "rlog -r $h,v | awk '/^head:/ {print $$2}'";
diff_command = "fcomp -w -s $original $input -o $output";
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

# allow timestamps to be different
sleep 2

# build
activity="build 261"
$bin/aegis -build -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# diff
activity="diff 266"
$bin/aegis -diff -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# develop end
activity="develop end 271"
$bin/aegis -de -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# integrate
activity="integrate begin 276"
$bin/aegis -ib -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate build 280"
$bin/aegis -build -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate diff 284"
$bin/aegis -diff -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 288"
$bin/aegis -ipass -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi


###### Create the second change -- add a test and a hosttest source file ######

activity="new change 295"
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

activity="develop begin 309"
$bin/aegis -db 11 -v -dir $chanDir > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="new test 313"
$bin/aegis -nt $chanDir/test/00/t0001a.sh -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > $chanDir/test/00/t0001a.sh << 'EOF'
#!/bin/sh
exit 0
EOF
if test $? -ne 0 ; then no_result; fi

activity="new file 323"
$bin/aegis -nf $chanDir/hosttest/0001/main.cc -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > $chanDir/hosttest/0001/main.cc << EOF
Does not matter what is in here.
EOF
if test $? -ne 0 ; then no_result; fi

# turn off "test -bl"
activity="change attributes 333"
$bin/aegis -ca -file changeAttributes -c 11 > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

sleep 2

# build
activity="build 340"
$bin/aegis -build -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# diff
activity="diff 345"
$bin/aegis -diff -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# test
activity="test 350"
$bin/aegis -test -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# develop end
activity="develop end 355"
$bin/aegis -de -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# integrate
activity="integrate begin 360"
$bin/aegis -ib -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate build 364"
$bin/aegis -build -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate diff 368"
$bin/aegis -diff -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate test 372"
$bin/aegis -test -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 376"
$bin/aegis -ipass -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# Roll in the branch
activity="branch develop end 381"
$bin/aegis -de -c 0 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="branch integrate begin 385"
$bin/aegis -ib -c 0 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="branch integrate build 389"
$bin/aegis -build -c 0 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="branch integrate diff 393"
$bin/aegis -diff -c 0 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="branch integrate pass 397"
$bin/aegis -ipass -c 0 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi


###### Create a new branch ######

activity="new branch 404"
$bin/aegis -nbr 1 -p $theProject.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="branch staff 408"
$bin/aegis -nd $USER -p $theProject.1.1 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

AEGIS_PROJECT=$theProject.1.1
export AEGIS_PROJECT

###### Create a new change -- removes the hosttest ######

activity="new change 417"
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

activity="develop begin 432"
$bin/aegis -db 10 -v -dir $chanDir > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="remove file 436"
$bin/aegis -rm $chanDir/hosttest/0001/main.cc > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

sleep 2

# build
activity="build 443"
$bin/aegis -build -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# diff
activity="diff 448"
$bin/aegis -diff -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# test -reg
activity="test -reg 453"
$bin/aegis -test -reg -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# develop end
activity="develop end 458"
$bin/aegis -de -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# integrate
activity="integrate begin 463"
$bin/aegis -ib -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate build 467"
$bin/aegis -build -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate diff 471"
$bin/aegis -diff -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate test reg 475"
$bin/aegis -test -reg -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 479"
$bin/aegis -ipass -c 10 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

###### Create a new change -- reinstates the removed hosttest file ######

activity="new change 485"
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

activity="develop begin 500"
$bin/aegis -db 11 -v -dir $chanDir > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="new file 504"
$bin/aegis -nf $chanDir/hosttest/0001/main.cc -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# Write to the hosttest
cat > $chanDir/hosttest/0001/main.cc << EOF
Reinstated hosttest
EOF
if test $? -ne 0 ; then no_result; fi

sleep 2

# build
activity="build 517"
$bin/aegis -build -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# diff
activity="diff 522"
$bin/aegis -diff -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

# test -reg
activity="test reg 527"
$bin/aegis -test -reg -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

action="check project file state"
cat > ok << 'EOF'
src =
[
	{
		file_name = "config";
		action = create;
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
		file_name = "hosttest/0001/main.cc";
		action = create;
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
		action = create;
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
activity="develop end 660"
$bin/aegis -de -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

action="check project file state"
cat > ok << 'EOF'
src =
[
	{
		file_name = "hosttest/0001/main.cc";
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
activity="integrate begin 702"
$bin/aegis -ib -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate build 706"
$bin/aegis -build -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate diff 710"
$bin/aegis -diff -c 11 -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

#### The test -reg command will fail, if the bug is present. ####

activity="integrate test reg 716"
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
