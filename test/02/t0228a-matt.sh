#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2005 Matthew Lee;
#       All rights reserved.
#       Copyright (C) 2007, 2008, 2012 Peter Miller
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

unset AEGIS_DATADIR
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
        log_file_preference = never; \
        default_development_directory = \"$work\";"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

# This tells aeintegratq that it is being used by a test.
AEGIS_TEST_DIR=$work
export AEGIS_TEST_DIR

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

#
# set the path, so that the aegis command that aepatch/aedist invokes
# is from the same test set as the aepatch/aedist command itself.
#
PATH=${bin}:$PATH
export PATH

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
        echo "FAILED test of the aeget rssicon functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT when testing the aeget rssicon functionality \
              ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory"
mkdir $work $work/lib $work/icon
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
# test the aeget/?rssicon functionality
#
cat > rss.gif.uue << 'EOF'
begin 600 icon/rss.gif
M1TE&.#=A)``.`/0``"$.`?"N@O!C!=58!/KT[Y]!`]^><O^%-'TS`NW)L=!R
M,^6$0\)0!/'4P/]F`.>SD?^:5^5>!/?@T/]R%>"G@O____^K<]1A%/_1LN>]
MH??IX/FR@O!L%/]\)/_V\/#!H2P`````)``.```%[:"'C61IGJC9*)#COG`L
MSS+2OL>F0P<'=Y:-1?#;$6$VV*;";"Z(EB9S0,1(-0/DS;7,*"Z-2H/*5#`T
M%4/$4;Z4UZ[D:VE`,!3B0H3Y8#`6%UE,%'<+#%EQ6PY+!`T23`9Z$$T>A&L!
ME`8,<`YR7%(5"@54$0,9311K$1>GD)R>BVD(:!)Z$QH)#`4+%00,!QH!!04&
M8@Q'L$L4!6YI:Y`*'WD<3`L*80\%R(K*F\45%Q&84J(1":`%QXDP!P]F`@(4
M%`H1$0;RD:,.]OC#KQ8Q!%!Y,0"1@X(%C[A`./`%@@(`(DJ<2+&B18LA```[
`
end
EOF

#
# Decode the expected file and add a Content-Type header (since aeget
# will add one when it serves up the icon).
#
activity="build rss.gif"
$bin/test_base64 -nh -uu -i rss.gif.uue icon/rss.gif
if test $? -ne 0 ; then fail; fi

#
# We need to invoke test_magic here because the behaviour of libmagic
# change with versions and we need to replicate it exactly to have the
# test pass.
#
echo "Content-Type:" `test_magic icon/rss.gif` > rss.expected.gif
echo "Content-Length: 360" >> rss.expected.gif
echo >> rss.expected.gif
cat icon/rss.gif >> rss.expected.gif

# Ask aeget for the icon.
activity="aeget"
AEGIS_DATADIR=$work \
REQUEST_METHOD=get \
SCRIPT_NAME=/cgi-bin/aeget \
PATH_INFO=icon/rss.gif \
QUERY_STRING= \
$bin/aeget > rss.served.gif 2>&1
if test $? -ne 0 ; then fail; fi

# Compare the expected and the actual received file.
activity="Compare gifs"
cmp rss.expected.gif rss.served.gif > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass


# vim: set ts=8 sw=4 et :
