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
	echo 'FAILED test of the plural-forms functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the plural-forms functionality' 1>&2
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
# Need a project so that aesub works
#
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
# test the plural-forms substitution
#
cat > test.in << 'fubar'
200 ${plf
'@nplurals=3;plural=(n% 10==1&&n% 100!=11)?0: (n% 10==2&&n% 100!=12)?1: 2'
	200 singular plural plural2}
201 ${plf 201 singular plural plural2}
202 ${plf 202 singular plural plural2}
203 ${plf 203 singular plural plural2}
204 ${plf 204 singular plural plural2}
205 ${plf 205 singular plural plural2}
206 ${plf 206 singular plural plural2}
207 ${plf 207 singular plural plural2}
208 ${plf 208 singular plural plural2}
209 ${plf 209 singular plural plural2}
210 ${plf 210 singular plural plural2}
211 ${plf 211 singular plural plural2}
212 ${plf 212 singular plural plural2}
213 ${plf 213 singular plural plural2}
214 ${plf 214 singular plural plural2}
215 ${plf 215 singular plural plural2}
216 ${plf 216 singular plural plural2}
217 ${plf 217 singular plural plural2}
218 ${plf 218 singular plural plural2}
219 ${plf 219 singular plural plural2}
220 ${plf 220 singular plural plural2}
221 ${plf 221 singular plural plural2}
222 ${plf 222 singular plural plural2}
223 ${plf 223 singular plural plural2}
224 ${plf 224 singular plural plural2}
fubar
if test $? -ne 0 ; then no_rtesult; fi

cat > test.ok << 'fubar'
200 plural2
201 singular
202 plural
203 plural2
204 plural2
205 plural2
206 plural2
207 plural2
208 plural2
209 plural2
210 plural2
211 plural2
212 plural2
213 plural2
214 plural2
215 plural2
216 plural2
217 plural2
218 plural2
219 plural2
220 plural2
221 singular
222 plural
223 plural2
224 plural2
fubar
if test $? -ne 0 ; then no_rtesult; fi

$bin/aesub "\${read_file $work/test.in}" > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
