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
# MANIFEST: Test 'aegis -VERSion'
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE

fail()
{
	echo "FAILED test of 'aegis -VERSion'" 1>&2
	exit 1
}
pass()
{
	exit 0
}
trap "fail" 1 2 3 15

PAGER=cat
export PAGER

./bin/aegis -vers -help > /dev/null
if test $? -ne 0 ; then fail; fi

./bin/aegis -vers > /dev/null
if test $? -ne 0 ; then fail; fi

./bin/aegis -vers r | cat
if test $? -ne 0 ; then fail; fi

./bin/aegis -vers w | cat
if test $? -ne 0 ; then fail; fi

#
# the things tested by this test, worked
#
pass
