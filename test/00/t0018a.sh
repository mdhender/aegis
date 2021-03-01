#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998 Peter Miller;
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
# MANIFEST: Test the 'aegis -NewFile -Build' command.
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
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp

#
# make the directories
#
activity="create working directory 79"
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
# If the C compiler is called something other than ``cc'', as discovered
# by the configure script, create a shell script called ``cc'' which
# invokes the correct C compiler.  Make sure the current directory is in
# the path, so that it will be invoked.
#
if test "$CC" != "" -a "$CC" != "cc"
then
	cat >> cc << fubar
#!/bin/sh
exec $CC \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx cc
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# make a new project
#
activity="new project 112"
$bin/aegis -newpro foo -vers "" -dir $workproj -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# change project attributes
#
activity="project attributes 119"
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
activity="new change 133"
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
activity="new developer 145"
$bin/aegis -newdev $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# begin development of a change
#
activity="begin development 152"
$bin/aegis -devbeg 1 -p foo -dir $workchan -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add a new files to the change
#
activity="new file 159"
$bin/aegis -new_file $workchan/main.c -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
$bin/aegis -new_file $workchan/fubar -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
$bin/aegis -new_file $workchan/version -b -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -new_file $workchan/config -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# put something in 'main.c'
#
cat > $workchan/main.c << 'end'
void
main()
{
	exit(0);
}
end

#
# put something in 'config'
#
cat > $workchan/config << 'end'
build_command = "rm -f foo version; cc -o foo main.c; echo '$v' > version";
link_integration_directory = true;

history_get_command =
	"co -u'$e' -p $h,v > $o";
history_create_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_put_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_query_command =
	"rlog -r $h,v | awk '/^head:/ {print $$2}'";

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";

diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
end

#
# create a new test
#
activity="new test 205"
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
activity="development build 252"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# difference the change
#
activity="development diff 259"
$bin/aegis -diff -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# test the change
#
activity="development test 266"
$bin/aegis -test -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
activity="develop end 273"
$bin/aegis -dev_end -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# add a new reviewer
#
activity="new reviewer 280"
$bin/aegis -newrev $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the review
#
activity="review pass 287"
$bin/aegis -review_pass -chan 1 -proj foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add an integrator
#
activity="new inegrator 294"
$bin/aegis -newint $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# start integrating
#
activity="integrate begin 301"
$bin/aegis -intbeg 1 -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# integrate build
#
activity="integration build 308"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# integrate test
#
activity="integration test 315"
$bin/aegis -test -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# integration pass
#
activity="integrate pass 322"
$bin/aegis -intpass -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# create a new change
#	make sure it creates the files it should
#
activity="new change 330"
cat > $tmp << 'end'
brief_description = "A second Change";
cause = internal_bug;
end
$bin/aegis -new_change 2 -f $tmp -project foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# begin development of a change
#
activity="develop begin 341"
$bin/aegis -devbeg 2 -p foo -dir $workchan -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# set change attributes
#
activity="change attributes 348"
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
activity="copy file 360"
$bin/aegis -cp $workchan/main.c -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# make it re-compile
#
cat >> $workchan/main.c << 'end'
/* almost nothing */
end

#
# build the change
#
activity="development build 374"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# difference the change
#
activity="development diff 381"
$bin/aegis -diff -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
activity="develop end 388"
$bin/aegis -dev_end -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the review
#
activity="review pass 395"
$bin/aegis -review_pass -chan 2 -proj foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# start integrating
#
activity="integrate begin 402"
$bin/aegis -intbeg 2 -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# integrate build
#
activity="integration build 409"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# integration pass
#
activity="integrate pass 416"
$bin/aegis -intpass -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# the build file should now be a change file
#	even though user did not put it there
#
activity="check change file state 424"
$bin/aegis -l cf -c 2 -unf -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
cat > test.ok << 'fubar'
source modify 1.1 -> 1.2 main.c
build modify 1.1 -> 1.2 version
fubar
if test $? -ne 0 ; then no_result; fi
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass
