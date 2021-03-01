#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2006-2008 Peter Miller
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

#
# Make sure cvs is installed on this machine.
# If it is not, give an automatic pass.
#
if cvs --version > /dev/null 2>&1
then
    :
else
    echo ''
    echo '	You do not have CVS installed on this machine.'
    echo '	This test is assumed to pass by default.'
    echo ''
    exit 0
fi

CVSversion=`cvs --version |
awk 'NR==2{split($5,a,"[.]");printf("%02d.%02d.%02d\n",a[1],a[2],a[3]);}'`
if expr "$CVSversion" "<" "01.11.00" > /dev/null 2>&1
then
    echo ''
    echo '	You do not have CVS 1.11 or later installed on this machine.'
    echo '	This test is assumed to pass by default.'
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
	echo "FAILED test of the ae-repo-ci functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the ae-repo-ci functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 143"
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
# test the ae-repo-ci functionality
#
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="new project 165"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="project attributes 173"
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

activity="staff 186"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 194"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 204"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 208"
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

activity="develop end 235"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 239"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 243"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------
#
# Now we have to create the CVS repository
#
activity="cvs init 251"
mkdir $work/cvsroot
if test $? -ne 0 ; then no_result; fi

cvs -d $work/cvsroot init
if test $? -ne 0 ; then no_result; fi

#
# FIXME:  Is this a hack?  I can't work out how you are supposed to
# create a module's directory, but give it no initial contents, via
# the cvs command line interface.
#
# Module name == $AEGIS_PROJECT
#
mkdir $work/cvsroot/$AEGIS_PROJECT
if test $? -ne 0 ; then no_result; fi

CVSROOT=$work/cvsroot
export CVSROOT

# --------------------------------------------------------------------------
#
# Now we can test the ae-repo-ci command, to commit the Aegis change into CVS.
#
activity="ae-repo-ci one 274"
$bin/ae-repo-ci --repository=cvs -c 10 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

test -f $work/cvsroot/$AEGIS_PROJECT/aegis.conf,v || fail
test -f $work/cvsroot/$AEGIS_PROJECT/barney,v || fail
test -f $work/cvsroot/$AEGIS_PROJECT/fred/wilma,v || fail

# --------------------------------------------------------------------------

#
# second change
#
activity="new change 287"
cat > caf << 'fubar'
brief_description = "the second change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 296"
$bin/aegis -db 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="copy file 300"
$bin/aegis -cp $work/test.C002/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo second > $work/test.C002/barney
if test $? -ne 0 ; then no_result; fi

activity="copy file 307"
$bin/aegis -cp $work/test.C002/fred/wilma -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo third > $work/test.C002/fred/wilma
if test $? -ne 0 ; then no_result; fi

activity="develop end 314"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 318"
$bin/aegis -ib 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 322"
$bin/aegis -ipass -c 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# ------------------------------------------------------------------------
#
# run the ae-repo-ci command again
#
activity="ae-repo-ci two 330"
$bin/ae-repo-ci --repository=cvs -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

test -f $work/cvsroot/$AEGIS_PROJECT/aegis.conf,v || fail
test -f $work/cvsroot/$AEGIS_PROJECT/barney,v || fail
test -f $work/cvsroot/$AEGIS_PROJECT/fred/wilma,v || fail

# --------------------------------------------------------------------------

#
# third change
# (rename and remove)
#
activity="new change 344"
cat > caf << 'fubar'
brief_description = "the third change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 353"
$bin/aegis -db 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="move file 357"
$bin/aegis -mv $work/test.C003/barney $work/test.C003/rubble -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo third > $work/test.C003/rubble
if test $? -ne 0 ; then no_result; fi

activity="remove file 364"
$bin/aegis -rm $work/test.C003/fred/wilma -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 368"
$bin/aegis -de 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 372"
$bin/aegis -ib 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 376"
$bin/aegis -ipass -c 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# ------------------------------------------------------------------------
#
# run the ae-repo-ci command again
#
activity="ae-repo-ci three 384"
$bin/ae-repo-ci --repository=cvs -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

test -f $work/cvsroot/$AEGIS_PROJECT/rubble,v || fail
test -f $work/cvsroot/$AEGIS_PROJECT/fred/Attic/wilma,v || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
