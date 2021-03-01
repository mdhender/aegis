#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
unset AEGIS_PATH
unset AEGIS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

if test "$1" != "" ; then bin="./$1/bin"; else bin="./bin"; fi

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

AEGIS_FLAGS="delete_file_preference = no_keep; \
	diff_preference = automatic_merge;"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

$bin/aegis -help > /dev/null
if test $? -ne 0 ; then fail; fi
$bin/aegis -b -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -ca -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -cd -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -cp -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -cpu -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -db -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -dbu -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -de -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -deu -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -diff -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -ib -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -ibu -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -ipass -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -ifail -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -l -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -mv -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -na -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -nc -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -ncu -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -nd -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -nf -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -nfu -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -ni -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -npr -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -nrls -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -nrv -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -nt -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -ntu -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -pa -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -ra -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -rd -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -rm -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -rmu -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -rmpr -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -ri -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -rrv -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -rfail -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -rpass -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -rpu -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -t -help > /dev/null
if test $? -ne 0 ; then fail ; fi
$bin/aegis -vers -help > /dev/null
if test $? -ne 0 ; then fail ; fi

#
# the things tested in this test, worked
#
pass
