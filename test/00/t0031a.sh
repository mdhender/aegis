#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1995-2005 Peter Miller;
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
# MANIFEST: Test branching functionality.
#
# This test creates a project, and performs a single change.  It then
# creates a branch, and performs another change on that branch.
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

bin="$here/${1-.}/bin"

activity="create working directory 44"

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
	echo FAILED test of branching functionality "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo NO RESULT test of branching functionality "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap "no_result" 1 2 3 15

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*";/node = "NODE";/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		-e 's/uuid = ".*"/uuid = "UUID"/' \
		< $1 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $2 $work/sed.out
	if test $? -ne 0; then fail; fi
}

#
# some variables to make things easier to read
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

#
# make the directories
#
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi
chmod 0777 $work/lib
if test $? -ne 0 ; then no_result; fi

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# If the C++ compiler is called something other than ``c++'', as
# discovered by the configure script, create a shell script called
# ``c++'' which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
#
if test "$CXX" != "" -a "$CXX" != "c++"
then
	cat >> $work/c++ << fubar
#!/bin/sh
exec $CXX \$*
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
activity="new project 152"
$bin/aegis -newpro foo -dir $workproj -lib $worklib -vers '' > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 159"
cat > $tmp << 'end'
description = "A bogus project created to test branching.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -proatt -f $tmp -proj foo -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 173"
cat > $tmp << 'end'
brief_description = "change one";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_change 1 -f $tmp -project foo -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add staff
#
activity="staff 185"
$bin/aegis -newdev $USER -p foo -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -newrev $USER -p foo -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -newint $USER -p foo -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 196"
$bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="new file 199"
$bin/aegis -new_file $workchan/main.cc $workchan/aegis.conf -nl -lib $worklib \
	-Pro foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/main.cc << 'end'
int
main(int argc, char **argv)
{
	return 0;
}
end
cat > $workchan/aegis.conf << 'end'
build_command =
    "rm -f foo; c++ -o foo -D'VERSION=\"$v\"' -D'PATH=\"$sp\"' main.cc";

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
activity="new test 248"
$bin/aegis -nt -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
pass()
{
	exit 0
}
fail()
{
	echo SHUZBUTT 1>&2
	exit 1
}
no_result()
{
	echo WHIMPER 1>&2
	exit 2
}
trap "no_result" 1 2 3 15
./foo
if test $? -ne 0; then fail; fi
pass
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 277"
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out;no_result; fi

#
# difference the change
#
activity="diff 284"
$bin/aegis -diff -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# test the change
#
activity="test 291"
$bin/aegis -test -lib $worklib -p foo -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
aectivity="develop end 241"
$bin/aegis -dev_end -lib $worklib -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 305"
$bin/aegis -review_pass -chan 1 -proj foo -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 312"
$bin/aegis -intbeg 1 -p foo -lib $worklib -v > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# integrate build
#
activity="integration build 319"
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# integrate test
#
activity="integration test 326"
$bin/aegis -test -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# check the file state
#
cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		action = create;
		usage = config;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "main.cc";
		action = create;
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "test/00/t0001a.sh";
		action = create;
		usage = test;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		architecture_times =
		[
			{
				variant = "unspecified";
				test_time = TIME;
			},
		];
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/001.fs ok

#
# pass the integration
#
activity="integrate pass 402"
$bin/aegis -intpass -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# create a new branch
#
activity="new branch 409"
$bin/aegis -nbr -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
cat > ok << 'fubar'
brief_description = "A bogus project created to test branching, branch 2.";
description = "A bogus project created to test branching, branch 2.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
state = being_developed;
development_directory = "branch.2";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "USER";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "USER";
	},
];
branch =
{
	umask = 022;
	developer_may_review = true;
	developer_may_integrate = true;
	reviewer_may_integrate = true;
	developers_may_create_changes = false;
	default_test_exemption = false;
	default_test_regression_exemption = true;
	skip_unlucky = false;
	compress_database = false;
	develop_end_action = goto_being_reviewed;
	change =
	[
	];
	administrator =
	[
		"USER",
	];
	developer =
	[
		"USER",
	];
	reviewer =
	[
		"USER",
	];
	integrator =
	[
		"USER",
	];
	minimum_change_number = 10;
	reuse_change_numbers = true;
	minimum_branch_number = 1;
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $work/foo.proj/info/change/0/002 ok

#
# check that the branch shows in the project list
#
activity="project list 480"
$bin/aegis -l p -unf -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
grep '^foo[.0-9]* ' < test.out > test.out2
if test $? -ne 0 ; then no_result; fi
cat > ok << 'fubar'
foo .../foo.proj A bogus project created to test branching.
foo.2 .../foo.proj/branch.2 A bogus project created to test branching, branch 2.
fubar
if test $? -ne 0 ; then no_result; fi
check_it test.out2 ok

#
# make sure the right state files have been created
# and that they contain the right stuff
#
activity="check branch state files 496"
cat > ok << fubar
brief_description = "A bogus project created to test branching, branch 2.";
description = "A bogus project created to test branching, branch 2.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
state = being_developed;
development_directory = "branch.2";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "USER";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "USER";
	},
];
branch =
{
	umask = 022;
	developer_may_review = true;
	developer_may_integrate = true;
	reviewer_may_integrate = true;
	developers_may_create_changes = false;
	default_test_exemption = false;
	default_test_regression_exemption = true;
	skip_unlucky = false;
	compress_database = false;
	develop_end_action = goto_being_reviewed;
	change =
	[
	];
	administrator =
	[
		"USER",
	];
	developer =
	[
		"USER",
	];
	reviewer =
	[
		"USER",
	];
	integrator =
	[
		"USER",
	];
	minimum_change_number = 10;
	reuse_change_numbers = true;
	minimum_branch_number = 1;
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/002 ok
if test ! -d $workproj/branch.2; then fail; fi
if test ! -d $workproj/branch.2/baseline; then fail; fi

#
# create a change on the new wbranch
#
cat > $tmp << 'end'
brief_description = "change one of branch two";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_change 1 -f $tmp -project foo-2 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# make sure the right state files have been created
# and that they contain the right stuff
#
activity="check change state file 579"
cat > ok << fubar
brief_description = "change one of branch two";
description = "change one of branch two";
cause = internal_bug;
test_exempt = false;
test_baseline_exempt = false;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
state = awaiting_development;
given_regression_test_exemption = true;
history =
[
	{
		when = TIME;
		what = new_change;
		who = "USER";
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/002.branch/0/001 ok
cat > ok << fubar
src =
[
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/002.branch/0/001.fs ok

#
# start work on foo-2, change 1
#
activity="develop begin 615"
$bin/aegis -devbeg 1 -p foo-2 -dir $workchan -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# check the contents of the state files
#
activity="check user state file 622"
cat > ok << fubar
own =
[
	{
		project_name = "foo.2";
		changes =
		[
			1,
		];
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $worklib/user/$USER ok

#
# copy a file into the change
#
activity="copy file 641"
$bin/aegis -cp $workchan/main.cc -nl -lib $worklib -p foo-2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# change the file
#
cat > $workchan/main.cc << 'end'

#include <stdio.h>

int
main(int argc, char **argv)
{
	if (argc != 1)
	{
		fprintf(stderr, "usage: %s\n", argv[0]);
		return 1;
	}
	printf("hello, world\n");
	return 0;
}
end
if test $? -ne 0 ; then no_result; fi

#
# need another test
#
activity="new test 669"
$bin/aegis -nt -lib $worklib -p foo-2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
cat > $workchan/test/00/t0002a.sh << 'end'
#!/bin/sh
set -x
pass()
{
	exit 0
}
fail()
{
	echo SHUZBUTT 1>&2
	exit 1
}
no_result()
{
	echo WHIMPER 1>&2
	exit 2
}
trap "no_result" 1 2 3 15
./foo > /dev/null 2>&1
if test $? -ne 0; then fail; fi
./foo ickky
if test $? -ne 1; then fail; fi
pass
end

#
# build the change
# diff the change
# test the change
#
activity="build 702"
$bin/aegis -b -nl -lib $worklib -p foo-2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="diff 705"
$bin/aegis -diff -nl -lib $worklib -p foo-2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="test 708"
$bin/aegis -test -nl -lib $worklib -p foo-2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="test bl 711"
$bin/aegis -test -bl -nl -lib $worklib -p foo-2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="test reg 714"
$bin/aegis -test -reg -nl -lib $worklib -p foo-2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# end development
# review pass
# start integrating
#
activity="devlop end 723"
$bin/aegis -devend -lib $worklib -p foo-2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="review pass 726"
$bin/aegis -revpass -c 1 -p foo-2 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate begin 729"
$bin/aegis -intbeg -c 1 -p foo-2 -lib $worklib -v > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# build the integration
# test the integration
# test the integration against the baseline
#
activity="integrate build 738"
$bin/aegis -b -nl -lib $worklib -p foo-2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate diff 741"
$bin/aegis -diff -nl -lib $worklib -p foo-2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="integrate test 744"
$bin/aegis -t -nl -lib $worklib -p foo-2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
$bin/aegis -t -bl -nl -lib $worklib -p foo-2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
$bin/aegis -t -reg -nl -lib $worklib -p foo-2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# check the project file state
#
activity="check change file state 755"
cat > ok << 'fubar'
src =
[
	{
		file_name = "main.cc";
		uuid = "UUID";
		action = transparent;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = source;
		locked_by = 1;
		about_to_be_copied_by = 1;
		test =
		[
			"test/00/t0001a.sh",
		];
	},
	{
		file_name = "test/00/t0002a.sh";
		action = transparent;
		usage = test;
		locked_by = 1;
		about_to_be_created_by = 1;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/002.fs ok

#
# pass the integration
#	branch 2, change 1
#
activity="integrate pass 797"
$bin/aegis -intpass -nl -lib $worklib -p foo-2 > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# check the contents of the state files
#
activity="check branch state 804"
cat > ok << 'fubar'
brief_description = "A bogus project created to test branching, branch 2.";
description = "A bogus project created to test branching, branch 2.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
copyright_years =
[
	YYYY,
];
state = being_developed;
build_time = TIME;
test_time = TIME;
test_baseline_time = TIME;
regression_test_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
		test_time = TIME;
		test_baseline_time = TIME;
		regression_test_time = TIME;
	},
];
development_directory = "branch.2";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "USER";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "USER";
	},
];
branch =
{
	umask = 022;
	developer_may_review = true;
	developer_may_integrate = true;
	reviewer_may_integrate = true;
	developers_may_create_changes = false;
	default_test_exemption = false;
	default_test_regression_exemption = true;
	skip_unlucky = false;
	compress_database = false;
	develop_end_action = goto_being_reviewed;
	history =
	[
		{
			delta_number = 1;
			change_number = 1;
		},
	];
	change =
	[
		1,
	];
	administrator =
	[
		"USER",
	];
	developer =
	[
		"USER",
	];
	reviewer =
	[
		"USER",
	];
	integrator =
	[
		"USER",
	];
	minimum_change_number = 10;
	reuse_change_numbers = true;
	minimum_branch_number = 1;
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then cat test.out; no_result; fi
check_it $workproj/info/change/0/002 ok
activity="check branch file state 896"
cat > ok << 'fubar'
src =
[
	{
		file_name = "main.cc";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "1.2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		test =
		[
			"test/00/t0001a.sh",
			"test/00/t0002a.sh",
		];
	},
	{
		file_name = "test/00/t0002a.sh";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = test;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		architecture_times =
		[
			{
				variant = "unspecified";
				test_time = TIME;
				test_baseline_time = TIME;
			},
		];
	},
];
fubar
if test $? -ne 0 ; then cat test.out; no_result; fi
check_it $workproj/info/change/0/002.fs ok

#
# end development of the branch
#
activity="devlop end 977"
$bin/aegis -devend -lib $worklib -p foo -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# check the project file state
#
cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = config;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "main.cc";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		locked_by = 2;
		test =
		[
			"test/00/t0001a.sh",
		];
	},
	{
		file_name = "test/00/t0001a.sh";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = test;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		architecture_times =
		[
			{
				variant = "unspecified";
				test_time = TIME;
			},
		];
	},
	{
		file_name = "test/00/t0002a.sh";
		uuid = "UUID";
		action = transparent;
		usage = test;
		locked_by = 2;
		about_to_be_created_by = 2;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/trunk.fs ok

#
# review the branch and and start integrating it
#
activity="review pass 1081"
$bin/aegis -revpass -c 2 -p foo -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate begin 1084"
$bin/aegis -intbeg -c 2 -p foo -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate build 1087"
$bin/aegis -b -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate test 1090"
$bin/aegis -t -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
$bin/aegis -t -bl -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
$bin/aegis -t -reg -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# check the branch file state
#
cat > ok << 'fubar'
src =
[
	{
		file_name = "main.cc";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "1.2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		test =
		[
			"test/00/t0001a.sh",
			"test/00/t0002a.sh",
		];
	},
	{
		file_name = "test/00/t0002a.sh";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = test;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		architecture_times =
		[
			{
				variant = "unspecified";
				test_time = TIME;
				test_baseline_time = TIME;
			},
		];
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/002.fs ok

activity="integrate pass 1178"
$bin/aegis -intpass -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# check the contents of the branch file state
#
activity="check branch file state 1185"
cat > ok << fubar
src =
[
	{
		file_name = "main.cc";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "1.3";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = source;
		test =
		[
			"test/00/t0001a.sh",
			"test/00/t0002a.sh",
		];
	},
	{
		file_name = "test/00/t0002a.sh";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = test;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/002.fs ok

#
# the branch is in the 'completed' state, there shoud be no deveopment
# directory
#
if test -d $workproj/branch.2; then fail; fi

#
# check the contents of the project file state
#
activity="check trunk file state 1240"
cat > ok << fubar
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = config;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "main.cc";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.3";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.3";
			encoding = none;
		};
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		test =
		[
			"test/00/t0001a.sh",
			"test/00/t0002a.sh",
		];
	},
	{
		file_name = "test/00/t0001a.sh";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.1";
			encoding = none;
		};
		usage = test;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		architecture_times =
		[
			{
				variant = "unspecified";
				test_time = TIME;
			},
		];
	},
	{
		file_name = "test/00/t0002a.sh";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1.2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1.2";
			encoding = none;
		};
		usage = test;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		architecture_times =
		[
			{
				variant = "unspecified";
				test_time = TIME;
				test_baseline_time = TIME;
			},
		];
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/trunk.fs ok

#
# the things tested in this test, worked
#
pass
