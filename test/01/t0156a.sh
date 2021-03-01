#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2001, 2005-2008 Peter Miller
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

here=`pwd`
if test $? -ne 0; then exit 2; fi

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

activity="create working directory 41"

no_result()
{
	set +x
	echo NO RESULT for test of aed functionality "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo FAILED test of aed functionality "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	echo PASSED 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
}
trap "no_result" 1 2 3 15

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/delta[0-9][0-9]*/delta/' \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $1 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $2 $work/sed.out
	if test $? -ne 0; then fail; fi
}

#
# some variable to make things earier to read
#
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

worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp

#
# make the directories
#
mkdir $work $work/lib
if test $? -ne 0 ; then exit 2; fi
chmod 777 $work/lib
if test $? -ne 0 ; then exit 2; fi
cd $work
if test $? -ne 0 ; then exit 2; fi

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
activity="new project 133"
$bin/aegis -npr foo -version "" -dir $workproj -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# change project attributes
#
activity="project attributes 140"
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
minimum_change_number = 1;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -proatt -f $tmp -proj foo
if test $? -ne 0 ; then no_result; fi

#
# add staff
#
activity="staff 156"
$bin/aegis -newdev $USER -p foo
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newrev $USER -p foo
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newint $USER -p foo
if test $? -ne 0 ; then no_result; fi

#
# Now create the branches
#
activity="new branch 167"
$bin/aegis -nbr -p foo 3
if test $? -ne 0 ; then no_result; fi
activity="new branch 170"
$bin/aegis -nbr -p foo.3 29
if test $? -ne 0 ; then no_result; fi

AEGIS_PROJECT=foo.3.29
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 180"
cat > $tmp << 'end'
brief_description = "create a file";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_change 1 -f $tmp -project foo.3.29
if test $? -ne 0 ; then no_result; fi

#
# create a second change
#
cat > $tmp << 'end'
brief_description = "move a file";
cause = internal_bug;
end
$bin/aegis -new_change 2 -f $tmp -project foo.3.29
if test $? -ne 0 ; then no_result; fi

#
# begin development of a change
#	check it made the files it should
#
activity="develop begin 203"
$bin/aegis -devbeg 1 -dir $workchan
if test $? -ne 0 ; then no_result; fi

#
# add a new files to the change
#
activity="new file 210"
$bin/aegis -new_file $workchan/move_me $workchan/aegis.conf -nl
if test $? -ne 0 ; then no_result; fi

echo hello > $workchan/move_me
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
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
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 239"
$bin/aegis -build -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out;no_result; fi

#
# difference the change
#
activity="diff 246"
$bin/aegis -diff > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
activity="develop end 253"
$bin/aegis -dev_end
if test $? -ne 0 ; then no_result; fi

#
# pass the review
#
activity="review pass 260"
$bin/aegis -review_pass -chan 1
if test $? -ne 0 ; then no_result; fi

#
# start integrating
#
activity="integrate begin 267"
$bin/aegis -intbeg 1 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# integrate build
#
activity="integration build 274"
$bin/aegis -build -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integration diff 277"
$bin/aegis -diff -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the integration
#
activity="integrate pass 284"
$bin/aegis -intpass -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# start work on change 2
#
activity="develop begin 291"
$bin/aegis -devbeg 2 -dir $workchan
if test $? -ne 0 ; then no_result; fi

#
# move a file
#
activity="move file 298"
$bin/aegis -mv $workchan/move_me $workchan/remove_me -nl
if test $? -ne 0 ; then no_result; fi

#
# build the change
# diff the change
#
activity="build 306"
$bin/aegis -b -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="diff 309"
$bin/aegis -diff -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# end development
# review pass
# start integrating
# build the integration
# pass the integration
#
activity="devlop end 320"
$bin/aegis -devend
if test $? -ne 0 ; then no_result; fi
activity="review pass 323"
$bin/aegis -revpass -c 2
if test $? -ne 0 ; then no_result; fi
activity="integrate begin 326"
$bin/aegis -intbeg -c 2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate build 329"
$bin/aegis -b -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate diff 332"
$bin/aegis -diff -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate pass 335"
$bin/aegis -intpass -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

AEGIS_PROJECT=foo.3
export AEGIS_PROJECT

#
# end the first branch
#
activity="devlop end 345"
$bin/aegis -devend 29

if test $? -ne 0 ; then no_result; fi
activity="review pass 349"
$bin/aegis -revpass -c 29
if test $? -ne 0 ; then no_result; fi
activity="integrate begin 352"
$bin/aegis -intbeg -c 29 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate build 355"
$bin/aegis -b -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate pass 358"
activity="integrate diff 359"
$bin/aegis -diff -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate pass 362"
$bin/aegis -intpass -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

AEGIS_PROJECT=foo
export AEGIS_PROJECT

#
# end the second (grandparent) branch
#
activity="devlop end 372"
$bin/aegis -devend 3

if test $? -ne 0 ; then no_result; fi
activity="review pass 376"
$bin/aegis -revpass -c 3
if test $? -ne 0 ; then no_result; fi
activity="integrate begin 379"
$bin/aegis -intbeg -c 3 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate build 382"
$bin/aegis -b -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate diff 385"
$bin/aegis -diff -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="integrate pass 388"
$bin/aegis -intpass -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# the things tested in this test, worked
#
pass
