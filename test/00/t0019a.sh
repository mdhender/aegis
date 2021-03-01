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
	echo "NO RESULT for test of remove and recreate file ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of remove and recreate file ($activity)" 1>&2
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
#	and check files it should have made
#
activity="new project 135"
$bin/aegis -newpro foo -vers "" -dir $workproj -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# change project attributes
#
activity="project attributes 142"
cat > $tmp << 'end'
description = "test recreation of deleted files";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
$bin/aegis -proatt -f $tmp
if test $? -ne 0 ; then no_result; fi

#
# create a new change
#
activity="new change 156"
cat > $tmp << 'end'
brief_description = "create initial files";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p foo
if test $? -ne 0 ; then no_result; fi

#
# create a second change
#
activity="new change 168"
cat > $tmp << 'end'
brief_description = "delete a file";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f $tmp -p foo
if test $? -ne 0 ; then no_result; fi

#
# create a third change
#
activity="new change 180"
cat > $tmp << 'end'
brief_description = "recreate a file";
cause = internal_bug;
end
$bin/aegis -nc 3 -f $tmp -p foo
if test $? -ne 0 ; then no_result; fi

#
# add the staff
#
activity="staff 191"
$bin/aegis -newdev $USER
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newrev $USER
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newint $USER
if test $? -ne 0 ; then no_result; fi

#
# begin development of change 1
#
activity="develop begin 202"
$bin/aegis -devbeg 1 -dir $workchan
if test $? -ne 0 ; then no_result; fi

#
# add a few files to the change
#
activity="new file 209"
$bin/aegis -new_file $workchan/main.cc $workchan/junk -nl
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_file $workchan/aegis.conf -nl
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_file -list -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
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
# build the change
#
activity="build 245"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi

#
# difference the change
#
activity="diff 252"
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi

#
# finish development of the change
#
activity="develop end 259"
$bin/aegis -de
if test $? -ne 0 ; then no_result; fi

#
# pass the review
#
activity="review pass 266"
$bin/aegis -rpass 1
if test $? -ne 0 ; then no_result; fi

#
# integrate
#
activity="integrate begin 273"
$bin/aegis -ib 1
if test $? -ne 0 ; then no_result; fi
activity="build 276"
$bin/aegis -build -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="integrate pass 279"
$bin/aegis -ipass -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi

#
# develop change 2
#	remove a file
#
workchan=$work/foo.chan2.dir
activity="develop begin 287"
$bin/aegis -db 2 -dir $workchan
if test $? -ne 0 ; then no_result; fi
activity="remove file 290"
$bin/aegis -rm $workchan/junk -nl
if test $? -ne 0 ; then no_result; fi
activity="copy file 293"
$bin/aegis -cp $workchan/main.cc -nl
if test $? -ne 0 ; then no_result; fi
activity="build 296"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="diff 299"
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="develop end 302"
$bin/aegis -de
if test $? -ne 0 ; then no_result; fi
activity="review pass 305"
$bin/aegis -rpass 2
if test $? -ne 0 ; then no_result; fi

#
# integrate change 2
#
activity="integrate begin 312"
$bin/aegis -ib 2
if test $? -ne 0 ; then no_result; fi
activity="build 315"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="integrate pass 318"
$bin/aegis -ipass -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi

#
# develop change 3
#	recreate a file
#
workchan=$work/foo.chan3.dir
activity="develop begin 326"
$bin/aegis -db 3 -dir $workchan
if test $? -ne 0 ; then no_result; fi
activity="new file 329"
$bin/aegis -nf $workchan/junk -nl
if test $? -ne 0 ; then fail; fi
activity="copy file 332"
$bin/aegis -cp $workchan/main.cc -nl
if test $? -ne 0 ; then no_result; fi
activity="build 335"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="diff 338"
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
activity="develop end 341"
$bin/aegis -de
if test $? -ne 0 ; then fail; fi
activity="review pass 344"
$bin/aegis -rpass 3
if test $? -ne 0 ; then no_result; fi

#
# integrate change 3
#
activity="integrate begin 351"
$bin/aegis -ib 3
if test $? -ne 0 ; then no_result; fi
activity="build 354"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="integrate pass 357"
$bin/aegis -ipass -nl > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# the things tested in this test, worked
#
pass
