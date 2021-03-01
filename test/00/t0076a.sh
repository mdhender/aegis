#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1998, 2000, 2004, 2005 Peter Miller;
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
# MANIFEST: Test the trim_directory subst functionality
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
PAGER=cat
export PAGER
AEGIS_FLAGS="delete_file_preference = no_keep; \
	lock_wait_preference = always; \
	diff_preference = automatic_merge; \
	pager_preference = never; \
	persevere_preference = all; \
	log_file_preference = never; \
	default_project_directory = \"$work\"; \
	default_development_directory = \"$work\";"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin=$here/${1-.}/bin

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
	echo "FAILED test of the trim_directory subst functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of the trim_directory subst functionality ($activity)" 1>&2
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

AEGIS_PROJECT=test
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

#
# test the trim_directory subst functionality
#
activity="new project 121"
$bin/aegis -npr test -version '' -v -dir $work/proj.dir > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes 125"
cat > paf << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 136"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 144"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 154"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 158"
$bin/aegis -nf $work/test.C010/aegis.conf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "exit 0";
history_get_command =
	"co -u'$e' -p ${quote $h,v} > $o";
history_create_command =
	"ci -f -u -m/dev/null -t/dev/null ${quote $i} ${quote $h,v}; rcs -U ${quote $h,v}";
history_put_command =
	"ci -f -u -m/dev/null -t/dev/null ${quote $i} ${quote $h,v}; rcs -U ${quote $h,v}";
history_query_command =
	"rlog -r ${quote $h,v} | awk '/^head:/ {print $$2}'";
diff_command = "set +e; diff ${quote $orig} ${quote $i} > ${quote $out}; test $$? -le 1";
diff3_command = "(diff3 -e ${quote $mr} ${quote $orig} ${quote $i} | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
shell_safe_filenames = false;
fubar
if test $? -ne 0 ; then no_result; fi

activity="new file 179"
$bin/aegis -nf "$work/test.C010/template" -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat >> $work/test.C010/aegis.conf << 'fubar'

file_template =
[
	{
		pattern = [ "*" ];
		body = "${read_file ${source template abs}}";
	},
];
fubar
if test $? -ne 0 ; then no_result; fi

cat > $work/test.C010/template << 'fubar'
trimdir: ${trimdir $fn}
trimdir0: ${tdir 0 $fn}
trimdir1: ${trimdire 1 $fn}
trimdir2: ${trim_direc 2 $fn}
trimdir3: ${trimdirectory 3 $fn}
trimext: ${trim_extension $fn}
fubar
if test $? -ne 0 ; then no_result; fi

activity="new file 205"
$bin/aegis -nf "$work/test.C010/a/b/c/ddd.eee" -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > test.ok << 'fubar'
trimdir: b/c/ddd.eee
trimdir0: a/b/c/ddd.eee
trimdir1: b/c/ddd.eee
trimdir2: c/ddd.eee
trimdir3: ddd.eee
trimext: a/b/c/ddd
fubar
if test $? -ne 0 ; then cat log; no_result; fi

diff test.ok $work/test.C010/a/b/c/ddd.eee
if test $? -ne 0 ; then fail; fi

activity="new file 222"
$bin/aegis -nf "$work/test.C010/a/b/ddd.eee" -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > test.ok << 'fubar'
trimdir: b/ddd.eee
trimdir0: a/b/ddd.eee
trimdir1: b/ddd.eee
trimdir2: ddd.eee
trimdir3: ddd.eee
trimext: a/b/ddd
fubar
if test $? -ne 0 ; then cat log; no_result; fi

diff test.ok $work/test.C010/a/b/ddd.eee
if test $? -ne 0 ; then fail; fi

activity="new file 239"
$bin/aegis -nf "$work/test.C010/a/ddd.eee" -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > test.ok << 'fubar'
trimdir: ddd.eee
trimdir0: a/ddd.eee
trimdir1: ddd.eee
trimdir2: ddd.eee
trimdir3: ddd.eee
trimext: a/ddd
fubar
if test $? -ne 0 ; then cat log; no_result; fi

diff test.ok $work/test.C010/a/ddd.eee
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
