#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2003-2008 Peter Miller
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
	echo 'FAILED test of the aecvsserver update functionality' \
		"($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the aecvsserver update functionality' \
		"($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
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

workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 139"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 146"
cat > tmp << 'end'
description = "A bogus project created to test the aecvsserver update "
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
activity="staff 162"
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
activity="new change 175"
cat > tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 193"
$bin/aegis -nf  $workchan/aegis.conf $workchan/dir/bogus -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/dir/bogus << 'end'
the trunk version
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
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
patch_diff_command = "set +e; diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 224"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 231"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 238"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 245"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 252"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 259"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 266"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# create a new change
#
activity="new change 275"
cat > tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

unset AELOG
CVS_SERVER=$bin/aecvsserver
export CVS_SERVER

#
# Now use cvs to checkout the project.
#
activity="cvs checkout 299"
cvs -d :fork:/aegis co ${AEGIS_PROJECT}.C002 > log 2>&1
if test $? -ne 0 ; then ls -lR; cat log; fail; fi

diff $AEGIS_PROJECT.C002/aegis.conf $work/proj/baseline/aegis.conf
if test $? -ne 0 ; then fail; fi

diff $AEGIS_PROJECT.C002/dir/bogus $work/proj/baseline/dir/bogus
if test $? -ne 0 ; then fail; fi

# --------------------------------------------------------------------------

#
# Now integrate a new change, so that the version changes.
#
activity="new change 314"
cat > tmp << 'end'
brief_description = "The third change.";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 323"
$bin/aegis -db 3 -dir ${workchan}3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy file 327"
$bin/aegis -cp 3 -baserel dir/bogus > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo "third" > ${workchan}3/dir/bogus
if test $? -ne 0 ; then no_result; fi

activity="build 334"
$bin/aegis -build 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 338"
$bin/aegis -diff 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 342"
$bin/aegis -de 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 346"
$bin/aegis -rpass -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 350"
$bin/aegis -ib 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 354"
$bin/aegis -b -nl -v 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 358"
$bin/aegis -intpass -nl 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# Now make sure it gets updated.
#
activity="cvs update 367"
cvs -d :fork:/aegis update -d $AEGIS_PROJECT.C002 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# make sure the change tracks the cvs work area
#
diff $workproj/baseline/dir/bogus $AEGIS_PROJECT.C002/dir/bogus
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
