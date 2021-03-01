#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2001-2003, 2005-2008 Peter Miller
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
	echo NO RESULT for test of aerb functionality "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo FAILED test of aerb functionality "($activity)" 1>&2
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
activity="new project 130"
$bin/aegis -newpro foo -version "" -dir $workproj -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# change project attributes
#
activity="project attributes 137"
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
minimum_change_number = 1;
develop_end_action = goto_awaiting_review;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -proatt -f $tmp -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# create a new change
#
activity="new change 153"
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newcha -list -pro foo -lib $worklib > test.out
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_change -f $tmp -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add a new developer
#
activity="new developer 169"
$bin/aegis -newdev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# begin development of a change
#
activity="develop begin 176"
$bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add a new files to the change
#
activity="new file 183"
$bin/aegis -new_file $workchan/main.c -nl -lib $worklib -Pro foo
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_file $workchan/aegis.conf -nl -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi

cat > $workchan/main.c << 'end'
int
main()
{
	exit(0);
	return 0;
}
end
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

merge_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 221"
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# difference the change
#
activity="diff 228"
$bin/aegis -diff -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
activity="develop end 235"
$bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# check the state file
#
cat > ok << 'end'
brief_description = "This change is used to test the aegis functionality with respect to change descriptions.";
description = "This change is used to test the aegis functionality with respect to change descriptions.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
copyright_years =
[
	YYYY,
];
state = awaiting_review;
given_test_exemption = true;
given_regression_test_exemption = true;
development_directory = ".../foo.chan";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "USER";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "USER";
	},
	{
		when = TIME;
		what = develop_end_2ar;
		who = "USER";
	},
];
end
if test $? -ne 0 ; then fail; fi
check_it $workproj/info/change/0/001 ok

#
# add a new reviewer
#
activity="new reviewer 295"
$bin/aegis -newrev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# begin the review
#
activity="review begin 302"
$bin/aegis -review_begin -chan 1 -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# check the state file
#
cat > ok << 'end'
brief_description = "This change is used to test the aegis functionality with respect to change descriptions.";
description = "This change is used to test the aegis functionality with respect to change descriptions.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
copyright_years =
[
	YYYY,
];
state = being_reviewed;
given_test_exemption = true;
given_regression_test_exemption = true;
development_directory = ".../foo.chan";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "USER";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "USER";
	},
	{
		when = TIME;
		what = develop_end_2ar;
		who = "USER";
	},
	{
		when = TIME;
		what = review_begin;
		who = "USER";
	},
];
end
if test $? -ne 0 ; then fail; fi
check_it $workproj/info/change/0/001 ok

#
# the things tested in this test, worked
#
pass
