#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 2004, 2010 Walter Franzini
# Copyright (C) 2007, 2008, 2012 Peter Miller
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

TEST_SUBJECT="aed"

# load up standard prelude and test functions
. test_funcs

workproj=$work/proj
workchan=$work/chan
tmp=${work}/tmp

AEGIS_PROJECT=foo
export AEGIS_PROJECT

#
# make a new project
#
activity="new project 36"
aegis -npr ${AEGIS_PROJECT} -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 43"
cat > $tmp << 'end'
description = "A bogus project created to test the aepatch -send functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
if test $? -ne 0 ; then no_result; fi
aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 57"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
aegis -nc 1 -f $tmp -p ${AEGIS_PROJECT} > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 69"
aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 86"
aegis -c 1 -nf $workchan/main.c $workchan/test.c $workchan/Makefile \
        $workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.c << 'end'
int
main(argc, argv)
    int     argc;
    char    **argv;
{
    test();
    exit(0);
    return 0;
}
end
if test $? -ne 0 ; then no_result; fi
cat > $workchan/test.c << 'end'
void test() { }
end
if test $? -ne 0 ; then no_result; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`
sed "s/{TAB}/${TAB}/g" > $workchan/Makefile << 'end'
.c.o:
{TAB}date > $@

foo: main.o test.o
{TAB}date > $@
end
if test $? -ne 0 ; then no_result; fi
cat > $workchan/aegis.conf << 'end'
build_command = "make -f \\${source Makefile}";
create_symlinks_before_build = true;
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
activity="new test 138"
aegis -c 1 -nt > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
exit 0
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 150"
aegis -c 1 -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 157"
aegis -diff -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 164"
aegis -t -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the 1st change
#
activity="develop end 171"
aegis -de -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review of the 1st change
#
activity="review pass 178"
aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 185"
aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 192"
aegis -b -c 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate test
#
activity="test 199"
aegis -t -c 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 206"
aegis -intpass -c 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create the second change
#
activity="new change 213"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi
aegis -nc 2 -f $tmp -p ${AEGIS_PROJECT} > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
aegis -db 2 -dir $workchan.2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#

activity="new file 235"
aegis -c 2 -nf $workchan.2/dummy.c -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan.2/dummy.c << 'end'
int
dummy (argc, argv)
        int     argc;
        char    **argv;
{
        test();
        exit(0);
        return 0;
}
end
if test $? -ne 0 ; then no_result; fi

#
# new build file to 2nd change
#
activity="new build file 254"
aegis -c 2 -nf -build $workchan.2/foo
if test $? -ne 0 ; then cat log; no_result; fi

#
# build the 2nd change
#
activity="build 261"
aegis -c 2 -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 268"
aegis -diff -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# 3rd change
#
activity="new change 275"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi
aegis -nc 3 -f $tmp -p ${AEGIS_PROJECT} > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of the 3rd change
#
aegis -db 3 -dir $workchan.3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# new build file to 3rd change
#
activity="new build file 296"
aegis -c 3 -nf -build $workchan.3/foo
if test $? -ne 0 ; then cat log; no_result; fi

#
# build the 3nd change
#
activity="build 303"
aegis -c 3 -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the 2nd change
#
activity="develop end 310"
aegis -de -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result ; fi

activity="review pass 2nd 314"
aegis -rpass -c 2 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="integrate begin 2nd 318"
aegis -ib -c 2 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="integrate build 2nd 322"
aegis -b -c 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

activity="integrate diff 2nd 326"
aegis -diff -c 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

activity="integrate pass 2nd 330"
aegis -ipass -c 2 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

#
# go ahead with the 3rd change
#

activity="diff 3rd change 338"
aegis -diff -c 3 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

activity="develop end 3rd 342"
aegis -de -c 3 > log 2>&1
if test $? -ne 0 ; then cat log ; fail ; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
