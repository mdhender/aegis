#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2002, 2004-2008 Peter Miller
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
	echo 'FAILED test of the progress messages functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the progress messages functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap "\"no_result\"" 1 2 3 15

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
	cat >> $work/c++ << fubar
#!/bin/sh
exec ${CXX-g++} \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx $work/c++
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp

#
# make sure progress messages are printed if -PROGress option is specified
# and not printed if -Not-PROGress is specified.
#

#
# make a new project
#
activity="new project 137"
$bin/aegis -newpro foo -version "" -dir $workproj -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 144"
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
activity="new change 158"
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
activity="new change 171"
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
activity="new developer 183"
$bin/aegis -newdev $USER -p foo -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 190"
$bin/aegis -devbeg 10 -p foo -dir $workchan -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 197"
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
batch_test_command = "echo \"-- ${current} of ${total}\"; \
echo \"test_result = [\" >> ${output}; \
for f in ${file_names}; do echo \"{file_name = \\\"$$f\\\"; exit_status=0;},\">> ${output}; done; \
echo \"];\" >> ${output};";
end
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 246"
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
# create 199 more tests (batches is split by 100 tests)
#
activity="new test 283"

i=1
while test $i -le 199; do
	$bin/aegis -nt -v -lib $worklib -p foo > log 2>&1
	if test $? -ne 0 ; then cat log; no_result; fi

	cp $workchan/test/00/t0001a.sh $workchan/`cat log | grep 'new test' | sed -e's/^.*new test \(.*\) complete$/\1/'`
	if test $? -ne 0 ; then no_result; fi
	i=`expr $i + 1`
done


#
# build the change
#
activity="build 299"
$bin/aegis -build -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 306"
$bin/aegis -diff -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change with progress messages
#
activity="test 313"
$bin/aegis -test -prog -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# make sure progress messages is in the log
#
grep 'batch test' log > $tmp.log
cat > $tmp.log.ideal << 'end'
aegis: project "foo": change 10: batch test from 1 to 100 of 200
aegis: project "foo": change 10: batch test from 101 to 200 of 200
end
diff $tmp.log $tmp.log.ideal > log 2>&1
if test $? -ne 0; then cat log; fail; fi

#
# build the change again to reset test results
#
activity="build 331"
$bin/aegis -build -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change without progress messages
#
activity="test 338"
$bin/aegis -test -trace libaegis/change/test/batch_fake.c -nprog -nl -v -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# make sure progress messages is not in the log
#
cp log $tmp.log
if grep 'batch test' $tmp.log > log 2>&1; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
