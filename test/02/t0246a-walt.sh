#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2006, 2007 Walter Franzini
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
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

worklib=$work/lib
workproj=$work/example.proj
workchan=$work/example.chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH

# This tells aeintegratq that it is being used by a test.
AEGIS_TEST_DIR=$work
export AEGIS_TEST_DIR

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

parch=
test "$1" != "" && parch="$1/"
bin="$here/${parch}bin"

if test "$EXEC_SEARCH_PATH" != ""
then
    tpath=
    hold="$IFS"
    IFS=":$IFS"
    for tpath2 in $EXEC_SEARCH_PATH
    do
	tpath=${tpath}${tpath2}/${parch}bin:
    done
    IFS="$hold"
    PATH=${tpath}${PATH}
else
    PATH=${bin}:${PATH}
fi
export PATH

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
trap \"no_result\" 1 2 3 15

activity="create test directory 120"
mkdir $work $worklib
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
# make a new project
#
AEGIS_PROJECT=example
export AEGIS_PROJECT
activity="new project 162"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -lib $worklib \
    -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 170"
cat > $tmp << 'end'
description = "A bogus project created to test the change_file_nth functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 186"
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
activity="new change 199"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 217"
$bin/aegis -nf  $workchan/aegis.conf \
	$workchan/bogus \
	$workchan/slightly-bogus \
	-nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

date > $workchan/slightly-bogus
if test $? -ne 0 ; then no_result; fi

cat > $workchan/bogus << 'end'
the trunk version
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
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
# End the first change.
#
activity="develop end 256"
$bin/aefinish > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Integrate the first change.
#
activity="integrate begin 263"
$bin/aegis -ib -c 1 -v > log 2>&1
if test $? -ne 0; then no_result; fi

activity="integratq 267"
$bin/aefinish -p $AEGIS_PROJECT -c 1 -v > log 2>&1
if test $? -ne 0; then no_result; fi

# --------------------------------------------------------------------------

#
# create a branch
#
activity="new branch 276"
$bin/aegis -nbr -p $AEGIS_PROJECT 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=example.2
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 286"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 2 -f $tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 2 -dir $workchan > log 2>&1
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
# end the change.
#
activity="finish 317"
$bin/aefinish > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Integrate the change
#
activity="integrate begin 324"
$bin/aegis -ib -c 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the change 328"
$bin/aefinish -p $AEGIS_PROJECT -c 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Generate the file list
#
activity="view_path first 335"
$bin/test_cfn -p $AEGIS_PROJECT -c 2 -vp first > c2_vp_first.out 2> log
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
source modify bogus
EOF
if test $? -ne 0; then no_result; fi

diff c2_vp_first.out ok
if test $? -ne 0; then fail; fi

#ulimit -t 20

activity="view_path none 347"
$bin/test_cfn -p $AEGIS_PROJECT -c 2 -vp none > c2_vp_none.out 2> log
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
config create aegis.conf
source modify bogus
source create slightly-bogus
EOF
if test $? -ne 0; then no_result; fi

diff c2_vp_none.out ok
if test $? -ne 0; then fail; fi

activity="view_path simple 361"
$bin/test_cfn -p $AEGIS_PROJECT -c 2 -vp simple > c2_vp_simple.out 2> log
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
config create aegis.conf
source modify bogus
source create slightly-bogus
EOF
if test $? -ne 0; then no_result; fi

diff c2_vp_simple.out ok
if test $? -ne 0; then fail; fi

activity="view_path extreme 375"
$bin/test_cfn -p $AEGIS_PROJECT -c 2 -vp extreme > c2_vp_extreme.out 2> log
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
config create aegis.conf
source modify bogus
source create slightly-bogus
EOF
if test $? -ne 0; then no_result; fi

diff c2_vp_extreme.out ok
if test $? -ne 0; then fail; fi

#
# create a new change in the trunk
#
AEGIS_PROJECT=example
export AEGIS_PROJECT

activity="new change 395"
cat > $tmp << 'end'
brief_description = "The third change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f $tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 3 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add files to the change
#
activity="remove 413"
$bin/aegis -rm $workchan/slightly-bogus -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 417"
$bin/aegis -nf $workchan/baz -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="finish the change 421"
$bin/aefinish -c 3 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Integrate the change.
#
activity="integrate begin 428"
$bin/aegis -ib -c 3 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the change 432"
$bin/aefinish -p $AEGIS_PROJECT -c 3 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi


#
# Generate the file list
#
AEGIS_PROJECT=example.2
export AEGIS_PROJECT

activity="view_path first 443"
$bin/test_cfn -p $AEGIS_PROJECT -c 2 -vp first > c2_vp2_first.out 2> log
if test $? -ne 0; then cat log; no_result; fi

diff c2_vp_first.out c2_vp2_first.out
if test $? -ne 0; then fail; fi

activity="view_path none 450"
$bin/test_cfn -p $AEGIS_PROJECT -c 2 -vp none \
    > c2_vp2_none.out 2> log
if test $? -ne 0; then cat log; no_result; fi

diff c2_vp_none.out c2_vp2_none.out
if test $? -ne 0; then cat log; fail; fi

activity="view_path simple 458"
$bin/test_cfn -p $AEGIS_PROJECT -c 2 -vp simple > c2_vp2_simple.out 2> log
if test $? -ne 0; then cat log; no_result; fi

diff c2_vp_simple.out c2_vp2_simple.out
if test $? -ne 0; then fail; fi

activity="view_path extreme 465"
$bin/test_cfn -p $AEGIS_PROJECT -c 2 -vp extreme > c2_vp2_extreme.out 2> log
if test $? -ne 0; then cat log; no_result; fi

diff c2_vp_extreme.out c2_vp2_extreme.out
if test $? -ne 0; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
