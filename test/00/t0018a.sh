#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1993-1998, 2002, 2004-2008 Peter Miller
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

no_result()
{
	set +x
	echo "NO RESULT for test of 'aegis -NewFile -Build' command ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of 'aegis -NewFile -Build' command ($activity)" 1>&2
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
activity="create working directory 94"
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
# If the C++ compiler is called something other than "c++", as
# discovered by the configure script, create a shell script called
# "c++" which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
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
#
activity="new project 132"
$bin/aegis -newpro foo -vers "" -dir $workproj -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# change project attributes
#
activity="project attributes 139"
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
$bin/aegis -proatt -f $tmp -proj foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# create a new change
#	make sure it creates the files it should
#
activity="new change 153"
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
$bin/aegis -new_change 1 -f $tmp -project foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add a new developer
#
activity="new developer 165"
$bin/aegis -newdev $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# begin development of a change
#
activity="begin development 172"
$bin/aegis -devbeg 1 -p foo -dir $workchan -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add a new files to the change
#
activity="new file 179"
$bin/aegis -new_file $workchan/main.cc -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
$bin/aegis -new_file $workchan/fubar -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
$bin/aegis -new_file $workchan/version -b -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -new_file $workchan/aegis.conf -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# put something in 'main.cc'
#
cat > $workchan/main.cc << 'end'
int
main(int argc, char **argv)
{
	return 0;
}
end

#
# put something in 'config'
#
cat > $workchan/aegis.conf << 'end'
build_command = "rm -f foo version; c++ -o foo main.cc; echo '$v' > version";
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

#
# create a new test
#
activity="new test 225"
$bin/aegis -nt -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# put something in 'test/00/t0001a.sh'
#
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
#
# Project: "foo"
# Change: 1
#

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

#
# build the change
#
activity="development build 282"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# difference the change
#
activity="development diff 289"
$bin/aegis -diff -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# test the change
#
activity="development test 296"
$bin/aegis -test -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
activity="develop end 303"
$bin/aegis -dev_end -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# add a new reviewer
#
activity="new reviewer 310"
$bin/aegis -newrev $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the review
#
activity="review pass 317"
$bin/aegis -review_pass -chan 1 -proj foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add an integrator
#
activity="new inegrator 324"
$bin/aegis -newint $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# start integrating
#
activity="integrate begin 331"
$bin/aegis -intbeg 1 -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# integrate build
#
activity="integration build 338"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# integrate test
#
activity="integration test 345"
$bin/aegis -test -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# integration pass
#
activity="integrate pass 352"
$bin/aegis -intpass -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# create a new change
#	make sure it creates the files it should
#
activity="new change 360"
cat > $tmp << 'end'
brief_description = "A second Change";
cause = internal_bug;
end
$bin/aegis -new_change 2 -f $tmp -project foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# begin development of a change
#
activity="develop begin 371"
$bin/aegis -devbeg 2 -p foo -dir $workchan -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# set change attributes
#
activity="change attributes 378"
cat > $workchan/xx  << 'fubar'
test_exempt = true;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -ca -f $workchan/xx -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add a new files to the change
#
activity="copy file 390"
$bin/aegis -cp $workchan/main.cc -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# make it re-compile
#
cat >> $workchan/main.cc << 'end'
/* almost nothing */
end

#
# build the change
#
activity="development build 409"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# difference the change
#
activity="development diff 416"
$bin/aegis -diff -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
activity="develop end 423"
$bin/aegis -dev_end -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the review
#
activity="review pass 430"
$bin/aegis -review_pass -chan 2 -proj foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# start integrating
#
activity="integrate begin 437"
$bin/aegis -intbeg 2 -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# integrate build
#
activity="integration build 444"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# integration pass
#
activity="integrate pass 451"
$bin/aegis -intpass -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# the build file should now be a change file
#	even though user did not put it there
#
activity="check change file state 459"
$bin/aegis -l cf -c 2 -unf -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
cat > test.ok << 'fubar'
source modify 1 -> 2 main.cc
build modify 1 -> 2 version
fubar
if test $? -ne 0 ; then no_result; fi
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass
