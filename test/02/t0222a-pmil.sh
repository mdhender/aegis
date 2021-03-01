#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2006-2008, 2012 Peter Miller
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program. If not, see
#       <http://www.gnu.org/licenses/>.
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
umask 022

EDITOR=true
export EDITOR

#
# Make sure Subversion is installed on this machine.
# If it is not, give an automatic pass.
#
if svn --version > /dev/null 2>&1
then
    :
else
    echo ''
    echo '      You do not have Subversion installed on this machine.'
    echo '      This test is assumed to pass by default.'
    echo ''
    exit 0
fi

svnversion=`svn --version |
awk 'NR==1{split($3,a,"[.]");printf("%02d.%02d.%02d\n",a[1],a[2],a[3]);}'`
if expr "$svnversion" "<" "01.01.00" > /dev/null 2>&1
then
    echo ''
    echo '      You do not have Subversion 1.1 or later installed on   '
    echo '      this machine.  This test is assumed to pass by default.'
    echo ''
    exit 0
fi

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
        echo "FAILED test of the ae-repo-ci svn functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT when testing the ae-repo-ci svn functionality" \
            "($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 144"
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

LANG=C
export LANG

#
# test the ae-repo-ci functionality
#
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="new project 169"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
        -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="project attributes 177"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 190"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 198"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 208"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 212"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/fred/wilma \
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

diff_command = "exit 0";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
        echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/fred/wilma
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="develop end 239"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 243"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 247"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------
#
# Now we have to create the svn repository
#
activity="svnadmin create 255"
svnadmin create --fs-type=fsfs $work/svnroot
if test $? -ne 0 ; then no_result; fi

svn mkdir -mtesting file://$work/svnroot/$AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------
#
# Now we can test the ae-repo-ci command, to commit the Aegis change into svn.
#
activity="ae-repo-ci one 266"
$bin/ae-repo-ci --repository=svn --module file://$work/svnroot/$AEGIS_PROJECT \
    -c 10 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

test -f $work/svnroot/db/revs/2 -o -f $work/svnroot/db/revs/0/2 || fail

# --------------------------------------------------------------------------

#
# second change
#
activity="new change 278"
cat > caf << 'fubar'
brief_description = "the second change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 287"
$bin/aegis -db 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="copy file 291"
$bin/aegis -cp $work/test.C002/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo second > $work/test.C002/barney
if test $? -ne 0 ; then no_result; fi

activity="copy file 298"
$bin/aegis -cp $work/test.C002/fred/wilma -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo third > $work/test.C002/fred/wilma
if test $? -ne 0 ; then no_result; fi

activity="develop end 305"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 309"
$bin/aegis -ib 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 313"
$bin/aegis -ipass -c 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# ------------------------------------------------------------------------
#
# third change
# (rename and remove)
#
activity="new change 322"
cat > caf << 'fubar'
brief_description = "the third change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 331"
$bin/aegis -db 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="move file 335"
$bin/aegis -mv $work/test.C003/barney $work/test.C003/rubble -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo third > $work/test.C003/rubble
if test $? -ne 0 ; then no_result; fi

activity="remove file 342"
$bin/aegis -rm $work/test.C003/fred/wilma -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 346"
$bin/aegis -de 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 350"
$bin/aegis -ib 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 354"
$bin/aegis -ipass -c 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# ------------------------------------------------------------------------
#
# run the ae-repo-ci command again
#
activity="ae-repo-ci three 362"
$bin/ae-repo-ci --repository=svn --module file://$work/svnroot/$AEGIS_PROJECT \
    -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

test -f $work/svnroot/db/revs/3 -o -f $work/svnroot/db/revs/0/3 || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
