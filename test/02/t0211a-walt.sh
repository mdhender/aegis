#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004, 2005 Peter Miller;
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
# MANIFEST: Test the aed functionality
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
# test the aed functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 134"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 141"
cat > tmp << 'end'
description = "A bogus project created to test the aemv/merge "
    "functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 157"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# create a new change
#
activity="new change 170"
cat > tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 188"
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
build_command = "exit 0";
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
merge_command = "exit 1 # $input $output $orig $most_recent";
patch_diff_command = "set +e; diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 232"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 239"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 246"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 253"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 260"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 267"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 274"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 281"
cat > tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 293"
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Use the second change to move bogus2 to bogus3
#
activity="move file 300"
$bin/aegis -c 2 -mv -baserel bogus2 bogus3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# Create a third change to modify bogus2
#
activity="new change 309"
cat > tmp << 'end'
brief_description = "The third change.";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 318"
$bin/aegis -db 3 -dir ${workchan}3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 322"
$bin/aegis -cp 3 -baserel bogus2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > ${workchan}3/bogus2 << 'end'
line one
line two
line two-and-a-half
line three
end
if test $? -ne 0 ; then no_result; fi

activity="build 334"
$bin/aegis -build 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 338"
$bin/aegis -diff 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 342"
$bin/aegis -de 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 346"
$bin/aegis -rpass -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 350"
$bin/aegis -ib 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 354"
$bin/aegis -b -nl -v 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 358"
$bin/aegis -intpass -nl 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

ulimit -c unlimited

#
# Resume change 2, make sure that aed --merge works.
#
activity="aed --merge 367"
$bin/aegis --diff --merge-only > log 2>&1
if test $? -ne 1 ; then cat log; fail; fi

test -f $workchan/bogus3 || fail

pass
