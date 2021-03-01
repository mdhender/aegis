#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2003-2008 Peter Miller
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
if test $? -ne 0 ; then exit 2; fi

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

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		-e 's/uuid = ".*"/uuid = "UUID"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

no_result()
{
	set +x
	echo "NO RESULT for test of aemt functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of aemt functionality ($activity)" 1>&2
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
AEGIS_THROTTLE=-1
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
activity="working directory 119"
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
# make a new project
#
activity="new project 138"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 145"
cat > $tmp << 'end'
description = "A bogus project created to test the aemt functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 160"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# create a new change
#
activity="new change 173"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 191"
$bin/aegis -nf  $workchan/aegis.conf $workchan/bogus -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus << 'end'
the trunk version
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "echo no build required";
create_symlinks_before_build = true;
remove_symlinks_after_build = false;
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
patch_diff_command = "set +e; diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 224"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 231"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 238"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 245"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 252"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 259"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 266"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# create a branch
#
activity="new branch 275"
$bin/aegis -nbr -p foo 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=foo.2
export AEGIS_PROJECT

# --------------------------------------------------------------------------

#
# create a new change
#
activity="new change 287"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f $tmp -p foo.2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 3 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add files to the change
#
activity="copy file 305"
$bin/aegis -cp  $workchan/bogus -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus << 'end'
the branch version
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 317"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 324"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 331"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 338"
$bin/aegis -rpass -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 345"
$bin/aegis -ib 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 352"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate diff
#
activity="diff 359"
$bin/aegis -diff -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 366"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# create a new change
#
activity="new change 375"
cat > $tmp << 'end'
brief_description = "The third change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 4 -f $tmp -p foo.2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 4 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add files to the change
#
activity="make transparent 393"
$bin/aegis -mt $workchan/bogus -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check file contents 397"
cat > ok << 'end'
the trunk version
end
if test $? -ne 0 ; then no_result; fi

diff ok $workchan/bogus
if test $? -ne 0 ; then fail; fi

activity="check change file state 406"
cat > ok << 'end'
src =
[
	{
		file_name = "bogus";
		uuid = "UUID";
		action = transparent;
		edit_origin =
		{
			revision = "2";
			encoding = none;
		};
		usage = source;
	},
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.branch/0/004.fs

#
# remove the bogus file before the build,
# and a symlink to the correct place should be constructed
# during the build
#
rm -f $workchan/bogus
if test $? -ne 0 ; then no_result; fi

#
# build
#
activity="build 438"
$bin/aegis -build -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# now chek the bogus file again
# the symlink should point to the right place.
#
activity="check symlinks 446"
cat > ok << 'end'
the trunk version
end
if test $? -ne 0 ; then no_result; fi

diff ok $workchan/bogus
if test $? -ne 0 ; then fail; fi

activity="check change state 455"
cat > ok << 'end'
brief_description = "The third change";
description = "The third change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
state = being_developed;
given_test_exemption = true;
given_regression_test_exemption = true;
project_file_command_sync = 88;
build_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
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
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.branch/0/004

#
# diff
#
activity="diff 502"
$bin/aegis -diff -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check change state 506"
cat > ok << 'end'
brief_description = "The third change";
description = "The third change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
state = being_developed;
given_test_exemption = true;
given_regression_test_exemption = true;
project_file_command_sync = 88;
build_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
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
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.branch/0/004

activity="check change file state 550"
cat > ok << 'end'
src =
[
	{
		file_name = "bogus";
		uuid = "UUID";
		action = transparent;
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
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.branch/0/004.fs

#
# develop end
#
activity="develop end 586"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check change file state 590"
cat > ok << 'end'
src =
[
	{
		file_name = "bogus";
		uuid = "UUID";
		action = transparent;
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
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.branch/0/004.fs

activity="check change state 623"
cat > ok << 'end'
brief_description = "The third change";
description = "The third change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
state = being_reviewed;
given_test_exemption = true;
given_regression_test_exemption = true;
project_file_command_sync = 88;
build_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
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
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.branch/0/004

activity="check branch file state 672"
cat > ok << 'end'
src =
[
	{
		file_name = "bogus";
		uuid = "UUID";
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
		locked_by = 4;
	},
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.fs

#
# review pass
#
activity="review pass 714"
$bin/aegis --review-pass 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# integrate begin
#
activity="integrate begin 721"
$bin/aegis --integrate-begin 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# build
#
activity="build 728"
$bin/aegis -build -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# diff
#
activity="diff 735"
$bin/aegis -diff -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the integration
#
activity="integrate pass 742"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check change file state 746"
cat > ok << 'end'
src =
[
	{
		file_name = "bogus";
		uuid = "UUID";
		action = transparent;
		edit_origin =
		{
			revision = "2";
			encoding = none;
		};
		usage = source;
	},
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.branch/0/004.fs

activity="check change state 767"
cat > ok << 'end'
brief_description = "The third change";
description = "The third change";
cause = internal_bug;
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
given_test_exemption = true;
given_regression_test_exemption = true;
delta_number = 2;
delta_uuid = "UUID";
minimum_integration = true;
project_file_command_sync = 88;
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
uuid = "UUID";
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.branch/0/004

activity="check branch file state 829"
cat > ok << 'end'
src =
[
	{
		file_name = "bogus";
		uuid = "UUID";
		action = transparent;
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
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.fs

# ==========================================================================
#
# Now the tricky part: ending the branch.
#
AEGIS_PROJECT=foo
export AEGIS_PROJECT

#
# develop end
#
activity="develop end 865"
$bin/aegis -de 2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check change file state 869"
cat > ok << 'end'
src =
[
	{
		file_name = "bogus";
		uuid = "UUID";
		action = transparent;
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
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.fs

activity="check change state 895"
cat > ok << 'end'
brief_description = "A bogus project created to test the aemt functionality, branch 2.";
description = "A bogus project created to test the aemt functionality, branch 2.";
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
state = being_reviewed;
build_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
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
	{
		when = TIME;
		what = develop_end;
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
	default_test_exemption = true;
	default_test_regression_exemption = true;
	skip_unlucky = false;
	compress_database = false;
	develop_end_action = goto_being_reviewed;
	history =
	[
		{
			delta_number = 1;
			change_number = 3;
		},
		{
			delta_number = 2;
			change_number = 4;
		},
	];
	change =
	[
		3,
		4,
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
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002

# The transparent file should NOT be locked
# when its coming from a branch that it was transparent in.
activity="check trunk file state 995"
cat > ok << 'end'
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
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
		file_name = "bogus";
		uuid = "UUID";
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
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/trunk.fs

#
# review pass
#
activity="review pass 1052"
$bin/aegis --review-pass 2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# integrate begin
#
activity="integrate begin 1059"
$bin/aegis --integrate-begin 2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# build
#
activity="build 1066"
$bin/aegis -build -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the integration
#
activity="integrate pass 1073"
$bin/aegis -intpass -nl 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check change file state 1077"
cat > ok << 'end'
src =
[
	{
		file_name = "bogus";
		uuid = "UUID";
		action = transparent;
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
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002.fs

activity="check change state 1103"
cat > ok << 'end'
brief_description = "A bogus project created to test the aemt functionality, branch 2.";
description = "A bogus project created to test the aemt functionality, branch 2.";
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
delta_number = 2;
delta_uuid = "UUID";
minimum_integration = true;
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
uuid = "UUID";
branch =
{
	umask = 022;
	developer_may_review = true;
	developer_may_integrate = true;
	reviewer_may_integrate = true;
	developers_may_create_changes = false;
	default_test_exemption = true;
	default_test_regression_exemption = true;
	skip_unlucky = false;
	compress_database = false;
	develop_end_action = goto_being_reviewed;
	history =
	[
		{
			delta_number = 1;
			change_number = 3;
		},
		{
			delta_number = 2;
			change_number = 4;
		},
	];
	change =
	[
		3,
		4,
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
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/002

activity="check trunk file state 1210"
cat > ok << 'end'
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
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
		file_name = "bogus";
		uuid = "UUID";
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
		usage = source;
	},
];
end
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/trunk.fs

#
# the things tested in this test, worked
# can't speak for the rest of the code
#
pass
