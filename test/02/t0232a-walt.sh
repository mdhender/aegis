#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2005 Walter Franzini;
#	Copyright (C) 2006 Peter Miller
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
# MANIFEST: Test the aedist functionality
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
umask 022

LINES=24
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
	log_file_preference = never; \
	default_development_directory = \"$work\";"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

# This tells aeintegratq that it is being used by a test.
AEGIS_TEST_DIR=$work
export AEGIS_TEST_DIR

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
	echo "FAILED test of the aedist functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aedist functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 112"
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
	cat >> $work/c++ << fubar
#!/bin/sh
exec ${CXX-g++} \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx $work/c++
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi


#
# test the aedist functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH

#
# make a new project (proj1)
#

AEGIS_PROJECT=proj1 ; export AEGIS_PROJECT


activity="new project 177"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir ${workproj}_1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 184"
cat > tmp << 'end'
description = "A bogus project created to test the aemv/merge "
    "functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 201"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# make a new project (proj2)
#

AEGIS_PROJECT=proj2 ; export AEGIS_PROJECT


activity="new project 216"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir ${workproj}_2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 223"
cat > tmp << 'end'
description = "A bogus project created to test the aemv/merge "
    "functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 240"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Back to proj1
#
AEGIS_PROJECT=proj1
export AEGIS_PROJECT

#STEP 1:
#
# create a new change
#
activity="new change 258"
cat > tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 270"
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 277"
$bin/aegis -nf  $workchan/bogus1 -nl \
	--uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/bogus2 -nl \
	--uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/aegis.conf -nl \
	--uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus1 << 'end'
bogus1, line 1
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/bogus2 << 'end'
line one
line two
line three
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
merge_command = "exit 1 # $input $output $orig $most_recent";
patch_diff_command = "set +e; diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 321"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 328"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -change_attr --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd0 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 338"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate
#
activity="integrate 345"
$bin/aeintegratq -p $AEGIS_PROJECT -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#STEP2:
#
# create a new change
#
activity="new change 353"
cat > tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 365"
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify bogus1 369"
$bin/aegis -cp 2 $workchan/bogus1 -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat >> $workchan/bogus1 <<EOF
append some text
EOF
if test $? -ne 0; then no_result; fi

activity="modify bogus2 378"
$bin/aegis -cp 2 $workchan/bogus2 -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat >> $workchan/bogus2 <<EOF
append some text
EOF
if test $? -ne 0; then no_result; fi

activity="diff the change 387"
$bin/aegis -diff 2 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="build the change 391"
$bin/aegis -build 2 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 395"
$bin/aegis -dev_end 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="complete the change 399"
$bin/aeintegratq -p $AEGIS_PROJECT -c 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# end of step2
#

activity="send the change 407"
$bin/aedist -send -p $AEGIS_PROJECT -c 1 -o c11.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="send the change 411"
$bin/aedist -send -p $AEGIS_PROJECT -c 2 -o c12.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Switch to proj2
#
AEGIS_PROJECT=proj2
export AEGIS_PROJECT


#
# step3
#
activity="receive the change 426"
$bin/aedist -receive -p $AEGIS_PROJECT -f c11.ae -c 1 -no_trojan > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="complete the change 431"
$bin/aeintegratq -p $AEGIS_PROJECT -c 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
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
		file_name = "bogus1";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
		};
		usage = source;
	},
	{
		file_name = "bogus2";
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
EOF
check_it ok ${workproj}_2/info/change/0/001.fs

#
# step4
#
activity="receive the change 479"
$bin/aedist -receive -p $AEGIS_PROJECT -c 2 -f c12.ae -no_trojan > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="complete the change 483"
$bin/aeintegratq -p $AEGIS_PROJECT -c 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
src =
[
	{
		file_name = "bogus1";
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
	},
	{
		file_name = "bogus2";
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
	},
];
EOF
check_it ok ${workproj}_2/info/change/0/002.fs


#
# Step5
#
activity="new change 530"
cat > tmp << 'end'
brief_description = "The 3rd change (proj2)";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 542"
$bin/aegis -db 3 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify bogus1 546"
$bin/aegis -cp 3 $workchan/bogus1 -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat >> $workchan/bogus1 <<EOF
append some text
EOF
if test $? -ne 0; then no_result; fi

activity="modify bogus2 555"
$bin/aegis -cp 3 $workchan/bogus2 -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat >> $workchan/bogus2 <<EOF
append some text
EOF
if test $? -ne 0; then no_result; fi

activity="diff the change 564"
$bin/aegis -diff 3 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="build the change 568"
$bin/aegis -build 3 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 572"
$bin/aegis -dev_end 3 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="complete the change 576"
$bin/aeintegratq -p $AEGIS_PROJECT -c 3 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi


cat > ok <<EOF
src =
[
	{
		file_name = "bogus1";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "3";
			encoding = none;
		};
		edit_origin =
		{
			revision = "2";
			encoding = none;
		};
		usage = source;
	},
	{
		file_name = "bogus2";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "3";
			encoding = none;
		};
		edit_origin =
		{
			revision = "2";
			encoding = none;
		};
		usage = source;
	},
];
EOF
check_it ok ${workproj}_2/info/change/0/003.fs

activity="send the change 620"
$bin/aedist -send -p $AEGIS_PROJECT -c 3 -o c22.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Step6
#

#
# Switch back to proj1
#
AEGIS_PROJECT=proj1
export AEGIS_PROJECT

activity="receive the change 634"
$bin/aedist -receive -p $AEGIS_PROJECT -c 3 -f c22.ae \
    -dir $workchan -trojan > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

$bin/aegis -cp $workchan/aegis.conf > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

echo '/* append some text */' >> $workchan/aegis.conf
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "1";
			encoding = none;
		};
		usage = config;
	},
	{
		file_name = "bogus1";
		uuid = "UUID";
		action = modify;
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
	{
		file_name = "bogus2";
		uuid = "UUID";
		action = modify;
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
EOF
if test $? -ne 0; then no_result; fi

check_it ok ${workproj}_1/info/change/0/003.fs

activity="modify file 709"
cat >> $workchan/bogus2 <<EOF
append some text, again.
EOF
if test $? -ne 0; then no_result; fi

activity="diff the change 715"
$bin/aegis -diff 3 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="build the change 719"
$bin/aegis -build 3 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 723"
$bin/aegis -dev_end 3 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="complete the change 727"
$bin/aeintegratq -p $AEGIS_PROJECT -c 3 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
src =
[
	{
		file_name = "aegis.conf";
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
		usage = config;
	},
	{
		file_name = "bogus1";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "3";
			encoding = none;
		};
		edit_origin =
		{
			revision = "2";
			encoding = none;
		};
		usage = source;
	},
	{
		file_name = "bogus2";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "3";
			encoding = none;
		};
		edit_origin =
		{
			revision = "2";
			encoding = none;
		};
		usage = source;
	},
];
EOF
check_it ok ${workproj}_1/info/change/0/003.fs

$bin/aedist -send -p $AEGIS_PROJECT -c 3 -o c13.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Switch back to proj2
#
AEGIS_PROJECT=proj2
export AEGIS_PROJECT

activity="receive the change 795"
$bin/aedist -receive -p $AEGIS_PROJECT -c 4 -dir $workchan \
    -f c13.ae -trojan > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = modify;
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
		file_name = "bogus1";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "3";
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
	{
		file_name = "bogus2";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "3";
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
EOF

check_it ok ${workproj}_2/info/change/0/004.fs


activity="modify file 709"
cat >> $workchan/bogus2 <<EOF
append some text, again.
EOF
if test $? -ne 0; then no_result; fi

activity="diff the change 715"
$bin/aegis -diff 4 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="build the change 719"
$bin/aegis -build 4 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="develop end 723"
$bin/aegis -dev_end 4 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="complete the change 727"
$bin/aeintegratq -p $AEGIS_PROJECT -c 4 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="send the change"
$bin/aedist -send -p $AEGIS_PROJECT -c 4 -o c24.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Switch back to proj1
#

AEGIS_PROJECT=proj1
export AEGIS_PROJECT

activity="receive the change 795"
$bin/aedist -receive -p $AEGIS_PROJECT -c 4 -dir $workchan \
    -f c24.ae -trace receive -trojan > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "2";
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
		file_name = "bogus1";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "3";
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
	{
		file_name = "bogus2";
		uuid = "UUID";
		action = modify;
		edit_origin =
		{
			revision = "3";
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
EOF

check_it ok ${workproj}_1/info/change/0/004.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
