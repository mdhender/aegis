#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 1994-1999, 2006-2009, 2012 Peter Miller
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program. If not, see
#       <http://www.gnu.org/licenses/>.
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

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

no_result()
{
        set +x
        echo "NO RESULT for test of input_uudecode functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
}
fail()
{
        set +x
        echo "FAILED test of input_uudecode functionality ($activity)" 1>&2
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
trap "no_result" 1 2 3 15

#
# some variable to make things earier to read
#
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

worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT

#
# make the directories
#
activity="working directory 96"
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

activity="uudecode, no headers, line 112"

cat > test.ae.uue << 'fubar'

begin 644 test.ae
M'XL(`````````Y52P6K#,`SM-?X*M^>EC9M!!Z6''788[%1Z&R,HKI)ZI$ZP
ME6TP]N^SU[0TIH/$&(2D)_D]2\DJ626"G\Z??1!PW_E)8$7@Y__@`BLN=4AR
MT9CZ'27%&HXX88"ELO-T+@1CR367Y2@N8KD?QH47UUSD`72)L46:,,9RH[#(
M]FBE40VI6O,-G]%!6>XN<$)+LS4+\KM>GI\Z<M=Q[K`26HL.I32AT5!E>5NN
MF0=F^(7'AER.3(M=+`>+E=(8)@V6!JUU+V:W:BT!^5?@$Q0I73H)'UC5S1$U
MN:R1?,->6?3-HJA0%6;^Y\_28G<A[J1%$<A.ES3HFOI0:Z'T<%NW1OK(SQU[
M6[OOZL\K';D[Q<#=2<^^T['H,_8SZX\GX)2,Y#347O9^MWU\?GG:3J=3S^47
(V2C(4TP#````
`
end
fubar
test "$?" -eq 0 || no_result

$bin/test_base64 -uu -i test.ae.uue test.ae
test "$?" -eq 0 || no_result

$bin/aedist -l -f test.ae -o test.out
test "$?" -eq 0 || fail

#
# the things tested in this test, worked
#
pass
# vim: set ts=8 sw=4 et :
