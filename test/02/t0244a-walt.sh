#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2006, 2008 Walter Franzini
#       Copyright (C) 2007, 2008 Peter Miller
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
	echo "FAILED test of the 'aenpr -keep' ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the 'aenpr -keep' functionality " \
	    "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 116"
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
# test the aenpr -keep functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 163"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 170"
cat > tmp << 'end'
description = "A bogus project created to test the "
    "aenpr functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch 184"
$bin/aegis -p example -nbr 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="new branch 188"
$bin/aegis -p example.1 -nbr 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Create an archive of the repository.
#
activity="create a cpio archive 195"
$bin/test_cpio -create -file $work/example.cpio -cd $workproj \
    `(cd $workproj && find . -type f)` > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

mkdir $work/proj2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="extract the archive 203"
$bin/test_cpio -extract -file $work/example.cpio -cd $work/proj2 \
    > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="modify the trunk 208"
sed -e "s/$USER/${USER}not/g" < $work/proj2/info/trunk \
    > $work/proj2/info/trunk.new
if test $? -ne 0; then no_result; fi

mv $work/proj2/info/trunk.new $work/proj2/info/trunk > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="modify example.1 216"
sed -e "s/$USER/${USER}not/g" < $work/proj2/info/change/0/001 \
    > $work/proj2/info/change/0/001.new
if test $? -ne 0; then no_result; fi

mv $work/proj2/info/change/0/001.new $work/proj2/info/change/0/001 \
    > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="modify example.1.1 225"
sed -e "s/$USER/${USER}not/g" < $work/proj2/info/change/0/001.branch/0/001 \
    > $work/proj2/info/change/0/001.branch/0/001.new
if test $? -ne 0; then no_result; fi

mv $work/proj2/info/change/0/001.branch/0/001.new \
    $work/proj2/info/change/0/001.branch/0/001 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="create a new project 234"
$bin/aegis -npr -p example2 -keep -dir $work/proj2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > ok <<EOF
$USER
${USER}not
EOF
if test $? -ne 0; then no_result; fi

activity="check example2 244"
$bin/aegis -list admin -p example2 -ter > example2.adm
if test $? -ne 0; then no_result; fi

diff $work/ok $work/example2.adm
if test $? -ne 0; then fail; fi

activity="check example2.1 251"
$bin/aegis -list admin -p example2.1 -ter > example2.1.adm
if test $? -ne 0; then no_result; fi

diff $work/ok $work/example2.1.adm
if test $? -ne 0; then fail; fi

activity="check example2.1.1 258"
$bin/aegis -list admin -p example2.1.1 -ter > example2.1.1.adm
if test $? -ne 0; then no_result; fi

diff $work/ok $work/example2.1.1.adm
if test $? -ne 0; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
