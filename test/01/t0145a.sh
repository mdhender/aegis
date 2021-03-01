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
workproj=$work/proj
tmp=$work/tmp

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
	echo "FAILED test of the -dpt functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the -dpt functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

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
# make a new project
#
activity="new project 129"
$bin/aegis -npr foo -dir $workproj -LIB $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch 133"
$bin/aegis -nbr 2 -p foo -LIB $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="aena 138"
$bin/aegis -na $USER -Descend_Project_Tree -p foo.1 -lib $work/lib
if test $? -ne 0 ; then cat log; fail; fi

activity="aend 139"
$bin/aegis -nd $USER -Descend_Project_Tree -p foo.1 -lib $work/lib
if test $? -ne 0 ; then cat log; fail; fi

activity="aeni 143"
$bin/aegis -ni $USER -Descend_Project_Tree -p foo.1 -lib $work/lib
if test $? -ne 0 ; then cat log; fail; fi

activity="aenrv 147"
$bin/aegis -nrv $USER -Descend_Project_Tree -p foo.1 -lib $work/lib
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -list developers -p foo -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

activity="check dev 154"
test ! -s $work/file
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -list developers -p foo.1 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

diff $work/file - << EOF
$USER
EOF
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -list developers -p foo.1.0 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi
diff $work/file - << EOF
$USER
EOF
if test $? -ne 0 ; then cat log; fail; fi

activity="check int 177"
$bin/aegis -list integrators -p foo -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

test ! -s $work/file
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -list integrators -p foo.1 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

diff $work/file - << EOF
$USER
EOF
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -list integrators -p foo.1.0 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

diff $work/file - <<EOF
$USER
EOF
if test $? -ne 0 ; then cat log; fail; fi

activity="check admins 203"
$bin/aegis -list administrators -p foo -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

#Just the guy who created the project
diff $work/file - << EOF
$USER
EOF
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -list administrators -p foo.1 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

diff $work/file - << EOF
$USER
EOF
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -list administrators -p foo.1.0 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

diff $work/file - << EOF
$USER
EOF
if test $? -ne 0 ; then cat log; fail; fi

activity="check reveiwers 242"
$bin/aegis -list reviewers -p foo -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

test ! -s $work/file
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -list reviewers -p foo.1 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

diff $work/file - << EOF
$USER
EOF
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -list reviewers -p foo.1.0 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

diff $work/file - << EOF
$USER
EOF
if test $? -ne 0 ; then cat log; fail; fi

activity="aepa 267"
cat > $work/file <<EOF
description = "The description string";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
developers_may_create_changes = true;
umask = 022;
default_test_exemption = false;
default_test_regression_exemption = true;
minimum_change_number = 10;
reuse_change_numbers = true;
minimum_branch_number = 1;
skip_unlucky = false;
compress_database = false;
develop_end_action = goto_being_reviewed;
protect_development_directory = true;
EOF
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -pa -dpt -p foo.1 -f $work/file -lib $work/lib 2> $work/log
if test $? -ne 0 ; then cat log; no_result; fi

# Make sure that all attributes under foo.1.0 have changed
activity="test aepa 290"

$bin/aegis -pa -l -p foo.1 -lib $work/lib > $work/file1
if test $? -ne 0 ; then cat log; no_result; fi

diff $work/file $work/file1 > $work/log
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -pa -l -p foo.1.0 -lib $work/lib > $work/file1
if test $? -ne 0 ; then cat log; no_result; fi

diff $work/file $work/file1 > $work/log
if test $? -ne 0 ; then cat log; fail; fi

# Now test the remove functions
activity="aeri 314"
$bin/aegis -ri $USER -dpt -p foo.1 -lib $work/lib
if test $? -ne 0 ; then cat log; fail; fi

activity="check integrators 321"
$bin/aegis -list integrators -p foo.1 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

cp /dev/null ok
if test $? -ne 0 ; then no_result; fi

diff ok $work/file
if test $? -ne 0 ; then cat log; fail; fi

activity="check integrators 330"
$bin/aegis -list integrators -p foo.1.0 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

cp /dev/null ok
if test $? -ne 0 ; then no_result; fi

diff ok $work/file
if test $? -ne 0 ; then fail; fi

activity="aerd 336"
$bin/aegis -rd $USER -dpt -p foo.1 -lib $work/lib
if test $? -ne 0 ; then cat log; fail; fi

activity="check developers 341"
$bin/aegis -list developers -p foo.1 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

cp /dev/null ok
if test $? -ne 0 ; then no_result; fi

diff ok $work/file
if test $? -ne 0 ; then fail; fi

activity="check developers 351"
$bin/aegis -list developers -p foo.1.0 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

cp /dev/null ok
if test $? -ne 0 ; then no_result; fi

diff ok $work/file
if test $? -ne 0 ; then fail; fi

activity="aerrv 362"
$bin/aegis -rrv $USER -dpt -p foo.1 -lib $work/lib
if test $? -ne 0 ; then cat log; fail; fi

activity="check reviewers 364"
$bin/aegis -list reviewers -p foo.1 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

cp /dev/null ok
if test $? -ne 0 ; then no_result; fi

diff ok $work/file
if test $? -ne 0 ; then fail; fi

activity="check reviewers 373"
$bin/aegis -list reviewers -p foo.1.0 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

cp /dev/null ok
if test $? -ne 0 ; then no_result; fi

diff ok $work/file
if test $? -ne 0 ; then fail; fi

# can't test aera, because then there would be done, and that's an error
# activity="aera 312"
# $bin/aegis -ra $USER -dpt -p foo.1 -lib $work/lib
# if test $? -ne 0 ; then cat log; fail; fi
#
# activity="check admins 319"
# $bin/aegis -list administrators -p foo.1 -lib $work/lib -ter > $work/file
# if test $? -ne 0 ; then cat log; no_result; fi
#
# #Just the guy who created the project
# diff $work/file - << EOF
# $USER
# EOF
# if test $? -ne 0 ; then cat log; fail; fi

activity="check admins 330"
$bin/aegis -list administrators -p foo.1.0 -lib $work/lib -ter > $work/file
if test $? -ne 0 ; then cat log; no_result; fi

#Just the guy who created the project
echo $USER > ok
if test $? -ne 0 ; then no_result; fi

diff ok $work/file
if test $? -ne 0 ; then cat log; fail; fi

pass
