#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2005-2008, 2012 Peter Miller
#       Copyright (C) 2004, 2007, 2008 Walter Franzini
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

AEGIS_TEST_DIR=$work
export AEGIS_TEST_DIR

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

if test "$EXEC_SEARCH_PATH" != ""
then
    tpath=
    hold="$IFS"
    IFS=":$IFS"
    for tpath2 in $EXEC_SEARCH_PATH
    do
        tpath=${tpath}${tpath2}/${1-.}/bin:
    done
    IFS="$hold"
    PATH=${tpath}${PATH}
else
    PATH=${bin}:${PATH}
fi
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
        echo "FAILED test of the aedist functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT when testing the aedist functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
}
trap \"no_result\" 1 2 3 15

check_it()
{
        sed     -e "s|$work|...|g" \
                -e 's|= [0-9][0-9]*; /.*|= TIME;|' \
                -e "s/\"$USER\"/\"USER\"/g" \
                -e 's/uuid = ".*"/uuid = "UUID"/' \
                -e 's/19[0-9][0-9]/YYYY/' \
                -e 's/20[0-9][0-9]/YYYY/' \
                -e 's/node = ".*"/node = "NODE"/' \
                -e 's/crypto = ".*"/crypto = "GUNK"/' \
                < $2 > $work/sed.out
        if test $? -ne 0; then no_result; fi
        diff $1 $work/sed.out
        if test $? -ne 0; then fail; fi
}

activity="create test directory 117"
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
# test the aedist functionality
#
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="new project 139"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
        -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT


activity="project attributes 148"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 161"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 169"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 179"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 183"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/fred \
        $work/test.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "exit 0";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
        echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="build 210"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 214"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="uuid set 218"
$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd000 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 222"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 226"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 230"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 234"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 238"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# second change
#
activity="new change 245"
cat > caf << 'fubar'
brief_description = "the second change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 254"
$bin/aegis -db 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="copy file 258"
$bin/aegis -cp $work/test.C002/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo second > $work/test.C002/barney
if test $? -ne 0 ; then no_result; fi

activity="copy file 265"
$bin/aegis -cp $work/test.C002/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo third > $work/test.C002/fred
if test $? -ne 0 ; then no_result; fi

$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccdddddead -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build the change 275"
$bin/aegis -build -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 279"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="end the change 283"
$bin/aegis -dev_end -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 287"
$bin/aegis -ibeg 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate build 291"
$bin/aegis -build -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate pass 295"
$bin/aegis -ipass 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="inventory generate 299"
$bin/aedist -p test --inventory --path-prefix file: > $work/test.html
if test $? -ne 0; then fail; fi

activity="change inventory 303"
$bin/aegis -list change_inventory -ter > test.cin
if test $? -ne 0; then no_result; fi

activity="dump changes 307"
for d in `awk '{print $1}' < test.cin`
do
  uuid=`$bin/aesub -c test.$d '${change uuid}' | $bin/test_uuid --translate`
  mkdir -p `dirname $uuid`
  $bin/aedist -send -change test.$d -o $uuid.ae > LOG 2>&1
  if test $? -ne 0; then cat LOG; no_result; fi
done

activity="change list 316"
cat > $work/change.ok <<EOF
2
10
EOF
if test $? -ne 0; then no_result; fi

$bin/aegis -list change -p test -ter > test.changes
if test $? -ne 0; then no_result; fi

diff $work/change.ok test.changes
if test $? -ne 0; then no_result; fi

# ----------------------------------------------------------------

#
# Create a second project: test2
#
activity="new project 334"
$bin/aegis -npr test2 -version - -v -dir $work/test2.dir \
        -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test2
export AEGIS_PROJECT

activity="project attributes 342"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 355"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# The URL handling code must be improved, now we must write
# ./test.html (note the leading ./) instead of test.html
#
activity="replay 367"
$bin/aedist  --replay -p test2 -f ./test.html -no-trojan > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="dump inventory for test2 371"
$bin/aegis -list change_inventory -p test2 -terse > test2.cin
if test $? -ne 0; then no_result; fi

activity="check inventories 375"
diff test.cin test2.cin > cin.diff
if test $? -ne 0; then cat cin.diff; fail; fi

activity="change list 379"
$bin/aegis -list change -p test2 -ter > test2.changes
if test $? -ne 0; then no_result; fi

activity="check change list 383"
diff $work/change.ok test2.changes
if test $? -ne 0; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
