#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1999 Peter Miller;
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
# MANIFEST: Test the capitalize subst functionality
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
	log_file_preference = never;"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin=$here/${1-.}/bin

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
	echo 'FAILED test of the capitalize subst functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the capitalize subst functionality' 1>&2
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
# This next bit should read...
#       unset LANG
#       unset LANGUAGE
# but RedHat 6.2 (libc2.1.3) dumps core in towupper if it isn't set.
LANG=en; export LANG
LANGUAGE=en; export LANGUAGE

#
# test the capitalize subst functionality
#
$bin/aegis -npr test -version "" -lib $work/lib -dir $work/proj -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -nd $USER -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > caf << 'fubar'
brief_description = "not much";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc -f caf -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -db 10 -p test -lib $work/lib -dir $work/chan -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -nf $work/chan/config -c 10 -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > $work/chan/config << 'fubar'
build_command = "exit 0";
history_create_command = "exit 0";
history_get_command = "exit 0";
history_put_command = "exit 0";
history_query_command = "exit 0";
diff_command = "exit 0";
merge_command = "exit 0";
file_template =
[
	{
		pattern = [ "*" ];
		body = "${capitalize JUST-TRYING-it-all-out}";
	}
];
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nf $work/chan/foo -c 10 -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

cat > ok << 'fubar'
Just-Trying-It-All-Out
fubar
if test $? -ne 0 ; then no_result; fi

diff ok $work/chan/foo
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
