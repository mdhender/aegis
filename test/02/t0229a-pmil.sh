#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2006-2008 Peter Miller
#	Copyright (C) 2007 Walter Franzini
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
#	along with this program; if not, see
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

parch=
test "$1" != "" && parch="$1/"
bin="$here/${parch}bin"

if test "$EXEC_SEARCH_PATH" != ""
then
    tpath=
    hold="$IFS"
    IFS=":$IFS"
    for tpath2 in $EXEC_SEARCH_PATH
    do
	tpath=${tpath}${tpath2}/${parch}bin:
    done
    IFS="$hold"
    PATH=${tpath}${PATH}
else
    PATH=${bin}:${PATH}
fi
export PATH

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
	echo "FAILED test of the aed functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aed functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 115"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi
workchan=$work/chan
AEGIS_PROJECT=test
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

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
activity="new project 139"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $work/proj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 146"
cat > tmp << 'end'
description = "A bogus project created to test the "
    "aed functionality.";
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
activity="staff 163"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new branch
#
$bin/aegis -newbranch -p $AEGIS_PROJECT 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
AEGIS_PROJECT=${AEGIS_PROJECT}.1
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 182"
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
activity="develop begin 194"
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 201"
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
# finish change
#
activity="finish 233"
$bin/aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 240"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish the change
#
activity="finish the change 247"
$bin/aefinish -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# create a new change
#
activity="new change 254"
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
activity="develop begin 266"
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="rename 270"
$bin/aegis -mv -c 2 $workchan/bogus1 $workchan/bogus2 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat >> $workchan/bogus2 <<EOF
append some text
append some text
EOF
if test $? -ne 0; then no_result; fi

$bin/aefinish -c 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

$bin/aegis -ib -c 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

$bin/aefinish -p $AEGIS_PROJECT -c 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi
#
# create a new change
#
activity="new change 291"
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
activity="develop begin 303"
$bin/aegis -db 3 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify moved file 307"
$bin/aegis -cp -c 3 $workchan/bogus2 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat >> $workchan/bogus2 <<EOF
he he he
EOF
if test $? -ne 0; then no_result; fi

$bin/aefinish -c 3 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate begin 319"
$bin/aegis -ib -c 3 -v > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="integrate 323"
$bin/aefinish -p $AEGIS_PROJECT -c 3 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
