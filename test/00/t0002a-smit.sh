#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 2007, 2008 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.
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
        echo "FAILED test of the aer getenv() functionality" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT when testing the aer getenv() functionality" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
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
# test the aer getenv() functionality
#
$bin/aegis -np foo -dir  $work/proj -lib $work/lib
if test $? -ne 0 ; then no_result; fi

cat > test.in << 'fubar'
columns(80);
auto s1;
auto s2;
s1=getenv("AEGIS_MESSAGE_LIBRARY");
s2=getenv("LANGUAGE");
print(s1);
print(s2);
fubar
test $? -eq 0 || no_result

cat > test.ok << fubar
$work/no-such-dir

fubar
test $? -eq 0 || no_result

$bin/aereport -unf -f test.in -o test.out
test $? -eq 0 || fail

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
