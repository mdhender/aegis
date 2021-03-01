#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# MANIFEST: Test remove and recreate file
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$

here=`pwd`
if test $? -ne 0; then exit 1; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

fail()
{
	set +x
	echo FAILED test of remove and recreate file 1>&2
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
trap "fail" 1 2 3 15

#
# some variable to make things earier to read
#
PAGER=cat
export PAGER

AEGIS_FLAGS="delete_file_preference = no_keep; \
	diff_preference = automatic_merge;"
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
mkdir $work
if test $? -ne 0 ; then fail; fi
cd $work
if test $? -ne 0 ; then fail; fi

#
# make a new project
#	and check files it should have made
#
$bin/aegis -newpro foo -dir $workproj -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# change project attributes
#
cat > $tmp << 'end'
description = "test recreation of deleted files";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
$bin/aegis -proatt -f $tmp
if test $? -ne 0 ; then fail; fi

#
# create a new change
#
cat > $tmp << 'end'
brief_description = "create initial files";
cause = internal_bug;
end
$bin/aegis -nc -f $tmp -p foo
if test $? -ne 0 ; then fail; fi

#
# create a second change
#
cat > $tmp << 'end'
brief_description = "delete a file";
cause = internal_bug;
end
$bin/aegis -nc -f $tmp -p foo
if test $? -ne 0 ; then fail; fi

#
# create a third change
#
cat > $tmp << 'end'
brief_description = "recreate a file";
cause = internal_bug;
end
$bin/aegis -nc -f $tmp -p foo
if test $? -ne 0 ; then fail; fi

#
# add the staff
#
$bin/aegis -newdev $USER
if test $? -ne 0 ; then fail; fi
$bin/aegis -newrev $USER
if test $? -ne 0 ; then fail; fi
$bin/aegis -newint $USER
if test $? -ne 0 ; then fail; fi

#
# begin development of change 1
#
$bin/aegis -devbeg 1 -dir $workchan
if test $? -ne 0 ; then fail; fi

#
# add a few files to the change
#
$bin/aegis -new_file $workchan/main.c $workchan/junk -nl
if test $? -ne 0 ; then fail; fi
$bin/aegis -new_file $workchan/config -nl
if test $? -ne 0 ; then fail; fi
$bin/aegis -new_file -list -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
cat > $workchan/main.c << 'end'
void main() { exit(0); }
end
if test $? -ne 0 ; then fail; fi
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
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# difference the change
#
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# finish development of the change
#
$bin/aegis -de
if test $? -ne 0 ; then fail; fi

#
# pass the review
#
$bin/aegis -rpass 1
if test $? -ne 0 ; then fail; fi

#
# integrate
#
$bin/aegis -ib 1
if test $? -ne 0 ; then fail; fi
$bin/aegis -build -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -ipass -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# develop change 2
#	remove a file
#
$bin/aegis -db 2 -dir $workchan
if test $? -ne 0 ; then fail; fi
$bin/aegis -rm $workchan/junk -nl
if test $? -ne 0 ; then fail; fi
$bin/aegis -cp $workchan/main.c -nl
if test $? -ne 0 ; then fail; fi
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -de
if test $? -ne 0 ; then fail; fi
$bin/aegis -rpass 2
if test $? -ne 0 ; then fail; fi

#
# integrate change 2
#
$bin/aegis -ib 2
if test $? -ne 0 ; then fail; fi
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -ipass -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# develop change 3
#	recreate a file
#
$bin/aegis -db 3 -dir $workchan
if test $? -ne 0 ; then fail; fi
$bin/aegis -nf $workchan/junk -nl
if test $? -ne 0 ; then fail; fi
$bin/aegis -cp $workchan/main.c -nl
if test $? -ne 0 ; then fail; fi
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -de
if test $? -ne 0 ; then fail; fi
$bin/aegis -rpass 3
if test $? -ne 0 ; then fail; fi

#
# integrate change 3
#
$bin/aegis -ib 3
if test $? -ne 0 ; then fail; fi
$bin/aegis -b -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -ipass -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass
