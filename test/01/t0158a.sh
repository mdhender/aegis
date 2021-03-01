#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2002, 2006-2008 Peter Miller
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
	echo 'FAILED test of the aesub expr functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the aesub expr functionality' 1>&2
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

$bin/aegis -newproj example -version - -lib $work/lib -dir $work/proj
if test $? -ne 0 ; then no_result; fi

AEGIS_PROJECT=example
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

cat > ca << 'fubar'
description = "short";
brief_description = "short";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -newchan 42 -p example -f ca
if test $? -ne 0 ; then no_result; fi

AEGIS_CHANGE=42
export AEGIS_CHANGE

#
# test the aesub expr functionality
#
echo 10 > test.ok
if test $? -ne 0 ; then no_result; fi
$bin/aesub '${expr 23 % 13}' > test.out
if test $? -ne 0 ; then fail; fi
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

echo 1 > test.ok
if test $? -ne 0 ; then no_result; fi
$bin/aesub '${expr 1 < 2}' > test.out
if test $? -ne 0 ; then fail; fi
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

echo 1 > test.ok
if test $? -ne 0 ; then no_result; fi
$bin/aesub '${expr 1 <= 2}' > test.out
if test $? -ne 0 ; then fail; fi
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

echo 0 > test.ok
if test $? -ne 0 ; then no_result; fi
$bin/aesub '${expr 1 > 2}' > test.out
if test $? -ne 0 ; then fail; fi
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

echo 0 > test.ok
if test $? -ne 0 ; then no_result; fi
$bin/aesub '${expr 1 >= 2}' > test.out
if test $? -ne 0 ; then fail; fi
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

echo 0 > test.ok
if test $? -ne 0 ; then no_result; fi
$bin/aesub '${expr 1 == 2}' > test.out
if test $? -ne 0 ; then fail; fi
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

echo 1 > test.ok
if test $? -ne 0 ; then no_result; fi
$bin/aesub '${expr 1 != 2}' > test.out
if test $? -ne 0 ; then fail; fi
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

echo 1 > test.ok
if test $? -ne 0 ; then no_result; fi
$bin/aesub '${expr !0}' > test.out
if test $? -ne 0 ; then fail; fi
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
