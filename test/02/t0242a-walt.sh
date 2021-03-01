#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2006-2008 Walter Franzini
#	Copyright (C) 2008 Peter Miller
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

USER=`whoami`

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
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

# This tells aeintegratq that it is being used by a test.
AEGIS_TEST_DIR=$work
export AEGIS_TEST_DIR

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

AEGIS_DATADIR=$here/lib
export AEGIS_DATADIR

#
# set the path, so that the aegis command that aepatch/aedist invokes
# is from the same test set as the aepatch/aedist command itself.
#
PATH=${bin}:$PATH
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
	echo "FAILED test of the timesafe quality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the timesafe quality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/uuid = ".*"/uuid = "UUID"/' \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff -U1000 $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

trap \"no_result\" 1 2 3 15

activity="create test directory 113"
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
# test the timesafe quality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=example ; export AEGIS_PROJECT

ulimit -t 10
#
# make a new project
#
activity="new project 161"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 168"
cat > tmp << 'end'
description = "A bogus project created to test the "
    "aeipass functionality.";
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
activity="staff 185"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 196"
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
activity="develop begin 208"
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 215"
$bin/aegis -nf  $workchan/bogus1 -nl \
	--uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf $workchan/config \
    -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -nf  -config $workchan/configs/build $workchan/configs/diff \
    $workchan/configs/history -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus1 << 'end'
bogus1, line 1
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/config << 'end'
configuration_directory = "configs";
end
if test $? -ne 0; then no_result; fi

cat > $workchan/configs/build <<'EOF'
build_command = "exit 0";
link_integration_directory = true;
EOF
if test $? -ne 0 ; then no_result; fi

cat > $workchan/configs/history <<'EOF'
history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;
EOF
if test $? -ne 0 ; then no_result; fi

cat > $workchan/configs/diff <<'EOF'
diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
merge_command = "exit 0 # $input $output $orig $most_recent";
patch_diff_command = "set +e; diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
EOF
if test $? -ne 0 ; then no_result; fi


#
# develop_end the change
#
activity="build 265"
$bin/aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate the change
#
activity="integrate begin 272"
$bin/aegis -ib -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate the change 276"
$bin/aefinish -p $AEGIS_PROJECT -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Create a new branch
#
activity="new branch 283"
$bin/aegis -nbr 5 -p $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=example.5
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 293"
cat > tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 302"
$bin/aegis -db 1 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="remove config 306"
$bin/aegis -rm 1 $workchan/config -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="finish 310"
$bin/aefinish 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 314"
$bin/aegis -ib -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate change 318"
$bin/aefinish -p $AEGIS_PROJECT -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#ulimit -t 15

$bin/test_cfn -c 1 -vp none > $work/example5c1
if test $? -ne 0; then no_result; fi

#
# Create a new change
#
cat > tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -db 2 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -nf 2 -config $workchan/aegis.config -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -cp 2 $workchan/configs -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat $workchan/configs/build >> $workchan/aegis.config
if test $? -ne 0 ; then no_result; fi

cat $workchan/configs/diff >> $workchan/aegis.config
if test $? -ne 0 ; then no_result; fi

cat $workchan/configs/history >> $workchan/aegis.config
if test $? -ne 0 ; then no_result; fi

cat > $workchan/configs/build <<EOF
EOF
if test $? -ne 0 ; then no_result; fi

cat > $workchan/configs/history <<EOF
EOF
if test $? -ne 0 ; then no_result; fi

cat > $workchan/configs/diff <<EOF
EOF
if test $? -ne 0 ; then no_result; fi

$bin/aefinish 2 -v > log 2>&1
if test $? -ne 0 ; then no_result; fi

activity="integrate begin 371"
$bin/aegis -ib -c 2 -v > log 2>&1
if test $? -ne 0 ; then no_result; fi

activity="integrate the change 375"
$bin/aefinish -p $AEGIS_PROJECT -c 2 > log 2>&1
if test $? -ne 0 ; then no_result; fi

#
# Create a new change.
#
activity="the third change 382"
cat > tmp << 'end'
brief_description = "The third change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -db 3 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -mv 3 $workchan/aegis.config $workchan/aegis.conf \
    -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -rm 3 $workchan/configs -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aefinish -c 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 404"
$bin/aegis -ib -c 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate the change 408"
$bin/aefinish -p $AEGIS_PROJECT -c 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="list change files for 3rd 412"
$bin/test_cfn -p $AEGIS_PROJECT -c 3 -vp simple > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="list change files for 3rd 416"
$bin/test_cfn -p $AEGIS_PROJECT -c 3 -vp none > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="list change files for 3rd 420"
$bin/test_cfn -p $AEGIS_PROJECT -c 3 -vp  extreme > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="list change details for 3rd 424"
$bin/aegis -list cd -p $AEGIS_PROJECT -c 3 > log 2>&1
if test $? -ne 0 ; then fail; fi

activity="list change details for 2nd 428"
$bin/aegis -list cd -p $AEGIS_PROJECT -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
