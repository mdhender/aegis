#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2007, 2008 Peter Miller
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
	echo "FAILED test of the aeclone vs aerm functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aeclone vs aerm functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="working directory 113"
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
# test the aeclone vs aerm functionality
#
activity="new project 143"
$bin/aegis -npr example -version - -lib $worklib -dir $work/proj -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 150"
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
# add the staff
#
activity="new developer 166"
$bin/aegis -newdev $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="new reviewer 169"
$bin/aegis -newrev $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="new integrator 172"
$bin/aegis -newint $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------
#
# Create a new change to get the project going.
#
activity="new change 180"
cat > $tmp << 'TheEnd'
brief_description = "c1";
description = "c1";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 1 -f $tmp -project example -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_CHANGE=1
export AEGIS_CHANGE

#
# begin development of the change
#
activity="develop begin 197"
$bin/aegis -devbeg 1 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a suitable aegis.conf file
#
activity="new files 204"
$bin/aegis -nf $workchan/aegis.conf $workchan/template.sh -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/aegis.conf << 'fubar'
build_command = "exit 0";
link_integration_directory = true;
create_symlinks_before_build = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
merge_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
echo '1,$$p' ) | ed - $mr > $out";
history_put_trashes_file = warn;
fubar
if test $? -ne 0 ; then no_result; fi

activity="new files 227"
$bin/aegis -nf $workchan/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo fred > $workchan/fred
if test $? -ne 0 ; then no_result; fi

activity="diff 234"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 238"
$bin/aegis -build -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 242"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 246"
$bin/aegis -rpass 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 250"
$bin/aegis -ib 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 254"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 258"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 262"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------
#
# With a new change, remove a file.
#
AEGIS_CHANGE=10
export AEGIS_CHANGE

activity="new change 273"
cat > $tmp << 'TheEnd'
brief_description = "c2";
description = "c2";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 10 -f $tmp -project example -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 284"
$bin/aegis -devbeg 10 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="remove files 288"
$bin/aegis -rm $workchan/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		uuid = "UUID";
		action = remove;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/proj/info/change/0/010.fs

# --------------------------------------------------------------------------
#
# Clone the change with the file remove.
#
activity="clone 315"
$bin/aegis -clone 10 11 -dir ${workchan}11 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > ok << 'fubar'
src =
[
	{
		file_name = "fred";
		uuid = "UUID";
		action = remove;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/proj/info/change/0/011.fs

AEGIS_CHANGE=11
export AEGIS_CHANGE

activity="build 341"
$bin/aegis -build -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 345"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 349"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 353"
$bin/aegis -rpass 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 357"
$bin/aegis -ib 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 361"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 365"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 369"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------
#
# Now the file is gone, do an aecp -delta
#
AEGIS_CHANGE=12
export AEGIS_CHANGE

activity="new change 380"
cat > $tmp << 'TheEnd'
brief_description = "c12";
description = "c12";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 12 -f $tmp -project example -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 391"
$bin/aegis -devbeg 12 -dir ${workchan}12 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

set +e

#
# All that set-up was for this:
# [ aegis-Bugs-1684820 ] would segfault
#
activity="aecp -delta 401"
$bin/aegis -cp . -delta 2 -v -c 12 -base-rel > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

# --------------------------------------------------------------------------

#
# the things tested in this test, worked
# the things not tested in this test, may or may not work
#
pass
