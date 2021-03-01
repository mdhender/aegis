#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991-1998, 2000, 2002-2008 Peter Miller
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

no_result()
{
	set +x
	echo "NO RESULT for test of -Review_Pass_Undo functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of -Review_Pass_Undo functionality ($activity)" 1>&2
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
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

#
# some variable to make things earier to read
#
worklib=$work/lib
workproj=$work/foo.proj
workproj2=$work/foo.proj2
workchan=$work/foo.chan
tmp=$work/tmp

#
# make the directories
#
activity="working directory 110"
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
# If the C++ compiler is called something other than "c++", as discovered
# by the configure script, create a shell script called "c++" which
# invokes the correct C++ compiler.  Make sure the current directory is in
# the path, so that it will be invoked.
#
if test "$CXX" != "c++"
then
	cat >> c++ << fubar
#!/bin/sh
exec ${CXX-g++} \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx c++
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# make a new project
#	and check files it should have made
#
activity="new project 149"
$bin/aegis -newpro foo -version "" -dir $workproj -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -proatt -f $tmp -proj foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 169"
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_change 1 -f $tmp -project foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new developer
#
activity="new developer 182"
$bin/aegis -newdev $USER -p foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 189"
$bin/aegis -devbeg 1 -p foo -dir $workchan -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 196"
$bin/aegis -new_file $workchan/main.cc -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -new_file $workchan/aegis.conf -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.cc << 'end'
int
main(int argc, char **argv)
{
	return 0;
}
end
if test $? -ne 0 ; then no_result; fi
cat > $workchan/aegis.conf << 'end'
build_command = "rm -f foo; c++ -o foo -D'VERSION=\"$v\"' main.cc";
link_integration_directory = true;

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
# create a new test
#
activity="new test 232"
$bin/aegis -nt -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
no_result()
{
	echo WHIMPER 1>&2
	exit 2
}
fail()
{
	echo SHUZBUTT 1>&2
	exit 1
}
pass()
{
	exit 0
}
trap "no_result" 1 2 3 15

./foo
q=$?

# check for signals
if test $q -ge 128
then
	fail
fi

# should not complain
if test $q -ne 0
then
	fail
fi

# it probably worked
pass
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 276"
$bin/aegis -build -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 283"
$bin/aegis -diff -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 290"
$bin/aegis -test -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 297"
$bin/aegis -dev_end -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new reviewer
#
activity="new reviewer 304"
$bin/aegis -newrev $USER -p foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 311"
$bin/aegis -review_pass -chan 1 -proj foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# undo the review pass
#
activity="review pass undo 318"
$bin/aegis -review_pass_undo -chan 1 -proj foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# make sure it worked
#
activity="verify change state 325"
cat > ok << 'fubar'
brief_description = "This change is used to test the aegis functionality with respect to change descriptions.";
description = "This change is used to test the aegis functionality with respect to change descriptions.";
cause = internal_enhancement;
test_exempt = false;
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
given_regression_test_exemption = true;
build_time = TIME;
test_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
		test_time = TIME;
	},
];
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
		what = develop_end;
		who = "USER";
	},
	{
		when = TIME;
		what = review_pass;
		who = "USER";
	},
	{
		when = TIME;
		what = review_pass_undo;
		who = "USER";
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $workproj/info/change/0/001

#
# the things tested in this test, worked
#
pass
