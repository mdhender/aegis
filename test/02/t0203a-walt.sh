#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2005-2008 Peter Miller
#	Copyright (C) 2004, 2005 Walter Franzini;
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
        echo $work
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

activity="create test directory 93"
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
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the aedist/aemv functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 149"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 156"
cat > tmp << 'end'
description = "A bogus project created to test the aedist/aemv "
    "functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 172"
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
activity="new change 185"
cat > tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


#
# add a new files to the change
#
activity="new files 204"
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

diff_command = "set +e; $diff $orig $i > $out; test $$? -le 1";
merge_command = "(diff3 -e $i $orig $mr | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $i > $out";
patch_diff_command = "set +e; $diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 249"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 256"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 263"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 270"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 277"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 284"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 291"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 298"
cat > tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 310"
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Use the second change to move bogus2 to bogus3 and bogus1 to bogus4
#
activity="move files 317"
$bin/aegis -c 2 -mv -baserel bogus2 bogus3 bogus1 bogus4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Modify bogus4
#
activity="modify bogus4 324"
cat > $workchan/bogus4 <<EOF
this file is now named bogus4
EOF
if test $? -ne 0 ; then no_result; fi

#
# difference the change
#
activity="aed 333"
$bin/aegis --diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# build the change
#
activity="build 340"
$bin/aegis -build 2 -nl -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Set the change uuid
#
activity="change uuid 347"
$bin/aegis -change-attr \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd0 -c 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="end the change 352"
$bin/aegis -de 2 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="send the outstanding change 356"
$bin/aedist -send -c 2 -ndh -out $work/c02dev.ae > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="review pass 360"
$bin/aegis -rpass 2 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

acctivity="integrate begin"
$bin/aegis -ibegin 2 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build the change 368"
$bin/aegis -build 2 -verb -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 372"
$bin/aegis -diff 2 -verb -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate the change 376"
$bin/aegis -ipass 2 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="send the completed change 380"
$bin/aedist -send -c 2 -ndh -out $work/c02.ae > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

#
# Prepare reference data
#
activity="header 387"
cat > $work/header.ok <<EOF
MIME-Version: 1.0
Content-Type: application/aegis-change-set
Content-Transfer-Encoding: base64
Subject: example - The second change
Content-Name: example.C002.ae
Content-Disposition: attachment; filename=example.C002.ae
EOF
if test $? -ne 0; then no_result; fi

activity="file-list 398"
cat > $work/file-list.ok <<EOF
.../etc/change-number
.../etc/change-set
.../etc/project-name
.../patch/bogus3
.../patch/bogus4
.../src/bogus3
.../src/bogus4
EOF
if test $? -ne 0; then no_result; fi

activity="project-name 410"
cat > $work/project-name.ok <<EOF
$AEGIS_PROJECT
EOF
if test $? -ne 0; then no_result; fi

activity="change-number 416"
cat > $work/change-number.ok <<EOF
2
EOF
if test $? -ne 0; then no_result; fi

activity="change-set 422"
cat > $work/change-set.ok <<EOF
brief_description = "The second change";
description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
state = awaiting_development;
uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd0";
src =
[
	{
		file_name = "bogus1";
		uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
		action = remove;
		usage = source;
		move = "bogus4";
	},
	{
		file_name = "bogus2";
		uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
		action = remove;
		usage = source;
		move = "bogus3";
	},
	{
		file_name = "bogus3";
		uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2";
		action = create;
		usage = source;
		move = "bogus2";
	},
	{
		file_name = "bogus4";
		uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
		action = create;
		usage = source;
		move = "bogus1";
	},
];
EOF
if test $? -ne 0; then no_result; fi

#
# Check the archive from the outstanding change
#
activity="archive header check 469"
head -6 $work/c02dev.ae > header.c02dev
diff -b $work/header.ok header.c02dev
if test $? -ne 0; then fail; fi

activity="output dir create 474"
mkdir $work/c02dev.d > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="archive extract 478"
$bin/test_cpio -extract -change-dir $work/c02dev.d \
    -f $work/c02dev.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Check the archive structure
#
activity="check the file list 486"
find $work/c02dev.d -type f -print | sed -e "s!^${work}/c02dev.d!...!g" \
    | sort > $work/file-list.c02dev
diff -b $work/file-list.ok $work/file-list.c02dev
if test $? -ne 0; then fail; fi

#
# Check the metadata
#
activity="project-name  495"
diff -b $work/project-name.ok $work/c02dev.d/etc/project-name
if test $? -ne 0; then fail; fi

activity="change-number 499"
diff -b $work/change-number.ok $work/c02dev.d/etc/change-number
if test $? -ne 0; then fail; fi

activity="change-set  503"
diff -b $work/change-set.ok $work/c02dev.d/etc/change-set
if test $? -ne 0; then fail; fi

#
# Add some check for the patch/ dir
#

#
# Check the src/ dir
#
activity="check the src/ dir content 514"
diff -b $work/c02dev.d/src/bogus3 $workproj/baseline/bogus3
if test $? -ne 0; then fail; fi
diff -b $work/c02dev.d/src/bogus4 $workproj/baseline/bogus4
if test $? -ne 0; then fail; fi

#
# Check the archive from the completed change
#
activity="archive header check 523"
head -6 $work/c02dev.ae > header.c02dev
diff -b $work/header.ok header.c02dev
if test $? -ne 0; then fail; fi

activity="output dir create 528"
mkdir $work/c02.d > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="extract the archive 532"
$bin/test_cpio -extract -change-dir $work/c02.d \
    -f $work/c02.ae > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Check the archive structure
#
activity="check the file list 540"
find $work/c02.d -type f -print | sed -e "s!^${work}/c02.d!...!g" \
    | sort > $work/file-list.c02
if test $? -ne 0; then no_result; fi
diff -b $work/file-list.ok $work/file-list.c02
if test $? -ne 0; then fail; fi

#
# Check the metadata
#
activity="etc/project-name 550"
diff -b $work/project-name.ok $work/c02.d/etc/project-name
if test $? -ne 0; then fail; fi

activity="etc/change-number  554"
diff -b $work/change-number.ok $work/c02.d/etc/change-number
if test $? -ne 0; then fail; fi

activity="etc/change-set 558"
diff -b $work/change-set.ok $work/c02.d/etc/change-set
if test $? -ne 0; then fail; fi

#
# Add some check against the patch/ dir
#
activity="check the patch/ dir 565"

#
# Check the src/ dir
#
activity="check the src/ dir content 570"
diff -b $work/c02.d/src/bogus3 $workproj/baseline/bogus3
if test $? -ne 0; then fail; fi
diff -b $work/c02.d/src/bogus4 $workproj/baseline/bogus4
if test $? -ne 0; then fail; fi

#
# Only definite negatives are possibe.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
