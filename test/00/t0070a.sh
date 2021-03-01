#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994-1998, 2004-2008 Peter Miller
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
	echo "NO RESULT for test of symlink farm functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of symlink farm functionality ($activity)" 1>&2
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
AEGIS_THROTTLE=1
export AEGIS_THROTTLE

worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT

#
# make the directories
#
activity="working directory 96"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

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
# program to ask questions about symlinks
#
cat > symlink.cc << 'fubar'
#include <sys/types.h>
#include <sys/stat.h>
int
main(int argc, char **argv)
{
#ifdef S_IFLNK
	if (argc == 2)
	{
		struct stat st;
		if (lstat(argv[1], &st))
			return 0;
		/* fails if file exists */
		return 1;
	}
	return 0;
#else
	return 2;
#endif
}
fubar
if test $? -ne 0 ; then no_result; fi
c++ -o symlink symlink.cc
if test $? -ne 0 ; then no_result; fi

#
# if this system has no symlinks
# automagically pass the test
#
./symlink
if test $? -ge 2
then
	echo ""
	echo ""
	echo "	This system does not have symbolic links"
	echo "	so this test automagically passes."
	echo ""
	echo ""
	pass
fi

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# make a new project
#	and check files it should have made
#
activity="new project 179"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 186"
cat > $tmp << 'end'
description = "A bogus project created to test the symlink farm functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#	make sure it creates the files it should
#
activity="new change 201"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a second change
#
activity="new change 213"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 225"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 242"
$bin/aegis -nf $workchan/main.cc $workchan/test.cc $workchan/Makefile \
	$workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.cc << 'end'
extern void test(void);

int
main(int argc, char **argv)
{
	test();
	return 0;
}
end
if test $? -ne 0 ; then no_result; fi
cat > $workchan/test.cc << 'end'
void
test(void)
{
}
end
if test $? -ne 0 ; then no_result; fi
cat > $workchan/Makefile << 'end'
# SGImake doesn't have .cc in its default suffix list
.SUFFIXES: .cc

.cc.o:
	rm -f $*.o
	c++ $(CFLAGS) -D'VERSION="$(VERSION)"' -c $*.cc

foo: main.o test.o
	rm -f foo
	c++ -o foo main.o test.o
end
if test $? -ne 0 ; then no_result; fi
cat > $workchan/aegis.conf << 'end'
build_command = "make VERSION=$version";
link_integration_directory = true;
create_symlinks_before_build = true;

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
activity="new test 298"
$bin/aegis -nt > log 2>&1
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
if test $? -ne 0 ; then fail ; fi

# it probably worked
pass
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
# This will try to do symlink stuff.
#
activity="build 333"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 340"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 347"
$bin/aegis -t -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
#	It must ignore the symlinks when it chmod's
#
activity="develop end 356"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 363"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 370"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build and test
#
activity="build 377"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="test 380"
$bin/aegis -t -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 387"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start work on change 2
#
#	It will create symlinks into the baseline
#
activity="develop begin 396"
$bin/aegis -db -c 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# copy a file into the change
#
#	It will have to remove the symlink before copying
#
activity="copy file 405"
$bin/aegis -cp $workchan/main.cc -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# change the file
#
cat > $workchan/main.cc << 'end'
#include <stdio.h>
extern void test(void);

int
main(int argc, char **argv)
{
	if (argc != 1)
	{
		fprintf(stderr, "usage: %s\n", argv[0]);
		return 1;
	}
	test();
	return 0;
}
end
if test $? -ne 0 ; then no_result; fi

#
# need another test
#
activity="new test 433"
$bin/aegis -nt > log 2>&1
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

./foo ickky
if test $? -ne 1 ; then fail ; fi

# it probably worked
pass
end
if test $? -ne 0 ; then no_result; fi

#
# build the change - will double check symlinks
#	and the make will fail if the symlinks are not there
# diff the change
# test the change
#
activity="build 472"
$bin/aegis -b -nl -v -minimum > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="diff 475"
$bin/aegis -diff -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="test 478"
$bin/aegis -t -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="test baseline 481"
$bin/aegis -t -bl -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# end development
# review pass
# start integrating
#
activity="develop end 490"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="review pass 493"
$bin/aegis -revpass -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate begin 496"
$bin/aegis -intbeg -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# build the integration
# test the integration
# test the integration against the baseline
#
activity="build 505"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="test 508"
$bin/aegis -t -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="test baseline 511"
$bin/aegis -t -bl -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 518"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# should be no automatic logging
if test "`find $work -name 'aegis.log' -print`" != "" ; then no_result; fi

#
# the things tested in this test, worked
#
pass
