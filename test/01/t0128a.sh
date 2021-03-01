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
	echo "FAILED test of the aemvu functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aemvu functionality ($activity)" 1>&2
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

activity="working directory 99"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi
tmp="$work/temp-file"

worklib=$work/lib
workchan=$work/change-dir

AEGIS_PATH=$worklib
export AEGIS_PATH
PATH=$bin:$PATH
export PATH
AEGIS_PROJECT=example
export AEGIS_PROJECT

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the aemvu functionality
#
activity="new project 129"
$bin/aegis -npr example -version '' -lib $worklib -dir $work/proj -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 136"
cat > $tmp << 'TheEnd'
description = "bogosity";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -proatt -f $tmp -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 152"
cat > $tmp << 'TheEnd'
brief_description = "c1";
description = "c1";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

AEGIS_PROJECT=example
export AEGIS_PROKECT

$bin/aegis -nc 1 -f $tmp -project example -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new developer
#
activity="new developer 169"
$bin/aegis -newdev $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="new reviewer 172"
$bin/aegis -newrev $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="new integrator 175"
$bin/aegis -newint $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of the change
#
activity="develop begin 182"
$bin/aegis -devbeg 1 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the new files to the change
#
activity="new file 189"
$bin/aegis -new_file $workchan/Howto.cook $workchan/aegis.conf $workchan/f1 \
	$workchan/f2 $workchan/f3 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

date > $workchan/f1
if test $? -ne 0 ; then no_result; fi
date > $workchan/f2
if test $? -ne 0 ; then no_result; fi
date > $workchan/f3
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'TheEnd'
build_command = "exit 0";
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";

merge_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
TheEnd
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 224"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 231"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 238"
$bin/aegis -dev_end -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 245"
$bin/aegis -review_pass -chan 1 -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 252"
$bin/aegis -intbeg 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build and test
#
activity="build 259"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 266"
$bin/aegis -intpass -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create the second change
#
activity="new change 273"
cat > $tmp << 'fubar'
brief_description = "c2";
description = "c2";
cause = internal_bug;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f $tmp -project example -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of the change
#
activity="develop begin 286"
$bin/aegis -devbeg 2 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# move files with the change
#
activity="move file 293"
$bin/aegis -mv $workchan/f2 $workchan/bogus -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# move file undo
#
activity="move file undo 300"
$bin/aegis -mvu $workchan/bogus -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# make sure the file state looks like we expect
#
cat > ok << fubar
src =
[
];
fubar
if test $? -ne 0 ; then cat log; no_result; fi
check_it ok $work/proj/info/change/0/002.fs

#
# the things tested in this test, worked
# the things not tested in this test, may or may not work
#
pass
