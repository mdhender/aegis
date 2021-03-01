#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1999, 2004, 2006-2008 Peter Miller
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
	echo 'FAILED test of the zlib decode functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the zlib decode functionality' 1>&2
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
# test the zlib decode functionality
#
cat > test.in.uue << 'fubar'

begin 644 test.in
M'XL("#:5TC8"`W@`39%1;L0@#$3_<XI<)4<AP=UXFX($;*7<OAZ/4?J1F0<V
MC@U;:N>GKUMJM:S;GO:TVG=\0[\&M&2+A$N>I.4U\4?*6(([-KM0,PWQAR*#
M=E(E+_38C>(]?`@UTV:]\=0;TA8WKG!NVWV475QN*L)['=2.G#I\U>17[?CR
M8/ZWW1]F2W/A-_,LO.)M17*UUM;P3M"2+M#G&&'^#P#K!/7`BI$(<POU,8\=
M-+A<-!7W[N\G+8L02AHG>I'6DE\8('XU$87%8X/(GN"-;O.BQIW*@3^?M5&;
M)SIX<:<9XR6=;%RS4/'(9MSC$VE^>_OZ2HIQ])+BV9<.9:81'N\M?FTPZWOY
)`V/MU^VW`@``
`
end
fubar
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
Aarhus Aaron Ababa aback abaft abandon abandoned abandoning abandonment
abandons abase abased abasement abasements abases abash abashed
abashes abashing abasing abate abated abatement abatements abater
abates abating Abba abbe abbey abbeys abbot abbots Abbott abbreviate
abbreviated abbreviates abbreviating abbreviation abbreviations Abby
abdomen abdomens abdominal abduct abducted abduction abductions abductor
abductors abducts Abe abed Abel Abelian Abelson Aberdeen Abernathy
aberrant aberration aberrations abet abets abetted abetter abetting
abeyance abhor abhorred abhorrent abhorrer abhorring abhors abide abided
abides abiding Abidjan Abigail Abilene abilities ability abject abjection
fubar
if test $? -ne 0 ; then no_result; fi

$bin/test_base64 -uu -i test.in.uue test.in
if test $? -ne 0 ; then no_result; fi

$bin/test_zlib --input test.in test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
