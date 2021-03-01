#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994, 1995 Peter Miller;
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
# MANIFEST: Test the aechown functionality
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$
PAGER=cat
export PAGER
AEGIS_FLAGS="default_development_directory = \"$work\"; \
	default_project_directory = \"$work\"; \
	delete_file_preference = no_keep; \
	diff_preference = automatic_merge;"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 1; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

fail()
{
	set +x
	echo 'FAILED test of the aechown functionality' 1>&2
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
trap \"fail\" 1 2 3 15

mkdir $work
if test $? -ne 0 ; then fail; fi
mkdir $work/lib
if test $? -ne 0 ; then fail; fi
chmod 0777 $work/lib
if test $? -ne 0 ; then fail; fi

AEGIS_PATH=$work/lib
export AEGIS_PATH
if test $? -ne 0 ; then fail; fi
cd $work
if test $? -ne 0 ; then fail; fi

#
# test the aechown functionality
#
$bin/aegis -npr foo
if test $? -ne 0 ; then fail; fi

AEGIS_PROJECT=foo
export AEGIS_PROJECT

$bin/aegis -nd $USER
if test $? -ne 0 ; then fail; fi

cat > eric << 'fubar'
brief_description = "the change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -nc -f eric -p foo
if test $? -ne 0 ; then fail; fi

$bin/aegis -db 1
if test $? -ne 0 ; then fail; fi

$bin/aegis -nf foo.C001/xyzzy
if test $? -ne 0 ; then fail; fi

$bin/aegis -chown -c 1 -u $USER
if test $? -ne 0 ; then fail; fi

if test ! -f foo.D001/xyzzy ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
