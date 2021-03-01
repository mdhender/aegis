#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 2007, 2008, 2010 Walter Franzini
# Copyright (C) 2008, 2012 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
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

parch=
test "$1" != "" && parch="$1/"
bin="$here/${parch}bin"

if test "$EXEC_SEARCH_PATH" != ""
then
    tpath=
    hold="$IFS"
    IFS=":$IFS"
    for tpath2 in $EXEC_SEARCH_PATH
    do
        tpath=${tpath}${tpath2}/${parch}bin:
    done
    IFS="$hold"
    PATH=${tpath}${PATH}
else
    PATH=${bin}:${PATH}
fi
export PATH

AEGIS_DATADIR=$here/lib
export AEGIS_DATADIR

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
    echo "FAILED test of the aem functionality ($activity)" 1>&2
    cd $here
    find $work -type d -user $USER -exec chmod u+w {} \;
    rm -rf $work
    exit 1
}
no_result()
{
    set +x
    echo "NO RESULT when testing the aem functionality ($activity)" 1>&2
    cd $here
    find $work -type d -user $USER -exec chmod u+w {} \;
    rm -rf $work
    exit 2
}
trap \"no_result\" 1 2 3 15

check_it()
{
    sed -e "s|$work|...|g" \
        -e 's|= [0-9][0-9]*; /.*|= TIME;|' \
        -e "s/\"$USER\"/\"USER\"/g" \
        -e 's/19[0-9][0-9]/YYYY/' \
        -e 's/20[0-9][0-9]/YYYY/' \
        -e 's/node = ".*"/node = "NODE"/' \
        -e 's/crypto = ".*"/crypto = "GUNK"/' \
        < $2 > $work/sed.out
    if test $? -ne 0; then no_result; fi
    diff -b $1 $work/sed.out
    if test $? -ne 0; then fail; fi
}


activity="create test directory 130"
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
# If the C++ compiler is called something other than "c++", as
# discovered by the configure script, create a shell script called
# "c++" which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
#
if test "$CXX" != "c++"
then
        cat >> $work/c++ << fubar
#!/bin/sh
exec ${CXX-g++} \$*
fubar
        if test $? -ne 0 ; then no_result; fi
        chmod a+rx $work/c++
        if test $? -ne 0 ; then no_result; fi
        PATH=${work}:${PATH}
        export PATH
fi

#
# test the aem functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 177"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 184"
cat > tmp << 'end'
description = "A bogus project created to test the "
    "aeipass functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 201"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 212"
cat > tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 224"
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 231"
$bin/aegis -nf  $workchan/aegis.conf -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd0 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
link_integration_directory = true;
history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;
diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
merge_command = "exit 0 # $input $output $orig $most_recent";
patch_diff_command = "set +e; diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 255"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 262"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 269"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 276"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 280"
$bin/aegis -diff 1 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# integrate build
#
activity="build 287"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 294"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch 298"
$bin/aegis -p example -nbr 10 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# New branch
#
AEGIS_PROJECT=${AEGIS_PROJECT}.10
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 311"
cat > tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 323"
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 330"
$bin/aegis -nf  $workchan/file1 -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/file1 << 'end'
file1, line 1
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 343"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 350"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 357"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 364"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 368"
$bin/aegis -diff 1 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# integrate build
#
activity="build 375"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 382"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# New branch
#
activity="new branch 389"
$bin/aegis -nbr -p example 20 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

AEGIS_PROJECT=example.20
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 399"
cat > tmp << 'end'
brief_description = "The third change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 411"
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change.
# We set a UUID different from the one set in .10.C1 to trigger a bug
# in the aem code.
#
activity="new file 420"
$bin/aegis -nf $workchan/file1 -nl
if test $? -ne 0 ; then no_result; fi

activity="check cstate 424"
cat > ok <<EOF
src =
[
        {
                file_name = "file1";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                action = create;
                usage = source;
        },
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/020.branch/0/001.fs

#toberemoved: sovrascrivi i file cstate per impostare lo UUID del
#toberemoved: file, non ci sono attualmente altri modi di farlo.
cat > $work/proj/info/change/0/020.branch/0/001.fs <<EOF
src =
[
        {
                file_name = "file1";
                uuid = "aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1";
                action = create;
                usage = source;
        },
];
EOF
test $? -eq 0 || no_result

cat > $workchan/file1 << 'end'
Some random stuff
Some not so random stuff
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 464"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 471"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 478"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 485"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 489"
$bin/aegis -diff 1 -nl -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# integrate build
#
activity="build 496"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 503"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# close example.10
#
AEGIS_PROJECT=example
export AEGIS_PROJECT

activity="close the branch 513"
$bin/aegis -dev_end -c 10 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the branch 517"
$bin/aegis -ib -c 10 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="complete the branch integration 521"
$bin/aefinish -c 10 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Go back to example.20
#
AEGIS_PROJECT=${AEGIS_PROJECT}.20
export AEGIS_PROJECT

#
# create a new change
#
activity="new change 534"
cat > tmp << 'end'
brief_description = "The fourth change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 546"
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change.
# We set a UUID different from the one set in .1.C1 to trigger a bug
# in the aem code.
#
activity="new file 555"
$bin/aegis -cp  $workchan/file1 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat >> $workchan/file1 << 'end'
More random stuff
More not so random stuff
end
if test $? -ne 0 ; then no_result; fi

activity="merge the file 565"
$bin/aegis -diff -only-merge -gp -v > log 2>&1
if test $? -ne 0; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
