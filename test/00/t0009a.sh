#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999 Peter Miller;
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
# MANIFEST: Test -New_ReLeaSe functionality.
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
unset LINES
unset COLS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

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

work=${AEGIS_TMP:-/tmp}/$$

here=`pwd`
if test $? -ne 0; then exit 2; fi

bin=$here/${1-.}/bin

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
	echo "FAILED test of -New_ReLeaSe functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of -New_ReLeaSe functionality ($activity)" 1>&2
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
		-e "s/$USER/USER/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $1 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $2 $work/sed.out
	if test $? -ne 0; then fail; fi
}

#
# some variable to make things earier to read
#
worklib=$work/lib
workproj=$work/foo.proj
workproj2=$work/bar.proj
workchan=$work/foo.chan
tmp=$work/tmp

#
# make the directories
#
activity="create working directory 99"
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
	cat >> cc << fubar
#!/bin/sh
exec $CC \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx cc
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# make a new project
#	and check files it should have made
#
activity="create new project 153"
$bin/aegis -newpro foo -version "" -dir $workproj -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# change project attributes
#
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
$bin/aegis -proatt -f $tmp -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# create a new change
#
activity="create new change 172"
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
$bin/aegis -new_change 1 -f $tmp -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add a new developer
#
activity="add staff 184"
$bin/aegis -newdev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# begin development of a change
#
$bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add a new files to the change
#
activity="add files to change 197"
$bin/aegis -new_file $workchan/main.c -nl -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_file $workchan/config -nl -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
cat > $workchan/main.c << 'end'
void
main()
{
	exit(0);
}
end
if test $? -ne 0 ; then no_result; fi
cat > $workchan/config << 'end'
build_command = "rm -f foo; cc -o foo -D'VERSION=\"$v\"' main.c";
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
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 233"
$bin/aegis -nt -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
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
q=$?

# check for signals
if test $q -ge 128 
then
	no_result
fi

# should not complain
if test $q -ne 0 
then
	fail
fi

# it probably worked
pass
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 272"
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# difference the change
#
activity="diff 279"
$bin/aegis -diff -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# test the change
#
activity="test 286"
$bin/aegis -test -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
activity="develop end 293"
$bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi

#
# add a new reviewer
#
activity="new reviewer 300"
$bin/aegis -newrev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# pass the review
#
activity="review pass 307"
$bin/aegis -review_pass -chan 1 -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add an integrator
#
activity="new integrator 314"
$bin/aegis -newint $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# start integrating
#
activity="integrate begin 321"
$bin/aegis -intbeg 1 -p foo -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="integrate build 328"
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate test 331"
$bin/aegis -test -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the integration
#
activity="integrate pass 338"
$bin/aegis -intpass -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# make it look like an old-style project
#
activity="fake old style project 345"
cat >> $workproj/info/state << 'fubar'
version_major = 1;
version_minor = 0;
fubar
if test $? -ne 0 ; then no_result; fi

#
# create the new release
#
activity="new release 355"
$bin/aegis -nrls -l -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
$bin/aegis -nrls foo bar -dir $workproj2 -lib $worklib -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# check that files look as they should
#
activity="check change 1.1.10 state 364"
cat > ok << 'fubar'
brief_description = "New release derived from foo.";
description = "New release derived from foo.";
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
state = completed;
delta_number = 1;
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
	{
		when = TIME;
		what = develop_end;
		who = "USER";
	},
	{
		when = TIME;
		what = review_pass;
		who = "USER";
	},
	{
		when = TIME;
		what = integrate_begin;
		who = "USER";
	},
	{
		when = TIME;
		what = integrate_pass;
		who = "USER";
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001.branch/0/010 ok

activity="check change 1.1.10 file state 419"
cat > ok << 'fubar'
src =
[
	{
		file_name = "config";
		action = create;
		edit_number = "1.1";
		usage = source;
	},
	{
		file_name = "main.c";
		action = create;
		edit_number = "1.1";
		usage = source;
	},
	{
		file_name = "test/00/t0001a.sh";
		action = create;
		edit_number = "1.1";
		usage = test;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001.branch/0/010.fs ok

activity="check branch 1.1 state 446"
cat > ok << 'fubar'
brief_description = "A bogus project created to test things, branch 1.1.";
description = "A bogus project created to test things, branch 1.1.";
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
development_directory = "branch.1/branch.1";
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
	skip_unlucky = false;
	compress_database = false;
	history =
	[
		{
			delta_number = 1;
			change_number = 10;
		},
	];
	change =
	[
		10,
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
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001 ok

activity="check branch 1.1 file state 518"
cat > ok << 'fubar'
src =
[
	{
		file_name = "config";
		action = create;
		edit_number = "1.1";
		edit_number_origin = "1.1";
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
		];
	},
	{
		file_name = "main.c";
		action = create;
		edit_number = "1.1";
		edit_number_origin = "1.1";
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
		];
	},
	{
		file_name = "test/00/t0001a.sh";
		action = create;
		edit_number = "1.1";
		edit_number_origin = "1.1";
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
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001.fs ok

activity="check branch 1 state 589"
cat > ok << 'fubar'
brief_description = "A bogus project created to test things, branch 1.";
description = "A bogus project created to test things, branch 1.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
state = being_developed;
development_directory = "branch.1";
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
	skip_unlucky = false;
	compress_database = false;
	change =
	[
		1,
	];
	sub_branch =
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
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001 ok

activity="check branch 1 file state 654"
cat > ok << 'fubar'
src =
[
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.fs ok

activity="check project state 663"
cat > ok << 'fubar'
next_test_number = 2;
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/state ok

activity="check trunk state 670"
cat > ok << 'fubar'
brief_description = "A bogus project created to test things.";
description = "A bogus project created to test things.";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = false;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
version_previous = "1.0.D001";
state = being_developed;
development_directory = ".";
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
	skip_unlucky = false;
	compress_database = false;
	change =
	[
		1,
	];
	sub_branch =
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
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/trunk ok

activity="check trunk file state 736"
cat > ok << 'fubar'
src =
[
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/trunk.fs ok

#
# create a second change
#	make sure it creates the files it should
#
activity="new change 749"
cat > $tmp << 'end'
brief_description = "Second change of second project";
cause = internal_enhancement;
end
$bin/aegis -new_change 2 -f $tmp -project bar.1.1 -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# start work on change 2 of bar.1.1
#
activity="develop begin 760"
$bin/aegis -devbeg 2 -p bar.1.1 -dir $workchan -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# copy a file into the change
#
activity="copy file 767"
$bin/aegis -cp $workchan/main.c -nl -lib $worklib -p bar.1.1
if test $? -ne 0 ; then no_result; fi

#
# check file contents
#
activity="check file contents 774"
cat > ok << 'fubar'
brief_description = "Second change of second project";
description = "Second change of second project";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = false;
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
project_file_command_sync = 8410;
development_directory = ".../foo.chan";
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
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001.branch/0/002 ok

activity="check file contents 809"
cat > ok << 'fubar'
src =
[
	{
		file_name = "main.c";
		action = modify;
		edit_number_origin = "1.1";
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001.branch/0/002.fs ok

activity="check file contents 825"
cat > ok << 'fubar'
brief_description = "A bogus project created to test things, branch 1.1.";
description = "A bogus project created to test things, branch 1.1.";
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
development_directory = "branch.1/branch.1";
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
	skip_unlucky = false;
	compress_database = false;
	history =
	[
		{
			delta_number = 1;
			change_number = 10;
		},
	];
	change =
	[
		2,
		10,
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
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001 ok

#
# the things tested in this test, worked
#
pass
