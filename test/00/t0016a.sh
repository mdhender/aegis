#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1993-1998, 2000, 2001, 2004-2008 Peter Miller
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

LINES=25
export LINES
COLS=80
export COLS

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
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

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

activity="working directory 98"
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
# If the C++ compiler is called something other than "c++", as
# discovered by the configure script, create a shell script called
# "c++" which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
#
if test "$CXX" != "c++"
then
	cat >> c++ << fubar
#!/bin/sh
exec ${CXX-g++} \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx c++
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
activity="new project 146"
$bin/aegis -newpro foo -vers "" -dir $workproj -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# change project attributes
#
activity="project attributes 153"
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
activity="first new change 168"
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
activity="second new change 181"
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
activity="new developer 193"
$bin/aegis -newdev $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# begin development of a change
#	check it made the files it should
#
activity="develop begin 201"
$bin/aegis -devbeg 1 -p foo -dir $workchan -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add a new files to the change
#
activity="new file 208"
$bin/aegis -new_file $workchan/main.cc $workchan/old1 $workchan/old2 \
	$workchan/aegis.conf -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
cat > $workchan/main.cc << 'end'
int
main(int argc, char **argv)
{
	return 0;
}
end
cat > $workchan/aegis.conf << 'end'
build_command = "rm -f foo; c++ -o foo -D'VERSION=\"$version\"' main.cc";
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
posix_filename_charset = true;
end
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 243"
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
activity="deveopment build 275"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# difference the change
#
activity="development diff 282"
$bin/aegis -diff -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# test the change
#
activity="development test 289"
$bin/aegis -test -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
activity="develop end 296"
$bin/aegis -dev_end -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add a new reviewer
#
activity="new reviewer 303"
$bin/aegis -newrev $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the review
#
activity="review pass 310"
$bin/aegis -review_pass -chan 1 -proj foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# add an integrator
#
activity="new integrator 317"
$bin/aegis -newint $USER -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# start integrating
#
activity="integrate begin 324"
$bin/aegis -intbeg 1 -p foo -v -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# integrate build
#
activity="integration build 331"
$bin/aegis -build -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integration test 334"
$bin/aegis -test -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the integration
#
activity="integrate pass 341"
$bin/aegis -intpass -nl -v -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# start work on change 2
#
workchan=$work/foo.chan.2
activity="develop begin 348"
$bin/aegis -devbeg 2 -p foo -v -dir $workchan -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# test aemv vs. bad destination file name.  The aemv command should
# fail.
#
activity="test aemv vs. bad file name 356"
$bin/aegis -mv $workchan/old1 "$workchan/bad:name" \
    -nl -v -lib $worklib -c 2 -p foo > test.out.raw 2>&1
if test $? -eq 0; then cat test.out; fail; fi

sed -e '/warning: test mode/d' -e '/waiting for lock/d' \
	< test.out.raw > test.out
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# we also check aemv fails for the expected error
#
cat > expected-err <<EOF
aegis: project "foo": change 2: file name "bad:name" contains illegal
	characters
EOF
diff expected-err test.out > test.diff 2>&1
if test $? -ne 0 ; then cat test.diff; no_result; fi

#
# move a file in change 2
#
activity="move file 377"
$bin/aegis -mv $workchan/old1 $workchan/new1 \
    $workchan/old2 $workchan/new2 \
    -nl -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi
activity="copy file 382"
$bin/aegis -cp $workchan/main.cc -nl -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

activity="verify change file state 386"
cat > ok << 'fubar'
src =
[
	{
		file_name = "main.cc";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
	},
	{
		file_name = "new1";
		uuid = "UUID";
		action = create;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
		move = "old1";
	},
	{
		file_name = "new2";
		uuid = "UUID";
		action = create;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
		move = "old2";
	},
	{
		file_name = "old1";
		uuid = "UUID";
		action = remove;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
		move = "new1";
	},
	{
		file_name = "old2";
		uuid = "UUID";
		action = remove;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
		move = "new2";
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $workproj/info/change/0/002.fs

#
# build the change
# diff the change
#
activity="development build 458"
$bin/aegis -b -nl -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="development diff 461"
$bin/aegis -diff -nl -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# end development
# review pass
# start integrating
#
activity="develop end 470"
$bin/aegis -devend -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="review pass 473"
$bin/aegis -revpass -v -c 2 -p foo -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate begin 476"
$bin/aegis -intbeg -v -c 2 -p foo -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# build the integration
#
activity="integration build 483"
$bin/aegis -b -nl -v -lib $worklib -c 2 -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the integration
#	make sure it create the files, etc
#
activity="integrate pass 491"
$bin/aegis -intpass -nl -lib $worklib -c 2 -p foo > test.out 2>test.log
if test $? -ne 0 ; then cat test.out test.log; no_result; fi

activity="verify project file state 495"
cat > ok << 'fubar'
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
		file_name = "main.cc";
		uuid = "UUID";
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
		];
	},
	{
		file_name = "new1";
		uuid = "UUID";
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
		move = "old1";
	},
	{
		file_name = "new2";
		uuid = "UUID";
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
		move = "old2";
	},
	{
		file_name = "old1";
		uuid = "UUID";
		action = remove;
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
		move = "new1";
		deleted_by = 2;
		test =
		[
			"test/00/t0001a.sh",
		];
	},
	{
		file_name = "old2";
		uuid = "UUID";
		action = remove;
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
		move = "new2";
		deleted_by = 2;
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
];
fubar
check_it ok $workproj/info/trunk.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
