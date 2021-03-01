#! /bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
# MANIFEST: Test -Review_Pass_Undo functionality.
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE

PAGER=cat
export PAGER
work=${AEGIS_TMP-/tmp}/$$

fail()
{
	set +x
	echo FAILED test of -Review_Pass_Undo functionality 1>&2
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	echo PASSED 1>&2
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
}
trap "fail" 1 2 3 15

#
# some variable to make things earier to read
#
worklib=$work/lib
workproj=$work/foo.proj
workproj2=$work/foo.proj2
workchan=$work/foo.chan
tmp=$work/tmp

#
# echo commands so we can tell what failed
#
set -x

#
# make the directories
#
mkdir $work

#
# make a new project
#	and check files it should have made
#
./bin/aegis -newpro foo -dir $workproj -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# change project attributes
#
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
./bin/aegis -proatt $tmp -proj foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# create a new change
#
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
./bin/aegis -new_change $tmp -project foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add a new developer
#
./bin/aegis -newdev $USER -p foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# begin development of a change
#
./bin/aegis -devbeg 1 -p foo -dir $workchan -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add a new files to the change
#
./bin/aegis -new_file $workchan/main.c -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -new_file $workchan/config -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
cat > $workchan/main.c << 'end'
void
main()
{
	exit(0);
}
end
if test $? -ne 0 ; then fail; fi
cat > $workchan/config << 'end'
build_command = "rm -f foo; cc -o foo -D'VERSION=\"$v\"' main.c";
link_integration_directory = true;

history_get_command =
	"co -u'$e' -p $h,v > $o";
history_create_command =
	"ci -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_put_command =
	"ci -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_query_command =
	"rlog -r $h,v | awk '/^head:/ {print $$2}'";

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";

diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
end
if test $? -ne 0 ; then fail; fi

#
# create a new test
#
./bin/aegis -nt -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
#
# Project: "foo"
# Change: 1
#

fail()
{
	echo SHUZBUTT 1>&2
	exit 1
}
pass()
{
	exit 0
}
trap "fail" 1 2 3 15

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
if test $? -ne 0 ; then fail; fi

#
# let the clock tick over, so the build will be happy
#
sleep 1

#
# build the change
#
./bin/aegis -build -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# difference the change
#
./bin/aegis -diff -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# test the change
#
./bin/aegis -test -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# finish development of the change
#
./bin/aegis -dev_end -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# add a new reviewer
#
./bin/aegis -newrev $USER -p foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# pass the review
#
./bin/aegis -review_pass -chan 1 -proj foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# undo the review pass
#
./bin/aegis -review_pass_undo -chan 1 -proj foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# make sure it worked
#
./bin/aegis -list change_details -project foo -change 1 -verbose -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass
