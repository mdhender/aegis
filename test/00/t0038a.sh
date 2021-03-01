#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996-1998, 2001, 2002, 2004-2008 Peter Miller
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
	echo FAILED test of aeipass mod time functionality "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo NO RESULT for test of aeipass mod time functionality "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap "no_result" 1 2 3 15

#
# some variable to make things earier to read
#
PAGER=cat
export PAGER

# GNU sort (at least textutils 2.0) is locale sensitive
# ('a.o' and 'all' are swapped) with some LC_COLLATE values.
LC_ALL=C
export LC_ALL

AEGIS_FLAGS="delete_file_preference = no_keep; \
	lock_wait_preference = always; \
	diff_preference = automatic_merge; \
	pager_preference = never; \
	persevere_preference = all; \
	log_file_preference = never;"
export AEGIS_FLAGS
AEGIS_THROTTLE=1
export AEGIS_THROTTLE

worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp

#
# make the directories
#
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

#
# make a new project
#
activity="new project 138"
$bin/aegis -newpro foo -version "" -dir $workproj -lib $worklib
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newpro -list -unf -lib $worklib > test.out
if test $? -ne 0 ; then no_result; fi

#
# change project attributes
#
activity="project attributes 147"
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
minimum_change_number = 1;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -proatt -f $tmp -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# create a new change
#
activity="new change 162"
cat > $tmp << 'end'
brief_description = "first";
cause = internal_bug;
end
$bin/aegis -nc -f $tmp -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi
cat > $tmp << 'end'
brief_description = "second";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = false;
end
$bin/aegis -nc -f $tmp -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi
cat > $tmp << 'end'
brief_description = "third";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = false;
end
$bin/aegis -nc -f $tmp -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add the staff
#
activity="staff 191"
$bin/aegis -newdev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newrev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newint $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# begin development of a change
#
activity="develop begin 202"
$bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add a new files to the change
#
activity="new file 209"
$bin/aegis -nf $workchan/aegis.conf $workchan/a.cc $workchan/b.cc $workchan/h.h \
	$workchan/Makefile -nl -lib $worklib -Pro foo
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'fubar'
build_command = "make";
create_symlinks_before_build = true;
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
fubar
if test $? -ne 0 ; then no_result; fi

cat > $workchan/Makefile << 'fubar'
all: a.o b.o
	rm -f all
	c++ -o all a.o b.o

a.o: a.cc h.h
	rm -f a.o
	c++ -c a.cc

b.o: b.cc h.h
	rm -f b.o
	c++ -c b.cc
fubar
if test $? -ne 0 ; then no_result; fi

cat > $workchan/a.cc << 'fubar'
#include <string.h>
#include <stdio.h>
#include "h.h"
int
main(int argc, char **argv)
{
	if (0 != strcmp(b(), MESSAGE))
	{
		printf("b() -> \"%s\"\n", b());
		printf("MESSAGE -> \"%s\"\n", MESSAGE);
		return 1;
	}
	return 0;
}
fubar
if test $? -ne 0 ; then no_result; fi

cat > $workchan/b.cc << 'fubar'
#include "h.h"
char *b(void) { return MESSAGE; }
fubar
if test $? -ne 0 ; then no_result; fi

cat > $workchan/h.h << 'fubar'
#define MESSAGE "First"
char *b(void);
fubar
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 283"
$bin/aegis -nt -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh

fail()
{
	echo SHUZBUTT 1>&2
	exit 1
}
trap "fail" 1 2 3 15

./all
if test $? -ne 0; then fail; fi
exit 0
end

#
# build the change
#
activity="build 304"
$bin/aegis -b 1 -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out;no_result; fi

#
# difference the change
#
activity="diff 311"
$bin/aegis -diff -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# test the change
#
activity="test 318"
$bin/aegis -test -lib $worklib -p foo -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
aectivity="develop end 444"
$bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi

#
# pass the review
#
activity="review pass 332"
$bin/aegis -review_pass -c 1 -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# start integrating
#
activity="integrate begin 339"
$bin/aegis -intbeg 1 -p foo -lib $worklib > test.err 2>&1
if test $? -ne 0 ; then cat test.err; no_result; fi

#
# integrate build
#
activity="build 346"
$bin/aegis -b 1 -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="test 349"
$bin/aegis -test -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the integration
#
activity="integrate pass 356"
$bin/aegis -intpass -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# start work on change 2
#
activity="develop begin 363"
$bin/aegis -devbeg 2 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# copy a file into the change
#
activity="copy file 370"
$bin/aegis -cp $workchan/h.h -nl -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi

#
# change the file
#
cat > $workchan/h.h << 'fubar'
#define MESSAGE "Second"
#if __STDC__ >= 1
char *b(void);
#else
char *b();
#endif
fubar
if test $? -ne 0 ; then no_result; fi

#
# build the change
# diff the change
# test the change
#
activity="build 393"
$bin/aegis -b 2 -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="diff 396"
$bin/aegis -diff -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="test 399"
$bin/aegis -test -reg -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# end development
# review pass
# start integrating
#
activity="devlop end 408"
$bin/aegis -devend -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
activity="review pass 411"
$bin/aegis -revpass -c 2 -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi
activity="integrate begin 414"
$bin/aegis -intbeg -c 2 -p foo -lib $worklib > test.err 2>&1
if test $? -ne 0 ; then cat test.err; no_result; fi

#
# start work on change 3
#
activity="develop begin 421"
$bin/aegis -devbeg 3 -p foo -dir $workchan.3 -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# copy a file into change 3
#
activity="copy file 428"
$bin/aegis -cp 3 $workchan.3/b.cc -nl -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
cat >> $workchan.3/b.cc << 'fubar'
/* nothing */
fubar
if test $? -ne 0 ; then no_result; fi

#
# build the integration, change 2
# test the integration
#
activity="build 441"
$bin/aegis -b 2 -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="test 444"
$bin/aegis -t 2 -reg -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# build the change, change 3
#
activity="build 451"
$bin/aegis -b 3 -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the integration, change 2
#
activity="integrate pass 458"
$bin/aegis -intpass 2 -nl -lib $worklib -p foo -v > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# Find files in the baseline newer than "all" in change 3's
# development directory.  If aeipass has not touched the files
# correctly, there will be none.
#
activity="check baseline mtimes 467"
find $workproj/baseline -type f -newer $workchan.3/all -print > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
sed -e "s|$workproj/baseline/||" < test.out > test.out2 2>&1
if test $? -ne 0 ; then cat test.out2; no_result; fi
sed -e "s|.exe||" < test.out2 > test.out3 2>&1
if test $? -ne 0 ; then cat test.out3; no_result; fi
sort < test.out3 > test.out4 2>&1
if test $? -ne 0 ; then cat test.out4; no_result; fi
cat > ok << 'fubar'
a.o
all
b.o
h.h
fubar
if test $? -ne 0 ; then no_result; fi
diff ok test.out4
if test $? -ne 0 ; then fail; fi

#
# build the change, change 3
#
# When the bug is expressed, this build concludes "all up to date".
# There *should* be something to do, because the mod time of h.h in the
# baseline should have been adjusted by aeipass.
#
activity="build 493"
rm $workchan.3/all*
if test $? -ne 0 ; then no_result; fi
$bin/aegis -b 3 -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# test the change
#
# If the new a.o in the baseline is linked against the old b.o in change
# 3's development directory, the regression test will fail, because each
# has a different MESSAGE.  The find should have failed before this,
# however the absence of a correct build, and thus the failure of
# subsequent tests, is the reported bug symptom.
#
activity="test 508"
$bin/aegis -test 3 -reg -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# the things tested in this test, worked
#
pass
