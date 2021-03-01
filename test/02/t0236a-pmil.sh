#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2007, 2008 Peter Miller
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

activity="create test directory"
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
# test the aemakegen functionality
#
activity="aemakegen"

TAB=`awk 'BEGIN{printf("\t")}' /dev/null`
sed "s|        |$TAB|g" > test.ok << 'fubar'
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
#    W   W    A    RRRR   N   N   III  N   N  III  N   N   GGG
#    W   W   A A   R   R  NN  N    I   NN  N   I   NN  N  G   G
#    W   W  A   A  RRRR   N N N    I   N N N   I   N N N  G
#    W W W  AAAAA  R R    N  NN    I   N  NN   I   N  NN  G  GG
#    W W W  A   A  R  R   N   N    I   N   N   I   N   N  G   G
#     W W   A   A  R   R  N   N   III  N   N  III  N   N   GGG
#
# Warning: DO NOT send patches which fix this file.
# This file is GENERATED from the Aegis repository file
# manifest.  If you find a bug in this file, it could
# well be an Aegis bug.
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#
# Tell automake to put the object file for foo/bar.cc in
# directory foo/
#
AUTOMAKE_OPTIONS = subdir-objects

bin/test_prelude.inc: aegis.extra/test_prelude.inc.sh
        cp aegis.extra/test_prelude.inc.sh $@
        chmod a+rx $@

bin/foo: elsewhere/foo.sh
        cp elsewhere/foo.sh $@
        chmod a+rx $@

bin/guano: script/guano.sh
        cp script/guano.sh $@
        chmod a+rx $@

# executables to be installed
bin_PROGRAMS = \
        bin/prog1

# manual pages
man_MANS = \
        prog1.1

EXTRA_DIST = \
        aegis.extra/test_prelude.inc.sh \
        elsewhere/foo.sh \
        lib/a.cc \
        lib/c.cc \
        prog1.1 \
        prog1/main.cc \
        script/guano.sh \
        test/00/t0001a.sh \
        test/00/t0002a.sh

# scripts to be installed
bin_SCRIPTS = \
        bin/guano

# scripts needed to run tests
check_SCRIPTS = \
        bin/test_prelude.inc

# scripts not to be installed
noinst_SCRIPTS = \
        bin/foo

noinst_LIBRARIES = \
        lib/lib.a

lib_lib_a_SOURCES = \
        lib/a.cc \
        lib/c.cc

bin_prog1_SOURCES = \
        prog1/main.cc

bin_prog1_LDADD = \
        lib/lib.a
fubar
test $? -eq 0 || no_result

$bin/aemakegen --target=automake prog1/main.cc lib/a.cc lib/c.cc \
        test/00/t0001a.sh test/00/t0002a.sh \
        prog1.1 \
        --script '*.sh' \
        aegis.extra/test_prelude.inc.sh \
        script/guano.sh \
        elsewhere/foo.sh \
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
