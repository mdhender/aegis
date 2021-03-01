#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2006, 2008 Peter Miller
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
output=
here=`pwd`
test $? -eq 0 || exit 1
tmp=${TMP_DIR:-/tmp}/$$

fail() {
    cd $here
    rm -rf $tmp
    exit 1
}
trap "fail" 1 2 3 15

set -x
mkdir $tmp $tmp/lib
test $? -eq 0 || fail
cd $tmp
test $? -eq 0 || fail

arch=$1
shift
bin=$here/$arch/bin

AEGIS_PATH=$tmp/lib
export AEGIS_PATH

chmod 0777 $tmp/lib
test $? -eq 0 || fail

AEGIS_PROJECT=example
export AEGIS_PROJECT

USER=`whoami`
export USER

AEGIS_FLAGS="delete_file_preference = no_keep; \
        lock_wait_preference = always; \
        diff_preference = automatic_merge; \
        pager_preference = never; \
        persevere_preference = all; \
        log_file_preference = never;"

#
# Create a new project
#
$bin/aegis -new-project $AEGIS_PROJECT --version=- --dir=$tmp/proj.dir
test $? -eq 0 || fail

activity="project attributes 147"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$tmp";
fubar
test $? -eq 0 || fail

$bin/aegis -pa -f paf --project=$AEGIS_PROJECT
test $? -eq 0 || fail

#
# add project staff
#
$bin/aegis --new-dev -p $AEGIS_PROJECT $USER
test $? -eq 0 || fail

$bin/aegis --new-rev -p $AEGIS_PROJECT $USER
test $? -eq 0 || fail

$bin/aegis --new-int -p $AEGIS_PROJECT $USER
test $? -eq 0 || fail

AEGIS_CHANGE=1
export AEGIS_CHANGE

#
# Create a change set.
#
cat > caf << 'fubar'
brief_description = "minimum initial configuration";
description =
    "This change set creates the minimum Aegis configuration to get a "
    "small project up and running quickly.  It does not include any "
    "source files, as those are expected to be supplied by release "
    "tarballs imported using the aetar command."
    ;
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
test $? -eq 0 || fail

$bin/aegis --new-change -f caf \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE
test $? -eq 0 || fail

#
# Begin development of the change
#
$bin/aegis --develop-begin -dir $tmp/chan.dir -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Create the top-level aegis.conf file,
# pointing into the aegis.conf.d directory.
#
$bin/aegis -nf $tmp/chan.dir/aegis.conf -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

echo 'configuration_directory = "aegis.conf.d";' > $tmp/chan.dir/aegis.conf
test $? -eq 0 || fail

#
# Create the build file.  It says we don't do builds at all.
# We will have to wait until there is some content for that to be useful.
#
$bin/aegis -nf $tmp/chan.dir/aegis.conf.d/build -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

echo 'build_command = "exit 0";' > $tmp/chan.dir/aegis.conf.d/build
test $? -eq 0 || fail

#
# Add anything else given on the command line.
# (This includes history, diff, merge, etc.)
#
while [ $# -ge 1 ]
do
    case "$1" in
    output=*)
        output=`echo $1 | sed 's|.*=||'`
        ;;

    *=*)
        name=`echo $1 | sed 's|=.*||'`
        value=`echo $1 | sed 's|.*=||'`

        $bin/aegis -nf $tmp/chan.dir/$name -v -nolog \
            --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
            > log 2>&1
        if test $? -ne 0 ; then cat log; fail; fi

        cp $here/$value $tmp/chan.dir/$name
        test $? -eq 0 || fail
        ;;

    *)
        echo "$0 name=value ..." 1>&2
        fail
        ;;
    esac
    shift
done

if test -z "$output"
then
    echo "No output=[target] option given."
    fail
fi

$bin/aegis -diff -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -dev-end \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE
test $? -eq 0 || fail

# now integrate it

$bin/aegis -integrate-begin -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -diff -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -int-pass -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

# --------------------------------------------------------------------------

AEGIS_CHANGE=3
export AEGIS_CHANGE

#
# Now the second change, which adds the build.
#
cat > caf << 'fubar'
brief_description = "new file templates";
description =
    "This change set adds new file templates, so that all files will "
    "have the appropriate header.  It is also used to provide the "
    "infrastructure for tests."
    ;
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
test $? -eq 0 || fail

$bin/aegis --new-change -f caf \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE
test $? -eq 0 || fail

#
# Begin development of the change
#
$bin/aegis --dev-begin -dir $tmp/chan.dir.3 -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -nf $tmp/chan.dir.3/aegis.conf.d/file_template -v -nolog > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $tmp/chan.dir.3/aegis.conf.d/file_template << 'fubar'
file_template =
[
    {
        pattern = [ "*.[cyl]" ];
        body = "${read_file ${source aegis.file.template/c abs}}";
    },
    {
        pattern = [ "*.h" ];
        body = "${read_file ${source aegis.file.template/h abs}}";
    },
    {
        pattern = [ "test/*/*.sh" ];
        body = "${read_file ${source aegis.file.template/test.sh abs}}";
    },
    {
        pattern = [ "*.sh" ];
        body = "${read_file ${source aegis.file.template/sh abs}}";
    },
    {
        pattern = [ "*" ];
        body = "${read_file ${source aegis.file.template/generic abs}}";
    }
];

filename_pattern_reject =
[
    /* No Aegis-special file names. */
    "*,*",

    /* No C files with upper case letters in their names. */
    "*[A-Z]*.c"
    "*[A-Z]*.h"

    /* No C++ file names. */
    "*.C",
    "*.[cC][cC]",
    "*.[cC][xX][xX]",
    "*.H",
    "*.[hH][hH]",
];

posix_filename_charset = true;
fubar
test $? -eq 0 || fail

#
# now create the content templates
#
$bin/aegis --new-file \
        $tmp/chan.dir.3/aegis.file.template/c \
        $tmp/chan.dir.3/aegis.file.template/generic \
        $tmp/chan.dir.3/aegis.file.template/h \
        $tmp/chan.dir.3/aegis.file.template/sh \
        $tmp/chan.dir.3/aegis.file.template/test.sh \
        $tmp/chan.dir.3/aegis.supplementary/test_prelude.inc.sh \
        -no-template -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $tmp/chan.dir.3/aegis.file.template/c << 'fubar'
/*
 * ${project trunk_name} - ${project trunk_description}
 * Copyright (C) ${date %Y} ${copyright-owner}
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * you option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>
 */

#include <${trim_ext $fn}.h>


void
${id ${trim_ext $fn}}(void)
{
}
fubar
test $? -eq 0 || fail

cat > $tmp/chan.dir.3/aegis.file.template/generic << 'fubar'
#
# ${project trunk_name} - ${project trunk_description}
# Copyright (C) ${date %Y} ${copyright-owner}
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# you option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>
#
fubar
test $? -eq 0 || fail

cat > $tmp/chan.dir.3/aegis.file.template/h << 'fubar'
/*
 * ${project trunk_name} - ${project trunk_description}
 * Copyright (C) ${date %Y} ${copyright-owner}
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * you option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>
 */

#ifndef ${id ${upcase $fn}}
#define ${id ${upcase $fn}}

/**
  * The ${id ${trim_ext $fn}} function may be used to
  *
  * @param arg1
  *     insert description here (indented)
  * @returns
  *     explain return value here (indented)
  */
void ${id ${trim_ext $fn}}(void);

#endif /* ${id ${upcase $fn}} */
fubar
test $? -eq 0 || fail

cat > $tmp/chan.dir.3/aegis.file.template/sh << 'fubar'
#!/bin/sh
#
# ${project trunk_name} - ${project trunk_description}
# Copyright (C) ${date %Y} ${copyright-owner}
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# you option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>
#
exit 0
fubar
test $? -eq 0 || fail

cat > $tmp/chan.dir.3/aegis.file.template/test.sh << 'fubar'
#!/bin/sh
#
# ${project trunk_name} - ${project trunk_description}
# Copyright (C) ${date %Y} ${copyright-owner}
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# you option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>
#
TEST_SUBJECT="fill me in"

#
# The test prelude is common to all tests.  It creates a temporary
# diretcory and changes directory into it, so you can create temporary
# files to your heart's content.  It also loads our modules.  The
# functions "pass", "fail" and "no_result" are the only correct ways of
# leaving this test script.  The development directory is available via
# the $devdir variable, if you need data files or test programs.
#
# That dot means "include" in shell speak.  It has to be an include,
# executing it in a sub-shell will not work.
#
. aegis.supplementary/test_prelude.inc.sh

#
# Now test whatever it is you are testing.  Make sure you test ALL
# exit statuses, and report no_result for anything (anything at all)
# unexpected.
#
# Make sure that in the success case, your test is absolutely silent.
# Its tough enough figuring out what went wrong without having to wade
# through megabytes of verbal diarrhoea from tests which pass to find
# the one useful output indicating what failed.
#
$devdir/blah blah blah
test $? -eq 0 || fail

#
# All the things tested by this individual test passed.
# No other assertions are made.
#
pass
fubar
test $? -eq 0 || fail

cat > $tmp/chan.dir.3/aegis.supplementary/test_prelude.inc.sh << 'fubar'
#!/bin/sh
#
# ${project trunk_name} - ${project trunk_description}
# Copyright (C) ${date %Y} ${copyright-owner}
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# you option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>
#

if [ "$TEST_SUBJECT" = "fill me in" ]
then
    echo '    You must set the TEST_SUBJECT environment variable' 1>&2
    echo '    at the top of your test script to something' 1>&2
    echo '    descriptive.' 1>&2
    exit 2
fi

#
# Remember where we came from, so that we can refer back to it as necessary.
#
devdir=`pwd`
test $? -eq 0 || exit 2

#
# We are going to create a temporary for running tests within.  This
# makes it easy to clean up after tests (just rm -r) and it means tests
# can be run even if the development directory is read only (e.g. for
# reviewers).
#
testdir=/tmp/$$

#
# The tear_down function is used by all the ways of leaving a test
# script, so that they all clean up in exactly the same way.
#
tear_down()
{
    set +x
    cd $devdir
    rm -rf $testdir
}

#
# The pass function (command) is used to declare a test to have passed,
# and exit.  The exit code of 0 is dictated by Aegis, so Aegis can know
# the result of running the test.
#
# Note that we don't say what was being tested, because only failed
# tests are interesting, especially when your project get to the point
# of having hundreds of tests.
#
pass()
{
    tear_down
    echo PASSED
    exit 0
}

#
# The fail function (command) is used to declare a test to have failed,
# and exit.  The exit code of 1 is dictated by Aegis, so Aegis can know
# the result of running the test.
#
fail()
{
    tear_down
    echo "FAILED test of $TEST_SUBJECT"
    exit 1
}

#
# The no_result function (command) is used to declare a test to have
# failed in an unexpected way, and exit.  This is used for any case
# where the "scaffolding" of a test does not succeed, effectively making
# the correctedness of the functionality being tested indeterminate.
# The exit code of 2 is dictated by Aegis, so Aegis can know the result
# of running the test.
#
no_result()
{
    tear_down
    echo "NO RESULT for test of $TEST_SUBJECT"
    exit 2
}

#
# Create our testing directory and cd into it.
#
mkdir $testdir
test $? -eq 0 || exit 2
cd $testdir
test $? -eq 0 || no_result
fubar
test $? -eq 0 || fail

#
# Package the change set.
#
$bin/aedist -send -ndh -mh -naa -comp-alg=gzip -compat=4.6 \
        --ignore-uuid \
        --output="$here/$output" \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE

# --------------------------------------------------------------------------

cd $here
rm -rf $tmp

exit 0
