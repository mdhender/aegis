#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991-1998, 2004-2008 Peter Miller
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
# Files which are removed by a change should not be checked to see if
# their last-modified-time is valid at develop end time.  This test
# verifies that aed and aede behave correctly in this case.
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
	echo "NO RESULT for test of 'aegis -DevEnd' command ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of 'aegis -DevEnd' command ($activity)" 1>&2
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
worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp

#
# make the directories
#
activity="working directory 98"
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
activity="new project 136"
$bin/aegis -newpro foo -version "" -dir $workproj -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 143"
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
$bin/aegis -proatt -f $tmp -proj foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#	make sure it creates the files it should
#
activity="new change 157"
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
$bin/aegis -new_change -f $tmp -project foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a second change
#	make sure it creates the files it should
#
activity="new change 170"
cat > $tmp << 'end'
brief_description = "This change was added to make the various listings \
much more interesting.";
cause = internal_bug;
end
$bin/aegis -new_change -f $tmp -project foo -v -lib $worklib > log 2>&1
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
$bin/aegis -devbeg 10 -p foo -dir $workchan -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 196"
$bin/aegis -new_file $workchan/main.cc -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -new_file $workchan/fubar -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -new_file $workchan/aegis.conf -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

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
build_command = "rm -f foo; c++ -o foo -D'VERSION=\"$vers\"' main.cc";
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
activity="new test 241"
$bin/aegis -nt -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# put something in 'test/00/t0001a.sh'
#
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

# should not complain
./foo
if test $? -ne 0; then fail; fi

# it probably worked
pass
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 278"
$bin/aegis -build -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 285"
$bin/aegis -diff -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 292"
$bin/aegis -test -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 299"
$bin/aegis -dev_end -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new reviewer
#
activity="new reviewer 306"
$bin/aegis -newrev $USER -p foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 313"
$bin/aegis -review_pass -chan 10 -proj foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add an integrator
#
activity="new integrator 320"
$bin/aegis -newint $USER -p foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 327"
$bin/aegis -intbeg 10 -p foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 334"
$bin/aegis -build -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate test
#
activity="test 341"
$bin/aegis -test -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integration pass
#
activity="integrate pass 348"
$bin/aegis -intpass -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start work on change 2
#
activity="develop begin 355"
$bin/aegis -devbeg 11 -p foo -v -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# copy a file into the change
#
activity="copy file 362"
$bin/aegis -cp $workchan/main.cc -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# remove a file with the change
#
activity="remove file 369"
$bin/aegis -rm $workchan/fubar -nl -p foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change the file
#
cat > $workchan/main.cc << 'end'
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	if (argc != 1)
	{
		fprintf(stderr, "usage: %s\n", argv[0]);
		exit(1);
	}
	printf("hello, world\n");
	return 0;
}
end
if test $? -ne 0 ; then no_result; fi

#
# need another test
#
activity="new test 397"
$bin/aegis -nt -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/test/00/t0002a.sh << 'end'
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

./foo > /dev/null 2>&1
test $? -eq 0 || fail

# should have complained
./foo ickky
if test $? -ne 1; then fail; fi

# it probably worked
pass
end
if test $? -ne 0 ; then no_result; fi

#
# diff the change
#
# We are testing aede for correctness here.
#
activity="diff 435"
$bin/aegis -diff -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# build the change
#
activity="build 442"
$bin/aegis -build -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 449"
$bin/aegis -test -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -test -bl -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# end development of the change
#
# We are testing aede for correctness here.
#
activity="develop end 460"
$bin/aegis -devend -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

# should be no automatic logging
if test "`find $work -name 'aegis.log' -print`" != "" ; then no_result; fi

#
# the things tested in this test, worked
#
pass
