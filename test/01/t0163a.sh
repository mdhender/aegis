#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2002, 2004-2008 Peter Miller
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
PAGER=cat
export PAGER
AEGIS_FLAGS="delete_file_preference = no_keep; \
	lock_wait_preference = always; \
	diff_preference = automatic_merge; \
	pager_preference = never; \
	persevere_preference = all; \
	log_file_preference = never; \
	default_project_directory = \"$work\"; \
	default_development_directory = \"$work\";"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin=$here/${1-.}/bin

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
	echo "NO RESULT for test of the executable source file functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of the executable source file functionality ($activity)" 1>&2
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
trap \"no_result\" 1 2 3 15

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

AEGIS_PROJECT=test
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

#
# test the executable source file functionality
#
activity="new project 122"
$bin/aegis -npr test -version '' -v -dir $work/test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 126"
cat > paf << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 137"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 145"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 155"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 159"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/fred \
	$work/test.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "exit 0";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
development_directory_style =
{
    source_file_copy = true;
};
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

chmod a+rx $work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="build 193"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 197"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 201"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 205"
$bin/aegis -rpass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 209"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check integration file mode 213"
test -x $work/test/delta*/barney || fail

activity="integrate build 216"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 220"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 224"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# ------------------------------------------------------------------------------

activity="check project file state 230"
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
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "barney";
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
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		executable = true;
	},
	{
		file_name = "fred";
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
check_it ok $work/test/info/trunk.fs

#
# The executable field is NOT set in the change, because it is a property
# set by integrate pass.  Until then, the file mode is the authority.
#
activity="check change file state 328"
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
		usage = config;
	},
	{
		file_name = "barney";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
		executable = true;
	},
	{
		file_name = "fred";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/010.fs

# ------------------------------------------------------------------------------

#
# Create a new change, and make sure when we copy the file that it is
# also executable.
#
activity="new change 377"
cat > caf << 'fubar'
brief_description = "the second change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 11 -f caf -v -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 386"
$bin/aegis -db 11 -dir $work/c11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="testing barney's mode 390"
test -x $work/c11/barney || fail

activity="copy file 393"
$bin/aegis -cp $work/c11/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check executability 397"
test -x $work/c11/barney || fail

$bin/aegis -cp $work/c11/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat >> $work/c11/fred <<EOF
bogus text
EOF
chmod a+x $work/c11/fred
if test $? -ne 0 ; then no_result; fi

# ------------------------------------------------------------------------------

#
# The executable field is NOT set in the change, because it is a property
# set by integrate pass.  Until then, the file mode is the authority.
#
activity="check project file state 415"
cat > ok << 'fubar'
src =
[
	{
		file_name = "barney";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
		executable = true;
	},
	{
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
	},
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it ok $work/test/info/change/0/011.fs

activity="build 446"
$bin/aegis -build -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 450"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

acctivity="dev end"
$bin/aegis -dev_end -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 458"
$bin/aegis -rev_pass 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="ibegin 462"
$bin/aegis -ibegin 11 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

deltadir=`$bin/aegis -cd`
if test $? -ne 0 ; then no_result; fi

activity="test executable mode fred 469"
test -x $deltadir/fred || fail
activity="test executable mode barney 471"
test -x $deltadir/barney || fail

activity="build 474"
$bin/aegis -build -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 478"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check barney permissions 482"
test -x $deltadir/barney || fail
activity="check fred permissions 484"
test -x $deltadir/fred || fail
activity="integrate pass 486"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check project file state 490"
cat > ok << 'fubar'
src =
[
	{
		file_name = "barney";
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
		executable = true;
	},
	{
		file_name = "fred";
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
		executable = true;
	},
];
fubar
check_it ok $work/test/info/change/0/011.fs

# ------------------------------------------------------------------------------

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
