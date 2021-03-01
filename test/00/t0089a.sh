#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1999, 2000, 2002, 2004, 2005 Peter Miller;
#	Copyright (C) 2005 Walter Franzini;
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
# MANIFEST: Test aedist functionality
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
unset LINES
unset COLS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

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
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT

#
# make the directories
#
activity="working directory 112"
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
activity="new project 131"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 138"
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
activity="staff 153"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -nbr -p foo 4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -nbr -p foo.4 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=foo.4.2 ; export AEGIS_PROJECT

#
# create a new change
#
activity="new change 172"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p foo.4.2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 190"
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
history_get_command =
	"co -u'$e' -p $h,v > $o";
history_create_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_put_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_query_command =
	"rlog -r $h,v | awk '/^head:/ {print $$2}'";
diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
history_put_trashes_file = warn;
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 221"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 228"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# now make a distribution set
#
activity="aedist -send 235"
$bin/aedist -send -o test.out -ndh > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Check the aedist archive
#
activity="extract the archive 242"
mkdir $work/test.d > log 2>&1
if test $? -ne 0; then cat log; no_result; fi
$bin/test_cpio -extract -change-dir $work/test.d -f test.out > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Check the archive header
#
activity="archive header 251"
cat > $work/header.ok <<EOF
MIME-Version: 1.0
Content-Type: application/aegis-change-set
Content-Transfer-Encoding: base64
Subject: foo.4.2 - The first change
Content-Name: foo.4.2.C001.ae
Content-Disposition: attachment; filename=foo.4.2.C001.ae
EOF
if test $? -ne 0; then no_result; fi

head -6 $work/test.out > $work/header.test
if test $? -ne 0; then no_result; fi

diff $work/header.ok $work/header.test
if test $? -ne 0; then fail; fi

#
# Chech the archive structure
#
activity="check the archive structure 271"
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
activity="etc/project-name 290"
cat > $work/test.ok <<EOF
$AEGIS_PROJECT
EOF
if test $? -ne 0; then no_result; fi
diff test.ok test.d/etc/project-name
if test $? -ne 0; then fail; fi

activity="etc/change-number 298"
cat > $work/test.ok <<'EOF'
1
EOF
if test $? -ne 0; then no_result; fi
diff $work/test.ok $work/test.d/etc/change-number
if test $? -ne 0; then fail; fi

activity="etc/change-set 306"
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
diff $work/test.ok $work/test.d/etc/change-set
if test $? -ne 0; then fail; fi

#
# Check the archive contents
#
activity="check the archive contents 336"

diff $work/test.d/src/aegis.conf $workchan/aegis.conf > log 2>&1
if test $? -ne 0; then cat log; fail; fi
diff $work/test.d/src/main.c $workchan/main.c > log 2>&1
if test $? -ne 0; then cat log; fail; fi

#
# the things tested in this test, worked
#
pass
