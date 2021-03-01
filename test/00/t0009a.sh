#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 1991-2008, 2012 Peter Miller
#       Copyright (C) 2008 Walter Franzini
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

work=${AEGIS_TMP:-/tmp}/$$

here=`pwd`
if test $? -ne 0; then exit 2; fi

bin=$here/${1-.}/bin

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
        echo "FAILED test of -New_ReLeaSe functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT for test of -New_ReLeaSe functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
}
trap "no_result" 1 2 3 15

check_it()
{
        sed     -e "s|$work|...|g" \
                -e 's|= 0; /.*|= TIME_NOT_SET;|' \
                -e 's|= [0-9][0-9]*; /.*|= TIME;|' \
                -e "s/\"$USER\"/\"USER\"/g" \
                -e 's/19[0-9][0-9]/YYYY/' \
                -e 's/20[0-9][0-9]/YYYY/' \
                -e 's/crypto = ".*"/crypto = "GUNK"/' \
                -e 's/uuid = ".*"/uuid = "UUID"/' \
                < $1 > $work/sed.out
        if test $? -ne 0; then no_result; fi
        diff -b $2 $work/sed.out
        if test $? -ne 0; then fail; fi
}

#
# some variable to make things earier to read
#
worklib=$work/lib
workproj=$work/foo.proj
workproj2=$work/bar.proj
workchan=$work/foo.chan
tmp=$work/tmp

#
# make the directories
#
activity="create working directory 110"
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
        cat >> c++ << fubar
#!/bin/sh
exec ${CXX-g++} \$*
fubar
        if test $? -ne 0 ; then no_result; fi
        chmod a+rx c++
        if test $? -ne 0 ; then no_result; fi
        PATH=${work}:${PATH}
        export PATH
fi

#
# make a new project
#       and check files it should have made
#
activity="create new project 149"
$bin/aegis -newpro foo -version "" -dir $workproj -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# change project attributes
#
cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
$bin/aegis -proatt -f $tmp -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# create a new change
#
activity="create new change 168"
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
$bin/aegis -new_change 1 -f $tmp -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add a new developer
#
activity="add staff 180"
$bin/aegis -newdev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# begin development of a change
#
$bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add a new files to the change
#
activity="add files to change 193"
$bin/aegis -new_file $workchan/main.cc -nl -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
$bin/aegis -new_file $workchan/aegis.conf -nl -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
cat > $workchan/main.cc << 'end'
int
main(int argc, char **argv)
{
        return 0;
}
end
if test $? -ne 0 ; then no_result; fi
cat > $workchan/aegis.conf << 'end'
build_command = "rm -f foo; c++ -o foo -D'VERSION=\"$v\"' main.cc";
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";

diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
        echo '1,$$p' ) | ed - $mr > $out";
end
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 229"
$bin/aegis -nt -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
pass()
{
        exit 0
}
fail()
{
        echo SHUZBUTT 1>&2
        exit 1
}
no_result()
{
        echo WHIMPER 1>&2
        exit 2
}
trap "no_result" 1 2 3 15

./foo
q=$?

# check for signals
if test $q -ge 128
then
        no_result
fi

# should not complain
if test $q -ne 0
then
        fail
fi

# it probably worked
pass
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 273"
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# difference the change
#
activity="diff 280"
$bin/aegis -diff -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# test the change
#
activity="test 287"
$bin/aegis -test -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# finish development of the change
#
activity="develop end 294"
$bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then no_result; fi

#
# add a new reviewer
#
activity="new reviewer 301"
$bin/aegis -newrev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# pass the review
#
activity="review pass 308"
$bin/aegis -review_pass -chan 1 -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add an integrator
#
activity="new integrator 315"
$bin/aegis -newint $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# start integrating
#
activity="integrate begin 322"
$bin/aegis -intbeg 1 -p foo -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="integrate build 329"
$bin/aegis -build -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
activity="integrate test 332"
$bin/aegis -test -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# pass the integration
#
activity="integrate pass 339"
$bin/aegis -intpass -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi

#
# make it look like an old-style project
#
activity="fake old style project 346"
cat >> $workproj/info/state << 'fubar'
version_major = 1;
version_minor = 0;
fubar
if test $? -ne 0 ; then no_result; fi

#
# create the new release
#
activity="new release 356"
$bin/aegis -nrls -l -lib $worklib > test.out 2>&1
if test $? -ne 0 ; then cat test.out; no_result; fi
$bin/aegis -nrls foo bar -dir $workproj2 -lib $worklib -nl > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi

#
# check that files look as they should
#
activity="check change 1.1.10 state 365"
cat > ok << 'fubar'
brief_description = "New release derived from foo.";
description = "New release derived from foo.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
        "unspecified",
];
copyright_years =
[
        YYYY,
];
state = completed;
delta_number = 1;
history =
[
        {
                when = TIME;
                what = new_change;
                who = "USER";
        },
        {
                when = TIME;
                what = develop_begin;
                who = "USER";
        },
        {
                when = TIME;
                what = develop_end;
                who = "USER";
        },
        {
                when = TIME;
                what = review_pass;
                who = "USER";
        },
        {
                when = TIME;
                what = integrate_begin;
                who = "USER";
        },
        {
                when = TIME;
                what = integrate_pass;
                who = "USER";
        },
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001.branch/0/010 ok

activity="check change 1.1.10 file state 420"
cat > ok << 'fubar'
src =
[
        {
                file_name = "aegis.conf";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                };
                usage = config;
        },
        {
                file_name = "main.cc";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                };
                usage = source;
        },
        {
                file_name = "test/00/t0001a.sh";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                };
                usage = test;
        },
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001.branch/0/010.fs ok

activity="check branch 1.1 state 462"
cat > ok << 'fubar'
brief_description = "A bogus project created to test things, branch 1.1.";
description = "A bogus project created to test things, branch 1.1.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
        "unspecified",
];
copyright_years =
[
        YYYY,
];
state = being_developed;
development_directory = "branch.1/branch.1";
history =
[
        {
                when = TIME;
                what = new_change;
                who = "USER";
        },
        {
                when = TIME;
                what = develop_begin;
                who = "USER";
        },
];
branch =
{
        umask = 022;
        developer_may_review = true;
        developer_may_integrate = true;
        reviewer_may_integrate = true;
        developers_may_create_changes = false;
        default_test_exemption = false;
        default_test_regression_exemption = true;
        skip_unlucky = false;
        compress_database = false;
        develop_end_action = goto_being_reviewed;
        history =
        [
                {
                        delta_number = 1;
                        change_number = 10;
                        when = TIME;
                        is_a_branch = no;
                },
        ];
        change =
        [
                10,
        ];
        administrator =
        [
                "USER",
        ];
        developer =
        [
                "USER",
        ];
        reviewer =
        [
                "USER",
        ];
        integrator =
        [
                "USER",
        ];
        minimum_change_number = 10;
        reuse_change_numbers = true;
        minimum_branch_number = 1;
        protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001 ok

activity="check branch 1.1 file state 540"
cat > ok << 'fubar'
src =
[
        {
                file_name = "aegis.conf";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                };
                usage = config;
                file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
                diff_file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
        },
        {
                file_name = "main.cc";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                };
                usage = source;
                file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
                diff_file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
                test =
                [
                        "test/00/t0001a.sh",
                ];
        },
        {
                file_name = "test/00/t0001a.sh";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                };
                usage = test;
                file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
                diff_file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
        },
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001.fs ok

activity="check branch 1 state 637"
cat > ok << 'fubar'
brief_description = "A bogus project created to test things, branch 1.";
description = "A bogus project created to test things, branch 1.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
        "unspecified",
];
state = being_developed;
development_directory = "branch.1";
history =
[
        {
                when = TIME;
                what = new_change;
                who = "USER";
        },
        {
                when = TIME;
                what = develop_begin;
                who = "USER";
        },
];
branch =
{
        umask = 022;
        developer_may_review = true;
        developer_may_integrate = true;
        reviewer_may_integrate = true;
        developers_may_create_changes = false;
        default_test_exemption = false;
        default_test_regression_exemption = true;
        skip_unlucky = false;
        compress_database = false;
        develop_end_action = goto_being_reviewed;
        change =
        [
                1,
        ];
        sub_branch =
        [
                1,
        ];
        administrator =
        [
                "USER",
        ];
        developer =
        [
                "USER",
        ];
        reviewer =
        [
                "USER",
        ];
        integrator =
        [
                "USER",
        ];
        minimum_change_number = 10;
        reuse_change_numbers = true;
        minimum_branch_number = 1;
        protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001 ok

activity="check branch 1 file state 708"
cat > ok << 'fubar'
src =
[
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.fs ok

activity="check project state 717"
cat > ok << 'fubar'
next_test_number = 2;
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/state ok

activity="check trunk state 724"
cat > ok << 'fubar'
brief_description = "A bogus project created to test things.";
description = "A bogus project created to test things.";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = false;
regression_test_exempt = true;
architecture =
[
        "unspecified",
];
version_previous = "1.0.D001";
state = being_developed;
development_directory = ".";
history =
[
        {
                when = TIME;
                what = new_change;
                who = "USER";
        },
        {
                when = TIME;
                what = develop_begin;
                who = "USER";
        },
];
branch =
{
        umask = 022;
        developer_may_review = true;
        developer_may_integrate = true;
        reviewer_may_integrate = true;
        developers_may_create_changes = false;
        default_test_exemption = false;
        default_test_regression_exemption = true;
        skip_unlucky = false;
        compress_database = false;
        develop_end_action = goto_being_reviewed;
        change =
        [
                1,
        ];
        sub_branch =
        [
                1,
        ];
        administrator =
        [
                "USER",
        ];
        developer =
        [
                "USER",
        ];
        reviewer =
        [
                "USER",
        ];
        integrator =
        [
                "USER",
        ];
        minimum_change_number = 10;
        reuse_change_numbers = true;
        minimum_branch_number = 1;
        protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/trunk ok

activity="check trunk file state 796"
cat > ok << 'fubar'
src =
[
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/trunk.fs ok

#
# create a second change
#       make sure it creates the files it should
#
activity="new change 809"
cat > $tmp << 'end'
brief_description = "Second change of second project";
cause = internal_enhancement;
end
$bin/aegis -new_change 2 -f $tmp -project bar.1.1 -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# start work on change 2 of bar.1.1
#
activity="develop begin 820"
$bin/aegis -devbeg 2 -p bar.1.1 -dir $workchan -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# copy a file into the change
#
activity="copy file 827"
$bin/aegis -cp $workchan/main.cc -nl -lib $worklib -p bar.1.1
if test $? -ne 0 ; then no_result; fi

#
# check file contents
#
activity="check file contents 834"
cat > ok << 'fubar'
brief_description = "Second change of second project";
description = "Second change of second project";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = false;
regression_test_exempt = true;
architecture =
[
        "unspecified",
];
copyright_years =
[
        YYYY,
];
state = being_developed;
given_regression_test_exemption = true;
project_file_command_sync = 8410;
development_directory = ".../foo.chan";
history =
[
        {
                when = TIME;
                what = new_change;
                who = "USER";
        },
        {
                when = TIME;
                what = develop_begin;
                who = "USER";
        },
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001.branch/0/002 ok

activity="check file contents 871"
cat > ok << 'fubar'
src =
[
        {
                file_name = "main.cc";
                uuid = "UUID";
                action = modify;
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                };
                usage = source;
        },
];
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001.branch/0/002.fs ok

activity="check file contents 891"
cat > ok << 'fubar'
brief_description = "A bogus project created to test things, branch 1.1.";
description = "A bogus project created to test things, branch 1.1.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
        "unspecified",
];
copyright_years =
[
        YYYY,
];
state = being_developed;
development_directory = "branch.1/branch.1";
history =
[
        {
                when = TIME;
                what = new_change;
                who = "USER";
        },
        {
                when = TIME;
                what = develop_begin;
                who = "USER";
        },
];
branch =
{
        umask = 022;
        developer_may_review = true;
        developer_may_integrate = true;
        reviewer_may_integrate = true;
        developers_may_create_changes = false;
        default_test_exemption = false;
        default_test_regression_exemption = true;
        skip_unlucky = false;
        compress_database = false;
        develop_end_action = goto_being_reviewed;
        history =
        [
                {
                        delta_number = 1;
                        change_number = 10;
                        when = TIME;
                        is_a_branch = no;
                },
        ];
        change =
        [
                2,
                10,
        ];
        administrator =
        [
                "USER",
        ];
        developer =
        [
                "USER",
        ];
        reviewer =
        [
                "USER",
        ];
        integrator =
        [
                "USER",
        ];
        minimum_change_number = 10;
        reuse_change_numbers = true;
        minimum_branch_number = 1;
        protect_development_directory = false;
};
fubar
if test $? -ne 0 ; then no_result; fi
check_it $workproj2/info/change/0/001.branch/0/001 ok

#
# the things tested in this test, worked
#
pass
# vim: set ts=8 sw=4 et :
