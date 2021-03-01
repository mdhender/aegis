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
	echo "NO RESULT for test of the -Integrate_Begin_Undo functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of the -Integrate_Begin_Undo functionality ($activity)" 1>&2
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

activity="working directory 82"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

#
# some variable to make things earier to read
#
worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp

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
#	and check files it should have made
#
activity="new project 129"
$bin/aegis -newpro foo -version "" -dir $workproj -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# change project attributes
#
activity="project attributes 136"
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
$bin/aegis -proatt -f $tmp -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# create a new change
#	make sure it creates the files it should
#
activity="new change 150"
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
$bin/aegis -new_change 1 -f $tmp -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add a new developer
#
activity="new developer 162"
$bin/aegis -newdev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# begin development of a change
#	check it made the files it should
#
activity="develop begin 170"
$bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add a new files to the change
#
activity="new file 177"
$bin/aegis -new_file $workchan/main.cc -nl -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_file $workchan/aegis.conf -nl -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
cat > $workchan/main.cc << 'end'
int
main(int argc, char **argv)
{
	return 0;
}
end
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

merge_command = "(diff3 -e $i $orig $mr | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $i > $out";
end
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 212"
$bin/aegis -nt -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
#
# Project: "foo"
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
activity="build 257"
$bin/aegis -build -nl -lib $worklib -p foo > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi

#
# difference the change
#
activity="diff 264"
$bin/aegis -diff -nl -lib $worklib -p foo > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi

#
# test the change
#
activity="test 271"
$bin/aegis -test -nl -lib $worklib -p foo > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi

#
# finish development of the change
#
activity="develop end 278"
$bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi

#
# add a new reviewer
#
activity="new reviewer 285"
$bin/aegis -newrev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# pass the review
#
activity="review pass 292"
$bin/aegis -review_pass -chan 1 -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add an integrator
#
activity="new integrator 299"
$bin/aegis -newint $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# start integrating
#
activity="integrate begin 306"
$bin/aegis -ib 1 -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# integrate begin undo
#
activity="integrate begin undo 313"
$bin/aegis -ibu -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass
