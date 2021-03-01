#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991-2008 Peter Miller
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

activity="create working directory 41"

no_result()
{
	set +x
	echo NO RESULT for test of core functionality "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo FAILED test of core functionality "($activity)" 1>&2
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

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/delta[0-9][0-9]*/delta/' \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/delta_uuid = ".*"/delta_uuid = "UUID"/'\
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $1 > $work/sed.out
	if test $? -ne 0; then fail; fi
	diff $2 $work/sed.out
	if test $? -ne 0; then fail; fi
}

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

#
# make the directories
#
mkdir $work $work/lib
if test $? -ne 0 ; then exit 2; fi
chmod 777 $work/lib
if test $? -ne 0 ; then exit 2; fi
cd $work
if test $? -ne 0 ; then exit 2; fi

#
# If the C++ compiler is called something other than "c++", as
# discovered by the configure script, create a shell script called
# "c++" which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
#
if test "$CXX" != "c++"
then
	cat >> c++ << fubar
#!/bin/sh
exec ${CXX=g++} \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx c++
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# make a new project
#	and check files it should have made
#
activity="new project 151"
$bin/aegis -newpro foo -version "" -dir $workproj -lib $worklib
if test $? -ne 0 ; then fail; fi
$bin/aegis -newpro -list -unf -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi
grep '^foo ' < test.out > test.out2
if test $? -ne 0 ; then no_result; fi
cat > ok << 'fubar'
foo .../foo.proj The "foo" program.
fubar
if test $? -ne 0 ; then no_result; fi
check_it test.out2 ok

#
# check the contents of the various state files
#
cat > ok << 'fubar'
next_test_number = 1;
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/state ok

cat > ok << 'fubar'
brief_description = "The \"foo\" program.";
description = "The \"foo\" program.";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = false;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
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
	developer_may_review = false;
	developer_may_integrate = false;
	reviewer_may_integrate = false;
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
	reuse_change_numbers = true;
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/trunk ok

cat > ok << 'fubar'
src =
[
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/trunk.fs ok

#
# change project attributes
#
activity="project attributes 235"
$bin/aegis -proatt -list --proj=foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi
cat > ok << 'fubar'
description = "The \"foo\" program.";
developer_may_review = false;
developer_may_integrate = false;
reviewer_may_integrate = false;
developers_may_create_changes = false;
umask = 022;
default_test_exemption = false;
default_test_regression_exemption = true;
minimum_change_number = 10;
reuse_change_numbers = true;
minimum_branch_number = 1;
skip_unlucky = false;
compress_database = false;
develop_end_action = goto_being_reviewed;
protect_development_directory = false;
fubar
if test $? -ne 0 ; then no_result; fi
check_it test.out ok
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
minimum_change_number = 1;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -proatt -f $tmp -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# create a new change
#	make sure it creates the files it should
#
activity="new change 271"
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newcha -list -pro foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi
$bin/aegis -new_change -f $tmp -project foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# check the contents of the various state files
#
cat > ok << 'fubar'
brief_description = "This change is used to test the aegis functionality with respect to change descriptions.";
description = "This change is used to test the aegis functionality with respect to change descriptions.";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = true;
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
check_it $workproj/info/change/0/001 ok

cat > ok << 'fubar'
src =
[
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/001.fs ok

#
# create a second change
#	make sure it creates the files it should
#
cat > $tmp << 'end'
brief_description = "This change was added to make the various listings \
much more interesting.";
cause = internal_bug;
end
$bin/aegis -new_change -f $tmp -project foo -lib $worklib
if test $? -ne 0 ; then fail; fi
cat > ok << 'fubar'
1 awaiting_development This change is used to test the aegis functionality with respect to change descriptions.
2 awaiting_development This change was added to make the various listings much more interesting.
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_change -list -unf -pw=1000 -project foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi
check_it test.out ok

#
# add a new developer
#
activity="new developer 342"
$bin/aegis -newdev -list -pr foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi
$bin/aegis -newdev $USER -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
$bin/aegis -newdev -list -pr foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi

#
# begin development of a change
#	check it made the files it should
#
activity="develop begin 354"
$bin/aegis -devbeg -list -project foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi
$bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail; fi
if test ! -r $worklib/user/$USER ; then fail; fi
$bin/aegis -new_change -list -project foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi

#
# add a new files to the change
#
activity="new file 367"
$bin/aegis -new_file -list -lib $worklib -proJ foo > test.out
if test $? -ne 0 ; then fail; fi
$bin/aegis -new_file $workchan/main.cc -nl -lib $worklib -Pro foo \
	-uuid aabbcccc-cccc-4dde-8eee-eeefff000001
if test $? -ne 0 ; then fail; fi
$bin/aegis -new_file $workchan/aegis.conf -nl -lib $worklib -p foo \
	-uuid aabbcccc-cccc-4dde-8eee-eeefff000002
if test $? -ne 0 ; then fail; fi
cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000002";
		action = create;
		usage = config;
	},
	{
		file_name = "main.cc";
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000001";
		action = create;
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/001.fs ok

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
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 442"
$bin/aegis -nt -l -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
$bin/aegis -nt -lib $worklib -p foo -uuid aabbcccc-cccc-4dde-8eee-eeefff000004
if test $? -ne 0 ; then fail; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
#
# Project: "foo"
# Change: 1
#

no_result()
{
	echo WHIMPER 1>&2
	exit 2
}
fail()
{
	echo SHUZBUTT 1>&2
	exit 1
}
pass()
{
	exit 0
}
trap "no_result" 1 2 3 15

./foo
if test $? -ne 0; then fail; fi

# it probably worked
pass
end

#
# build the change
#
activity="build 480"
$bin/aegis -build -list -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out;fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail; fi

#
# difference the change
#
activity="diff 490"
$bin/aegis -diff -list -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -diff -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# test the change
#
activity="test 499"
$bin/aegis -test -l -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -test -lib $worklib -p foo -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi

$bin/aegis -ca -uuid aabbcccc-cccc-4dde-8eee-eeefff000003 \
	-c 1 -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# finish development of the change
#
activity="develop end 513"
$bin/aegis -dev_end -list -lib $worklib -p foo > test.out
if test $? -ne 0 ; then fail; fi
$bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# add a new reviewer
#
activity="new reviewer 525"
$bin/aegis -newrev -list -pr foo -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -newrev $USER -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
$bin/aegis -newrev -list -pr foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi

#
# fail the review
#
activity="review fail 536"
$bin/aegis -review_fail -list -p foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi
cat > $tmp << 'end'
This is a failed review comment.
end
$bin/aegis -review_fail -f $tmp -chan 1 -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# check the the file states are as they should be
#
activity="review fail ck.a 548"
cat > ok << 'fubar'
brief_description = "This change is used to test the aegis functionality with respect to change descriptions.";
description = "This change is used to test the aegis functionality with respect to change descriptions.";
cause = internal_enhancement;
test_exempt = false;
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
given_regression_test_exemption = true;
build_time = TIME;
test_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
		test_time = TIME;
	},
];
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
	{
		when = TIME;
		what = develop_end;
		who = "USER";
	},
	{
		when = TIME;
		what = review_fail;
		who = "USER";
		why = "This is a failed review comment.";
	},
];
uuid = "aabbcccc-cccc-4dde-8eee-eeefff000003";
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/001 ok
activity="review fail ck.b 606"
cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000002";
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
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000001";
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
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000004";
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

if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $workproj/info/trunk ; then fail ; fi
if test ! -r $workproj/info/trunk.fs ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# build the change again
# and difference
# and test
# end finish, again
#
activity="build 687"
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="diff 690"
$bin/aegis -diff -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="test 693"
$bin/aegis -test -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="develop end 696"
$bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# pass the review
#
activity="review pass 706"
$bin/aegis -review_pass -list -proj foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi
$bin/aegis -review_pass -chan 1 -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# add an integrator
#
activity="new integrator 718"
$bin/aegis -newint -list -pr foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi
$bin/aegis -newint $USER -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
$bin/aegis -newint -list -pr foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi

#
# start integrating
#
activity="integrate begin 729"
$bin/aegis -intbeg -l -p foo -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -intbeg 1 -p foo -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi
$bin/aegis -integrate_begin_undo -l -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# make sure -chdir works
#
activity="chdir 743"
$bin/aegis -cd -l -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
dir=`$bin/aegis -cd aegis.conf -bl -p foo -lib $worklib`
if test $? -ne 0 ; then fail; fi
if test $dir != $workproj/baseline/aegis.conf ; then fail; fi
dir=`$bin/aegis -cd -dd -p foo -c 1 aegis.conf -lib $worklib`
if test $? -ne 0 ; then fail; fi
if test $dir != $workchan/aegis.conf ; then fail; fi
dir=`$bin/aegis -cd -p foo -c 1 aegis.conf -lib $worklib`
if test $? -ne 0 ; then fail; fi
if test $dir != $workproj/delta*/aegis.conf ; then fail; fi

#
# integrate build
#
activity="integration build 759"
$bin/aegis -build -l -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -test -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test -f $workproj/delta*/aegis.log ; then fail; fi

#
# fail the integration
#
activity="integrate fail 772"
$bin/aegis -intfail -l -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
cat > $tmp << 'end'
This is a failed integration comment.
end
$bin/aegis -intfail -f $tmp -lib $worklib -p foo
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
activity="build 793"
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="diff 796"
$bin/aegis -diff -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="test 799"
$bin/aegis -test -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="develop end 802"
$bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
activity="review pass 805"
$bin/aegis -review_pass -chan 1 -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi
activity="integrate begin 808"
$bin/aegis -intbeg 1 -p foo -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# build and test the integration
#
activity="integrate build 818"
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -test -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi

#
# pass the integration
#
activity="integrate pass 828"
$bin/aegis -intpass -l -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -intpass -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
if test ! -r $workproj/info/change/0/001 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

diff $workproj/info/change/0/001.pfs $workproj/info/trunk.fs
test $? -eq 0 || fail

#
# check out the listings
#
activity="list 840"
$bin/aegis -list -list -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi
$bin/aegis -list chahist -c 1 -p foo -lib $worklib > test.out
if test $? -ne 0 ; then cat test.out; fail; fi
activity="check 845"
cat > ok << 'fubar'
brief_description = "This change is used to test the aegis functionality with respect to change descriptions.";
description = "This change is used to test the aegis functionality with respect to change descriptions.";
cause = internal_enhancement;
test_exempt = false;
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
given_regression_test_exemption = true;
delta_number = 1;
delta_uuid = "UUID";
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
		what = review_fail;
		who = "USER";
		why = "This is a failed review comment.";
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
		what = integrate_fail;
		who = "USER";
		why = "This is a failed integration comment.";
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
uuid = "aabbcccc-cccc-4dde-8eee-eeefff000003";
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/001 ok
activity="check 934"
cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000002";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		usage = config;
	},
	{
		file_name = "main.cc";
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000001";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
	},
	{
		file_name = "test/00/t0001a.sh";
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000004";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		usage = test;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
activity="list 974"
check_it $workproj/info/change/0/001.fs ok
activity="check 976"
cat > ok << 'fubar'
brief_description = "A bogus project created to test things.";
description = "The \"foo\" program.";
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
build_time = TIME;
test_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
		test_time = TIME;
	},
];
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
		2,
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
	minimum_change_number = 1;
	reuse_change_numbers = true;
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/trunk ok

#
# start work on change 2
#
activity="develop begin 1068"
$bin/aegis -devbeg 2 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then fail; fi
$bin/aegis -cp -l -unf -lib $worklib -p foo > test.out
if test $? -ne 0 ; then fail; fi
cat > ok << 'fubar'
config 1 aegis.conf
source 1 main.cc
test 1 test/00/t0001a.sh
fubar
if test $? -ne 0 ; then no_result; fi
check_it test.out ok

#
# copy a file into the change
#
activity="copy file 1084"
$bin/aegis -cp $workchan/main.cc -nl -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
if test ! -r $workproj/info/change/0/002 ; then fail ; fi
if test ! -r $workproj/info/state ; then fail ; fi
if test ! -r $worklib/user/$USER ; then fail ; fi

#
# change the file
#
cat > $workchan/main.cc << 'end'
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	if (argc != 1)
	{
		fprintf(stderr, "usage: %s\n", argv[0]);
		exit(1);
	}
	printf("hello, world\n");
	return 0;
}
end

#
# need another test
#
activity="new test 1114"
$bin/aegis -nt -lib $worklib -p foo -uuid aabbcccc-cccc-4dde-8eee-eeefff000005
if test $? -ne 0 ; then fail; fi
cat > $workchan/test/00/t0002a.sh << 'end'
#!/bin/sh
#
# Project: "foo"
# Change: 2
#

no_result()
{
	echo WHIMPER 1>&2
	exit 2
}
fail()
{
	echo SHUZBUTT 1>&2
	exit 1
}
pass()
{
	exit 0
}
trap "no_result" 1 2 3 15

./foo > /dev/null 2>&1
test $? -eq 0 || fail

# should complain
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
activity="build 1156"
$bin/aegis -b -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="diff 1159"
$bin/aegis -diff -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="test 1162"
$bin/aegis -test -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -test -bl -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
if test ! -r $workchan/main.cc,D ; then fail ; fi

#
# end development
# review pass
# start integrating
#
activity="devlop end 1174"
$bin/aegis -ca -uuid aabbcccc-cccc-4dde-8eee-eeefff000006 -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
$bin/aegis -devend -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi
activity="review pass 1179"
$bin/aegis -revpass -c 2 -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
activity="integrate begin 1182"
$bin/aegis -intbeg -c 2 -p foo -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
cat > ok << 'fubar'
brief_description = "This change was added to make the various listings much more interesting.";
description = "This change was added to make the various listings much more interesting.";
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
state = being_integrated;
given_regression_test_exemption = true;
delta_number = 2;
delta_uuid = "UUID";
project_file_command_sync = 1;
development_directory = ".../foo.chan";
integration_directory = "delta.002";
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
];
uuid = "aabbcccc-cccc-4dde-8eee-eeefff000006";
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/002 ok

#
# build the integration
# test the integration
# test the integration against the baseline
#
activity="integrate build 1245"
$bin/aegis -b -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="integrate test 1248"
$bin/aegis -t -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
$bin/aegis -t -bl -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# pass the integration
#	make sure it create the files, etc
#
activity="integrate pass 1258"
$bin/aegis -intpass -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

diff $workproj/info/change/0/002.pfs $workproj/info/trunk.fs
test $? -eq 0 || fail

# also, the 001.pfs file should still be there,
# and should now be different to the trunk file state
activity="project file state cache 1273"
diff $workproj/info/change/0/001.pfs $workproj/info/trunk.fs > LOG 2>&1
if test $? -ne 1
then
    echo "The files are supposed to be different"
    cat LOG
    fail
fi

cat > ok << 'fubar'
brief_description = "This change was added to make the various listings much more interesting.";
description = "This change was added to make the various listings much more interesting.";
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
state = completed;
given_regression_test_exemption = true;
delta_number = 2;
delta_uuid = "UUID";
project_file_command_sync = 1;
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
uuid = "aabbcccc-cccc-4dde-8eee-eeefff000006";
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/002 ok
activity="integrate pass 1319"
cat > ok << 'fubar'
src =
[
	{
		file_name = "main.cc";
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000001";
		action = modify;
		edit =
		{
			revision = "2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
	},
	{
		file_name = "test/00/t0002a.sh";
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000005";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		usage = test;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/change/0/002.fs ok
activity="integrate pass 1354"
cat > ok << 'fubar'
next_test_number = 3;
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/state ok
activity="integrate pass 1360"
cat > ok << 'fubar'
brief_description = "A bogus project created to test things.";
description = "The \"foo\" program.";
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
build_time = TIME;
test_time = TIME;
test_baseline_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
		test_time = TIME;
		test_baseline_time = TIME;
	},
];
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
		{
			delta_number = 2;
			change_number = 2;
		},
	];
	change =
	[
		1,
		2,
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
	minimum_change_number = 1;
	reuse_change_numbers = true;
	protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj/info/trunk ok
activity="integrate pass 1454"
cat > ok << 'fubar'
src =
[
	{
		file_name = "aegis.conf";
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000002";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1";
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
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000001";
		action = create;
		edit =
		{
			revision = "2";
			encoding = none;
		};
		edit_origin =
		{
			revision = "2";
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
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000004";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1";
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
		uuid = "aabbcccc-cccc-4dde-8eee-eeefff000005";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		edit_origin =
		{
			revision = "1";
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
activity="integrate pass 1570"
cat > ok << 'fubar'
own =
[
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $worklib/user/$USER ok
$bin/aegis -l projhist -unf -p foo -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

# should be no automatic logging
if test "`find $work -name 'aegis.log' -print`" != "" ; then fail; fi

#
# the things tested in this test, worked
#
pass
