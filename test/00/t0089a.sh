#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1999, 2000, 2002, 2004-2008 Peter Miller
#	Copyright (C) 2005 Walter Franzini;
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

#
# This test doesn't work for HP/UX
#
system=`uname -s`
case "$system" in
*HP?UX*)
    echo ''
    echo '	This test is not meaningful for HP/UX'
    echo '	It is declared to pass by default.'
    echo ''
    exit 0
    ;;
*)
    ;;
esac

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$

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
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

no_result()
{
	set +x
	echo "NO RESULT for test of aedist functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of aedist functionality ($activity)" 1>&2
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
trap "no_result" 1 2 3 15

#
# some variable to make things earier to read
#
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

worklib=$work/lib
workproj=$work/proj.dir
workchan=$work/chan.dir
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=abcdefg ; export AEGIS_PROJECT

#
# make the directories
#
activity="working directory 141"
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
# make a new project
#
activity="new project 160"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 167"
cat > $tmp << 'end'
description = "A bogus project created to test the aedist functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 182"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -nbr -p $AEGIS_PROJECT 4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -nbr -p ${AEGIS_PROJECT}.4 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=${AEGIS_PROJECT}.4.2 ; export AEGIS_PROJECT

#
# create a new change
#
activity="new change 201"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p ${AEGIS_PROJECT} > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 219"
$bin/aegis -nf $workchan/main.c $workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.c << 'end'
/* $Id$ */
int main() { test(); exit(0); return 0; }
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
link_integration_directory = true;
create_symlinks_before_build = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
history_put_trashes_file = warn;
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 250"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 257"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# now make a distribution set
#
activity="aedist -send 264"
$bin/aedist -send -o test.out -ndh > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Check the aedist archive
#
activity="extract the archive 271"
mkdir $work/test.d > log 2>&1
if test $? -ne 0; then cat log; no_result; fi
$bin/test_cpio -extract -change-dir $work/test.d -f test.out > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Check the archive header
#
activity="archive header 280"
cat > $work/header.ok <<EOF
MIME-Version: 1.0
Content-Type: application/aegis-change-set
Content-Transfer-Encoding: base64
Subject: ${AEGIS_PROJECT} - The first change
Content-Name: ${AEGIS_PROJECT}.C001.ae
Content-Disposition: attachment; filename=${AEGIS_PROJECT}.C001.ae
EOF
if test $? -ne 0; then no_result; fi

head -6 $work/test.out > $work/header.test
if test $? -ne 0; then no_result; fi

diff -b $work/header.ok $work/header.test
if test $? -ne 0; then fail; fi

#
# Check the archive structure
#
activity="check the archive structure 300"
cat > $work/test.ok <<EOF
$work/test.d/etc/change-number
$work/test.d/etc/change-set
$work/test.d/etc/project-name
$work/test.d/src/aegis.conf
$work/test.d/src/main.c
EOF
if test $? -ne 0; then no_result; fi

find $work/test.d -type f -print | sort > $work/out.list
if test $? -ne 0; then no_result; fi

diff $work/test.ok $work/out.list
if test $? -ne 0; then fail; fi

#
# Check the metadata
#
activity="etc/project-name 319"
cat > $work/test.ok <<EOF
$AEGIS_PROJECT
EOF
if test $? -ne 0; then no_result; fi
diff -b test.ok test.d/etc/project-name
if test $? -ne 0; then fail; fi

activity="etc/change-number 327"
cat > $work/test.ok <<'EOF'
1
EOF
if test $? -ne 0; then no_result; fi
diff -b $work/test.ok $work/test.d/etc/change-number
if test $? -ne 0; then fail; fi

activity="etc/change-set 335"
cat > $work/test.ok <<EOF
brief_description = "The first change";
description = "The first change";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
state = awaiting_development;
src =
[
	{
		file_name = "aegis.conf";
		action = create;
		usage = config;
	},
	{
		file_name = "main.c";
		action = create;
		usage = source;
	},
];
EOF
if test $? -ne 0; then fail; fi
diff -b $work/test.ok $work/test.d/etc/change-set
if test $? -ne 0; then fail; fi

#
# Check the archive contents
#
activity="check the archive contents 365"

diff -b $work/test.d/src/aegis.conf $workchan/aegis.conf > log 2>&1
if test $? -ne 0; then cat log; fail; fi
diff -b $work/test.d/src/main.c $workchan/main.c > log 2>&1
if test $? -ne 0; then cat log; fail; fi

############################################################################
#
# Test some things again, this time using the real cpio command
# but only if the real cpio command is available.
#
cpio -o < /dev/null > /dev/null 2>&1
if test $? -eq 0
then
    activity="aedist -send 380"
    $bin/aedist -send -o test.out.gz -nmh -ndh -cte=none \
	-comp-alg=gzip > log 2>&1
    if test $? -ne 0 ; then cat log; fail; fi

    gunzip < test.out.gz > test.out
    if test $? -ne 0; then cat log; no_result; fi

    #
    # Check the aedist archive
    #
    activity="extract the archive 391"
    mkdir $work/test.d2 > log 2>&1
    if test $? -ne 0; then cat log; no_result; fi
    ( cd test.d2 ; cpio -i -d < ../test.out ) > log 2>&1
    if test $? -ne 0; then cat log; no_result; fi

    grep 'junk' log
    test $? -ne 0 || fail

    #
    # Check the archive structure
    #
    activity="check the archive structure 403"
    cat > $work/test.ok <<EOF
$work/test.d2/etc/change-number
$work/test.d2/etc/change-set
$work/test.d2/etc/project-name
$work/test.d2/src/aegis.conf
$work/test.d2/src/main.c
EOF
    if test $? -ne 0; then no_result; fi

    find $work/test.d2 -type f -print | sort > $work/out.list
    if test $? -ne 0; then no_result; fi

    diff $work/test.ok $work/out.list
    if test $? -ne 0; then fail; fi
fi

#
# the things tested in this test, worked
#
pass
