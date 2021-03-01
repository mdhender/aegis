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
# MANIFEST: Test core functionality.
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE

work=${AEGIS_TMP-/tmp}/$$

fail()
{
	set +x
	echo FAILED test of core functionality 1>&2
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
PAGER=cat
export PAGER
worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp

#
# make the directories
#
mkdir $work
if test $? -ne 0 ; then exit 1; fi

#
# make a new project
#	and check files it should have made
#
./bin/aegis -newpro foo -dir $workproj -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -newpro -list -lib $worklib
if test $? -ne 0 ; then fail; fi
if test ! -r $worklib/state ; then fail; fi
if test ! -r $workproj/info/state ; then fail; fi

#
# change project attributes
#
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
./bin/aegis -proatt -list --proj=foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -proatt $tmp -proj foo -lib $worklib
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
./bin/aegis -newcha -list -pro foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -new_change $tmp -project foo -lib $worklib
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/state ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail; fi

#
# create a second change
#	make sure it creates the files it should
#
cat > $tmp << 'end'
brief_description = "This change was added to make the various listings \
much more interesting.";
cause = internal_bug;
end
./bin/aegis -new_change $tmp -project foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -new_change -list -project foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add a new developer
#
./bin/aegis -newdev -list -pr foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -newdev $USER -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -newdev -list -pr foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# begin development of a change
#	check it made the files it should
#
./bin/aegis -devbeg -list -project foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail; fi
if test ! -r $worklib/user/$USER ; then fail; fi
./bin/aegis -new_change -list -project foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add a new files to the change
#
./bin/aegis -new_file -list -lib $worklib -proJ foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -new_file $workchan/main.c -nl -lib $worklib -Pro foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -new_file $workchan/config -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail; fi
./bin/aegis -new_file -list -lib $worklib -p foo
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

/*
 * file templates
 */
file_template =
[
	{
		pattern = [ "*" ];
		body = "hello\n";
	},
	{
		pattern = [ "test/*/*.sh" ];
		body = "#!/bin/sh\nexit 1\n";
	}
];
end

#
# create a new test
#
./bin/aegis -nt -l -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -nt -lib $worklib -p foo
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

#
# let the clock tick over, so the build will be happy
#
sleep 1

#
# build the change
#
./bin/aegis -build -list -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -build -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail; fi

#
# difference the change
#
./bin/aegis -diff -list -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -diff -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# test the change
#
./bin/aegis -test -l -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -test -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi

#
# finish development of the change
#
./bin/aegis -dev_end -list -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# add a new reviewer
#
./bin/aegis -newrev -list -pr foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -newrev $USER -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -newrev -list -pr foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# fail the review
#
./bin/aegis -review_fail -list -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
cat > $tmp << 'end'
This is a failed review comment.
end
./bin/aegis -review_fail $tmp -chan 1 -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# build the change again
# and difference
# and test
# end finish, again
#
sleep 1
./bin/aegis -build -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -diff -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -test -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# pass the review
#
./bin/aegis -review_pass -list -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -review_pass -chan 1 -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# add an integrator
#
./bin/aegis -newint -list -pr foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -newint $USER -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -newint -list -pr foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# start integrating
#
./bin/aegis -intbeg -l -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -intbeg 1 -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi
./bin/aegis -integrate_begin_undo -l -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# make sure -chdir works
#
./bin/aegis -cd -l -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
dir=`./bin/aegis -cd fred -bl -p foo -lib $worklib`
if test $? -ne 0 ; then fail; fi
if test $dir != $workproj/baseline/fred ; then fail; fi
dir=`./bin/aegis -cd -dd -p foo -c 1 fred -lib $worklib`
if test $? -ne 0 ; then fail; fi
if test $dir != $workchan/fred ; then fail; fi
dir=`./bin/aegis -cd -p foo -c 1 fred -lib $worklib`
if test $? -ne 0 ; then fail; fi
if test $dir != $workproj/delta.001/fred ; then fail; fi

#
# integrate build
#
./bin/aegis -build -l -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
sleep 1
./bin/aegis -build -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -test -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test -f $workproj/delta.001/aegis.log ; then fail; fi

#
# fail the integration
#
./bin/aegis -intfail -l -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
cat > $tmp << 'end'
This is a failed integration comment.
end
./bin/aegis -intfail $tmp -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi


#
# build the change again
# and difference
# and test
# and finish, again
# and review pass again
# and start integrating again
#
sleep 1
./bin/aegis -build -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -diff -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -test -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -review_pass -chan 1 -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -intbeg 1 -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# build and test the integration
#
sleep 1
./bin/aegis -build -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -test -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi

#
# pass the integration
#
./bin/aegis -intpass -l -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -intpass -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# check out the listings
#
./bin/aegis -list -list -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -list chahist -c 1 -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# start work on change 2
#
./bin/aegis -devbeg 2 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -cp -l -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# copy a file into the change
#
./bin/aegis -cp $workchan/main.c -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/002 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

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
./bin/aegis -nt -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
cat > $workchan/test/00/t0002a.sh << 'end'
#!/bin/sh
#
# Project: "foo"
# Change: 2
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

./foo ickky
q=$?

# check for signals
if test $q -ge 128 
then
	fail
fi

# should have complained
if test $q -eq 0 
then
	fail
fi

# it probably worked
pass
end

#
# tick over clock to keep build happy
#
sleep 1

#
# build the change
# diff the change
# test the change
#
./bin/aegis -b -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -diff -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -test -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -test -bl -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workchan/main.c,D ; then fail ; fi


#
# end development
# review pass
# start integrating
#
./bin/aegis -devend -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -revpass -c 2 -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
./bin/aegis -intbeg -c 2 -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# build the integration
# test the integration
# test the integration against the baseline
#
./bin/aegis -b -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -t -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
./bin/aegis -t -bl -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# pass the integration
#	make sure it create the files, etc
#
./bin/aegis -intpass -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/002 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi
./bin/aegis -l projhist -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi

# should be no automatic logging
if test "`find $work -name 'aegis.log' -print`" != "" ; then fail; fi

#
# the things tested in this test, worked
#
pass
