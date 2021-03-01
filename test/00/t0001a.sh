#! /bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# MANIFEST: test -Help for each function
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE

fail()
{
	echo FAILED test of -Help for each function 1>&2
	exit 1
}
pass()
{
	exit 0
}
trap "fail" 1 2 3 15

#
# make sure the various variations have -help implemented
#
PAGER=cat
export PAGER
./bin/aegis -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -newpro -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -proatt -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -newcha -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -newdev -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -devbeg -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -new_file -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -newt -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -build -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -diff -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -test -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -dev_end -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -newrev -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -review_fail -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -review_pass -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -newint -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -intbeg -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -cd -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -integrate_begin_undo -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -intfail -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -intpass -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -list -help > /dev/null
if test $? -ne 0 ; then fail; fi
./bin/aegis -cp -help > /dev/null
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass
