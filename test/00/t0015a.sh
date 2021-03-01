#! /bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1993 Peter Miller.
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
# MANIFEST: Test the -Integrate_Begin_Undo functionality
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE

work=${AEGIS_TMP-/tmp}/$$
PAGER=cat
export PAGER

here=`pwd`
if test $? -ne 0 ; then exit 1; fi
mkdir $work
if test $? -ne 0 ; then exit 1; fi

fail()
{
	set +x
	echo FAILED test of the -Integrate_Begin_Undo functionality 1>&2
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

cd $work
if test $? -ne 0 ; then fail; fi

#
# some variable to make things earier to read
#
worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp

#
# make a new project
#	and check files it should have made
#
$here/bin/aegis -newpro foo -dir $workproj -lib $worklib
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
$here/bin/aegis -proatt $tmp -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# create a new change
#	make sure it creates the files it should
#
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
$here/bin/aegis -new_change $tmp -project foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add a new developer
#
$here/bin/aegis -newdev $USER -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# begin development of a change
#	check it made the files it should
#
$here/bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add a new files to the change
#
$here/bin/aegis -new_file $workchan/main.c -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -new_file $workchan/config -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
cat > $workchan/main.c << 'end'
void
main()
{
	exit(0);
}
end
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

#
# create a new test
#
$here/bin/aegis -nt -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
#
# Project: "foo"
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

#
# let the clock tick over, so the build will be happy
#
sleep 1

#
# build the change
#
$here/bin/aegis -build -nl -lib $worklib -p foo > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# difference the change
#
$here/bin/aegis -diff -nl -lib $worklib -p foo > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# test the change
#
$here/bin/aegis -test -nl -lib $worklib -p foo > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# finish development of the change
#
$here/bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# add a new reviewer
#
$here/bin/aegis -newrev $USER -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# pass the review
#
$here/bin/aegis -review_pass -chan 1 -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add an integrator
#
$here/bin/aegis -newint $USER -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# start integrating
#
$here/bin/aegis -ib 1 -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# integrate build
#
$here/bin/aegis -ibu -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass
