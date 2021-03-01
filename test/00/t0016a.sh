#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998 Peter Miller;
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
# MANIFEST: Test the aemv functionality
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

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/$USER/USER/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

no_result()
{
	set +x
	echo "NO RESULT for test of aemv command ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of aemv command ($activity)" 1>&2
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

activity="working directory 85"
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
# some variable to make things earier to read
#
worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
workchan3=$work/foo.chan3
tmp=$work/tmp

#
# make a new project
#	and check files it should have made
#
activity="new project 128"
$bin/aegis -newpro foo -vers "" -dir $workproj -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# change project attributes
#
activity="project attributes 135"
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
$bin/aegis -proatt -f $tmp -proj foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# create a new change
#	make sure it creates the files it should
#
activity="first new change 150"
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
$bin/aegis -new_change 1 -f $tmp -project foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# create a second change
#	make sure it creates the files it should
#
activity="second new change 163"
cat > $tmp << 'end'
brief_description = "This change was added to make the various listings \
much more interesting.";
cause = internal_bug;
end
$bin/aegis -new_change 2 -f $tmp -project foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add a new developer
#
activity="new developer 175"
$bin/aegis -newdev $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# begin development of a change
#	check it made the files it should
#
activity="develop begin 183"
$bin/aegis -devbeg 1 -p foo -dir $workchan -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add a new files to the change
#
activity="new file 190"
$bin/aegis -new_file $workchan/main.c $workchan/old \
	$workchan/config -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
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
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 223"
$bin/aegis -nt -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh

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
activity="deveopment build 250"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# difference the change
#
activity="development diff 257"
$bin/aegis -diff -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# test the change
#
activity="development test 264"
$bin/aegis -test -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
activity="develop end 271"
$bin/aegis -dev_end -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add a new reviewer
#
activity="new reviewer 278"
$bin/aegis -newrev $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the review
#
activity="review pass 285"
$bin/aegis -review_pass -chan 1 -proj foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add an integrator
#
activity="new integrator 292"
$bin/aegis -newint $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# start integrating
#
activity="integrate begin 299"
$bin/aegis -intbeg 1 -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# integrate build
#
activity="integration build 306"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integration test 309"
$bin/aegis -test -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the integration
#
activity="integrate pass 316"
$bin/aegis -intpass -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# start work on change 2
#
activity="develop begin 323"
$bin/aegis -devbeg 2 -p foo -v -dir $workchan -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# move a file in change 2
#
activity="move file 330"
$bin/aegis -mv $workchan/old $workchan/new -nl -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="copy file 333"
$bin/aegis -cp $workchan/main.c -nl -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

activity="verify change file state 337"
cat > ok << 'fubar'
src =
[
	{
		file_name = "main.c";
		action = modify;
		edit_number_origin = "1.1";
		usage = source;
	},
	{
		file_name = "new";
		action = create;
		usage = source;
		move = "old";
	},
	{
		file_name = "old";
		action = remove;
		edit_number_origin = "1.1";
		usage = source;
		move = "new";
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $workproj/info/change/0/002.fs

#
# build the change
# diff the change
#
activity="development build 371"
$bin/aegis -b -nl -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="development diff 374"
$bin/aegis -diff -nl -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# end development
# review pass
# start integrating
#
activity="develop end 383"
$bin/aegis -devend -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="review pass 386"
$bin/aegis -revpass -v -c 2 -p foo -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate begin 389"
$bin/aegis -intbeg -v -c 2 -p foo -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# build the integration
#
activity="integration build 396"
$bin/aegis -b -nl -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the integration
#	make sure it create the files, etc
#
activity="integrate pass 404"
$bin/aegis -intpass -nl -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

activity="verify project file state 408"
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
		test =
		[
			"test/00/t0001a.sh",
		];
	},
	{
		file_name = "main.c";
		action = create;
		edit_number = "1.2";
		edit_number_origin = "1.2";
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
		];
	},
	{
		file_name = "new";
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
		move = "old";
	},
	{
		file_name = "old";
		action = remove;
		edit_number = "1.1";
		edit_number_origin = "1.1";
		usage = source;
		move = "new";
		deleted_by = 2;
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
check_it ok $workproj/info/trunk.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
