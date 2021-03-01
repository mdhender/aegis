#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996-1998, 2005-2008 Peter Miller
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
	echo 'FAILED test of the large lexer strings functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT for test of the large lexer strings functionality' 1>&2
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

#
# test the large lexer strings functionality
#
$bin/aegis -np test -vers "" -lib $work/lib -dir $work/proj
if test $? -ne 0 ; then no_result; fi

#
# build a large attr file
#
cat > test.in << 'fubar'
Nothing much, until we multiply it a few times.\n\
fubar
if test $? -ne 0 ; then no_result; fi
for j in 1 2 3 4 5
do
	cat test.in test.in test.in test.in > test.in2
	if test $? -ne 0 ; then no_result; fi
	mv test.in2 test.in
	if test $? -ne 0 ; then no_result; fi
done

cat > attr-file << 'fubar'
brief_description = "something short";
description = "begin\n\
fubar
if test $? -ne 0 ; then no_result; fi
cat test.in >> attr-file
if test $? -ne 0 ; then no_result; fi
cat >> attr-file << 'fubar'
end";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
fubar
if test $? -ne 0 ; then no_result; fi

#
# the fixed buffer size in the lexer used to make this dump core or
# trash memory, etc.
#
$bin/aegis -nc 1 -f attr-file -p test -lib $work/lib
if test $? -ne 0 ; then fail; fi

$bin/aegis -ca -l 1 -p test -lib $work/lib > test.out
if test $? -ne 0 ; then fail; fi

diff attr-file test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
