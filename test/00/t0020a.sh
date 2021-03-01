#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994, 1995 Peter Miller;
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
# MANIFEST: Test symlink farm fucntionality
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$

here=`pwd`
if test $? -ne 0 ; then exit 1; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

fail()
{
	set +x
	echo FAILED test of symlink farm functionality 1>&2
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
# program to ask questions about symlinks
#
cat > symlink.c << 'fubar'
#include <sys/types.h>
#include <sys/stat.h>
void
main(argc, argv)
	int	argc;
	char	**argv;
{
#ifdef S_IFLNK
	if (argc == 2)
	{
		struct stat st;
		if (lstat(argv[1], &st))
			exit(0);
		/* fails if file exists */
		exit(1);
	}
	exit(0);
#else
	exit(2);
#endif
}
fubar
if test $? -ne 0 ; then fail; fi
cc -o symlink symlink.c
if test $? -ne 0 ; then fail; fi

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
# make a new project
#	and check files it should have made
#
$bin/aegis -npr foo -dir $workproj
if test $? -ne 0 ; then fail; fi

#
# change project attributes
#
cat > $tmp << 'end'
description = "A bogus project created to test the symlink farm functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
if test $? -ne 0 ; then fail; fi
$bin/aegis -pa -f $tmp
if test $? -ne 0 ; then fail; fi

#
# create a new change
#	make sure it creates the files it should
#
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then fail; fi
$bin/aegis -nc -f $tmp -p foo
if test $? -ne 0 ; then fail; fi

#
# create a second change
#
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then fail; fi
$bin/aegis -nc -f $tmp -p foo
if test $? -ne 0 ; then fail; fi

#
# add the staff
#
$bin/aegis -nd $USER
if test $? -ne 0 ; then fail; fi
$bin/aegis -nrv $USER
if test $? -ne 0 ; then fail; fi
$bin/aegis -ni $USER
if test $? -ne 0 ; then fail; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan
if test $? -ne 0 ; then fail; fi

#
# add a new files to the change
#
$bin/aegis -nf $workchan/main.c $workchan/test.c $workchan/Makefile \
	$workchan/config -nl
if test $? -ne 0 ; then fail; fi
cat > $workchan/main.c << 'end'
void main() { test(); exit(0); }
end
if test $? -ne 0 ; then fail; fi
cat > $workchan/test.c << 'end'
void test() { }
end
if test $? -ne 0 ; then fail; fi
cat > $workchan/Makefile << 'end'
.c.o:
	rm -f $*.o
	$(CC) $(CFLAGS) -D'VERSION="$(VERSION)"' -c $*.c

foo: main.o test.o
	rm -f foo
	$(CC) -o foo main.o test.o
end
if test $? -ne 0 ; then fail; fi
cat > $workchan/config << 'end'
build_command = "make VERSION=$version";
link_integration_directory = true;
create_symlinks_before_build = true;
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
if test $? -ne 0 ; then fail; fi

#
# create a new test
#
$bin/aegis -nt
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
if test $? -ne 0 ; then fail ; fi

# it probably worked
pass
end
if test $? -ne 0 ; then fail; fi

#
# build the change
#
$bin/aegis -build -nl -v
if test $? -ne 0 ; then fail; fi

#
# difference the change
#
$bin/aegis -diff > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# test the change
#
$bin/aegis -t -v
if test $? -ne 0 ; then fail; fi

#
# finish development of the change
#
$bin/aegis -de
if test $? -ne 0 ; then fail; fi

#
# pass the review
#
$bin/aegis -rpass -c 1
if test $? -ne 0 ; then fail; fi

#
# start integrating
#
$bin/aegis -ib 1
if test $? -ne 0 ; then fail; fi

#
# integrate build and test
#
$bin/aegis -b -nl -v
if test $? -ne 0 ; then fail; fi
$bin/aegis -t -nl -v
if test $? -ne 0 ; then fail; fi

#
# pass the integration
#
$bin/aegis -intpass -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

#
# start work on change 2
#
$bin/aegis -db -c 2 -dir $workchan
if test $? -ne 0 ; then fail; fi

#
# copy a file into the change
#
$bin/aegis -cp $workchan/main.c -nl
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
	test();
	exit(0);
}
end
if test $? -ne 0 ; then fail; fi

#
# need another test
#
$bin/aegis -nt
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
if test $? -ne 1 ; then fail ; fi

# it probably worked
pass
end
if test $? -ne 0 ; then fail; fi

#
# build the change
# diff the change
# test the change
#
$bin/aegis -b -nl -v
if test $? -ne 0 ; then fail; fi
$bin/aegis -diff -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi
$bin/aegis -t -nl -v
if test $? -ne 0 ; then fail; fi
$bin/aegis -t -bl -nl -v
if test $? -ne 0 ; then fail; fi

#
# end development
# review pass
# start integrating
#
$bin/aegis -de
if test $? -ne 0 ; then fail; fi
$bin/aegis -revpass -c 2
if test $? -ne 0 ; then fail; fi
$bin/aegis -intbeg -c 2
if test $? -ne 0 ; then fail; fi

#
# build the integration
# test the integration
# test the integration against the baseline
#
$bin/aegis -b -nl -v
if test $? -ne 0 ; then fail; fi
$bin/aegis -t -nl -v
if test $? -ne 0 ; then fail; fi
$bin/aegis -t -bl -nl -v
if test $? -ne 0 ; then fail; fi

#
# pass the integration
#
$bin/aegis -intpass -nl > /dev/null 2>&1
if test $? -ne 0 ; then fail; fi

# should be no automatic logging
if test "`find $work -name 'aegis.log' -print`" != "" ; then fail; fi

#
# the things tested in this test, worked
#
pass
