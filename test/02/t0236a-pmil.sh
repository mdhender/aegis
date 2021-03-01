#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 2007-2012 Peter Miller
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
    echo "FAILED test of the aemakegen functionality ($activity)" 1>&2
    cd $here
    find $work -type d -user $USER -exec chmod u+w {} \;
    rm -rf $work
    exit 1
}
no_result()
{
    set +x
    echo 'NO RESULT when testing the aemakegen functionality' \
            "($activity)" 1>&2
    cd $here
    find $work -type d -user $USER -exec chmod u+w {} \;
    rm -rf $work
    exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 114"
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

activity="new project 130"
$bin/aegis -newpro foo -version "" -dir $work/proj -lib $work/lib
if test $? -ne 0 ; then no_result; fi

AEGIS_PROJECT=foo
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="project attributes 139"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
if test $? -ne 0 ; then no_result; fi
aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 152"
aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 160"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 170"
aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 174"
aegis -nf -nl $work/${AEGIS_PROJECT}.C010/configure.ac
if test $? -ne 0 ; then no_result; fi

cat > $work/${AEGIS_PROJECT}.C010/configure.ac << 'fubar'
AC_CHECK_PROGS(GROFF, groff roff)
fubar
if test $? -ne 0 ; then no_result; fi

#
# test the aemakegen functionality
#
activity="aemakegen 186"

TAB=`awk 'BEGIN{printf("\t")}' /dev/null`
sed "s|{TAB}|$TAB|g" > test.ok << 'fubar'
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
#    W   W    A    RRRR   N   N   III  N   N  III  N   N   GGG
#    W   W   A A   R   R  NN  N    I   NN  N   I   NN  N  G   G
#    W   W  A   A  RRRR   N N N    I   N N N   I   N N N  G
#    W W W  AAAAA  R R    N  NN    I   N  NN   I   N  NN  G  GG
#    W W W  A   A  R  R   N   N    I   N   N   I   N   N  G   G
#     W W   A   A  R   R  N   N   III  N   N  III  N   N   GGG
#
# Warning: DO NOT send patches which fix this file. IT IS NOT the original
# source file. This file is GENERATED from the Aegis repository file manifest.
# If you find a bug in this file, it could well be an Aegis bug.
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#

#
# Tell automake to put the object file for foo/bar.cc in directory foo/
#
AUTOMAKE_OPTIONS = subdir-objects

bin/guano: script/guano.sh
{TAB}@mkdir -p bin
{TAB}cp script/guano.sh $@
{TAB}chmod a+rx $@

# manual pages
man_MANS = prog1/prog1.1

# executables to be installed
bin_PROGRAMS = bin/prog1

# scripts to be installed
bin_SCRIPTS = bin/guano

# The lib/libfoo.a library.
lib_libfoo_a_SOURCES = lib/a.cc lib/c.cc

# Static libraries, not to be installed.
noinst_LIBRARIES = lib/libfoo.a

# Files to be removed by the "distclean" make target.
DISTCLEANFILES = lib/config.h

# The prog1 program.
bin_prog1_SOURCES = prog1/main.cc
bin_prog1_LDADD = lib/libfoo.a

# How to run the test scripts.
TESTS_ENVIRONMENT = PATH=`pwd`/bin:$$PATH $(SHELL)

# The test scripts to be run.
TESTS = test/00/t0001a.sh test/00/t0002a.sh

# Additional source files to be included in the tarball.
EXTRA_DIST = aegis.extra/test_prelude.inc.sh configure.ac elsewhere/foo.sh \
     extra-super-realy-very-ultra-mega-long-source-code-line-to-check-for-size \
{TAB}{TAB}script/guano.sh

# vim: set ts=8 sw=8 noet :
fubar
test $? -eq 0 || no_result

# if we add --script '*.sh' to the command line,
# several more scripts get installed.
$bin/aemakegen --target=automake prog1/main.cc lib/a.cc lib/c.cc \
        test/00/t0001a.sh test/00/t0002a.sh \
        prog1/prog1.1 \
        aegis.extra/test_prelude.inc.sh \
extra-super-realy-very-ultra-mega-long-source-code-line-to-check-for-size \
        script/guano.sh \
        elsewhere/foo.sh \
        --project=foo --change=10 \
        > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
test $? -eq 0 || fail

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
