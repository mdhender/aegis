#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1993, 1994, 1995 Peter Miller;
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
# MANIFEST: Test 'aegis -DIFFerence -ANticipate' variant
#
# The bug manifested as a string of length 0 being passed.
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

PAGER=cat
export PAGER

AEGIS_FLAGS="delete_file_preference = no_keep; \
	diff_preference = automatic_merge;"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

work=${AEGIS_TMP:-/tmp}/$$

here=`pwd`
if test $? -ne 0; then exit 1; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

fail()
{
	set +x
	echo "FAILED test of 'aegis -DIFFerence -ANticipate' variant" 1>&2
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
worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
workchan3=$work/foo.chan3
tmp=$work/tmp

#
# echo commands so we can tell what failed
#
set -x

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
$bin/aegis -newpro foo -dir $workproj -v -lib $worklib
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
$bin/aegis -proatt -f $tmp -proj foo -v -lib $worklib
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
$bin/aegis -new_change -f $tmp -project foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# create a second change
#	make sure it creates the files it should
#
cat > $tmp << 'end'
brief_description = "This change was added to make the various listings \
much more interesting.";
cause = internal_bug;
end
$bin/aegis -new_change -f $tmp -project foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# create a third change
#
cat > $tmp << 'end'
brief_description = "change three";
cause = internal_bug;
end
$bin/aegis -new_change -f $tmp -project foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add a new developer
#
$bin/aegis -newdev $USER -p foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# begin development of a change
#	check it made the files it should
#
$bin/aegis -devbeg 1 -p foo -dir $workchan -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add a new files to the change
#
$bin/aegis -new_file $workchan/main.c -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
$bin/aegis -new_file $workchan/config -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
cat > $workchan/main.c << 'end'
void
main()
{
	exit(0);
}
end
cat > $workchan/config << 'end'
build_command = "rm -f foo; cc -o foo -D'VERSION=\"$version\"' main.c";
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
$bin/aegis -nt -l -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
$bin/aegis -nt -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh

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
if test $? -ne 0; then fail; fi

# it probably worked
pass
end

#
# build the change
#
$bin/aegis -build -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# difference the change
#
$bin/aegis -diff -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# test the change
#
$bin/aegis -test -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# finish development of the change
#
$bin/aegis -dev_end -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# add a new reviewer
#
$bin/aegis -newrev $USER -p foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# pass the review
#
$bin/aegis -review_pass -chan 1 -proj foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add an integrator
#
$bin/aegis -newint $USER -p foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# start integrating
#
$bin/aegis -intbeg 1 -p foo -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# integrate build
#
$bin/aegis -build -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
$bin/aegis -test -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# pass the integration
#
$bin/aegis -intpass -nl -v -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# start work on change 2
#
$bin/aegis -devbeg 2 -p foo -v -dir $workchan -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# start work on change 3
#
$bin/aegis -devbeg 3 -p foo -v -dir $workchan3 -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# copy a file into change 2
#
$bin/aegis -cp $workchan/main.c -nl -v -lib $worklib -c 2 -p foo
if test $? -ne 0 ; then fail; fi

#
# copy a file into change 3
#
$bin/aegis -cp $workchan3/main.c -nl -v -lib $worklib -c 3 -p foo
if test $? -ne 0 ; then fail; fi

#
# change the file
#
cat > $workchan/main.c << 'end'

#include <stdio.h>

void
main(argc, argv)
	int	argc;
	char	**argv;
{
	if (argc != 1)
	{
		fprintf(stderr, "usage: %s\n", argv[0]);
		exit(1);
	}
	printf("hello, world\n");
	exit(0);
}
end

#
# need another test
#
$bin/aegis -nt -v -lib $worklib -c 2 -p foo
if test $? -ne 0 ; then fail; fi
cat > $workchan/test/00/t0002a.sh << 'end'
#!/bin/sh

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

./foo ickky
if test $? -ne 1; then fail; fi

# it probably worked
pass
end

#
# build the change
# diff the change
# test the change
#
$bin/aegis -b -nl -v -lib $worklib -c 2 -p foo
if test $? -ne 0 ; then fail; fi
$bin/aegis -diff -nl -v -lib $worklib -c 2 -p foo
if test $? -ne 0 ; then fail; fi
$bin/aegis -test -nl -v -lib $worklib -c 2 -p foo
if test $? -ne 0 ; then fail; fi
$bin/aegis -test -bl -nl -v -lib $worklib -c 2 -p foo
if test $? -ne 0 ; then fail; fi

#
# end development
# review pass
# start integrating
#
$bin/aegis -devend -v -lib $worklib -c 2 -p foo
if test $? -ne 0 ; then fail; fi
$bin/aegis -revpass -v -c 2 -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
$bin/aegis -intbeg -v -c 2 -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# difference change 3 anticipating 2
#
$bin/aegis -diff -nl -c 3 -lib $worklib -p foo -an 2
if test $? -ne 0 ; then fail; fi

#
# the merged result (in change 3) should look like
# the edited one from change 2
#
diff $workchan3/main.c,D $workchan/main.c
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass
