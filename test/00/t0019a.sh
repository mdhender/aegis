#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998 Peter Miller;
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
# MANIFEST: Test remove and recreate file
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
AEGIS_THROTTLE=2
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
activity="working directory 81"
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
	cat >> $work/cc << fubar
#!/bin/sh
exec $CC \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx $work/cc
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# make a new project
#	and check files it should have made
#
activity="new project 115"
$bin/aegis -newpro foo -vers "" -dir $workproj -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# change project attributes
#
activity="project attributes 122"
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
activity="new change 136"
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
activity="new change 148"
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
activity="new change 160"
cat > $tmp << 'end'
brief_description = "recreate a file";
cause = internal_bug;
end
$bin/aegis -nc 3 -f $tmp -p foo
if test $? -ne 0 ; then no_result; fi

#
# add the staff
#
activity="staff 171"
$bin/aegis -newdev $USER
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newrev $USER
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newint $USER
if test $? -ne 0 ; then no_result; fi

#
# begin development of change 1
#
activity="develop begin 182"
$bin/aegis -devbeg 1 -dir $workchan
if test $? -ne 0 ; then no_result; fi

#
# add a few files to the change
#
activity="new file 189"
$bin/aegis -new_file $workchan/main.c $workchan/junk -nl
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_file $workchan/config -nl
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_file -list -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
cat > $workchan/main.c << 'end'
void main() { exit(0); }
end
if test $? -ne 0 ; then no_result; fi
cat > $workchan/config << 'end'
build_command = "rm -f foo; cc -o foo -D'VERSION=\"$v\"' main.c";

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
# build the change
#
activity="build 221"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi

#
# difference the change
#
activity="diff 228"
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi

#
# finish development of the change
#
activity="develop end 235"
$bin/aegis -de
if test $? -ne 0 ; then no_result; fi

#
# pass the review
#
activity="review pass 242"
$bin/aegis -rpass 1
if test $? -ne 0 ; then no_result; fi

#
# integrate
#
activity="integrate begin 249"
$bin/aegis -ib 1
if test $? -ne 0 ; then no_result; fi
activity="build 252"
$bin/aegis -build -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="integrate pass 255"
$bin/aegis -ipass -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi

#
# develop change 2
#	remove a file
#
activity="develop begin 263"
$bin/aegis -db 2 -dir $workchan
if test $? -ne 0 ; then no_result; fi
activity="remove file 266"
$bin/aegis -rm $workchan/junk -nl
if test $? -ne 0 ; then no_result; fi
activity="copy file 269"
$bin/aegis -cp $workchan/main.c -nl
if test $? -ne 0 ; then no_result; fi
activity="build 272"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="diff 275"
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="develop end 278"
$bin/aegis -de
if test $? -ne 0 ; then no_result; fi
activity="review pass 281"
$bin/aegis -rpass 2
if test $? -ne 0 ; then no_result; fi

#
# integrate change 2
#
activity="integrate begin 288"
$bin/aegis -ib 2
if test $? -ne 0 ; then no_result; fi
activity="build 291"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="integrate pass 294"
$bin/aegis -ipass -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi

#
# develop change 3
#	recreate a file
#
activity="develop begin 302"
$bin/aegis -db 3 -dir $workchan
if test $? -ne 0 ; then no_result; fi
activity="new file 305"
$bin/aegis -nf $workchan/junk -nl
if test $? -ne 0 ; then fail; fi
activity="copy file 308"
$bin/aegis -cp $workchan/main.c -nl
if test $? -ne 0 ; then no_result; fi
activity="build 311"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="diff 314"
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
activity="develop end 317"
$bin/aegis -de
if test $? -ne 0 ; then fail; fi
activity="review pass 320"
$bin/aegis -rpass 3
if test $? -ne 0 ; then no_result; fi

#
# integrate change 3
#
activity="integrate begin 327"
$bin/aegis -ib 3
if test $? -ne 0 ; then no_result; fi
activity="build 330"
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then no_result; fi
activity="integrate pass 333"
$bin/aegis -ipass -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass
