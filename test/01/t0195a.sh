#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 2004-2008, 2011, 2012 Peter Miller
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
unset LINES
unset COLS
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$

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

no_result()
{
    set +x
    echo "NO RESULT for test of aepatch -send functionality ($activity)" 1>&2
    cd $here
    find $work -type d -user $USER -exec chmod u+w {} \;
    rm -rf $work
    exit 2
}
fail()
{
    set +x
    echo "FAILED test of aepatch -send functionality ($activity)" 1>&2
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

#
# make a new project
#
activity="new project 115"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 122"
cat > $tmp << 'end'
description = "A bogus project created to test the aepatch -send functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 136"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create anew branch
#
$bin/aegis -nbr -p $AEGIS_PROJECT 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=foo.1 ; export AEGIS_PROJECT

# --------------------------------------------------------------------------
#
# create a new change
#
activity="new change 156"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 174"
$bin/aegis -nf $workchan/main.c $workchan/test.c $workchan/Makefile \
    $workchan/nothingmuch \
    $workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.c << 'end'
int
main(argc, argv)
    int argc;
    char **argv;
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
sed "s|{TAB}|$TAB|g" > $workchan/Makefile << 'end'
.c.o:
{TAB}date > $@

foo: main.o test.o
{TAB}date > $@
end
if test $? -ne 0 ; then no_result; fi
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
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
    echo '1,$$p' ) | ed - $mr > $out";
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/nothingmuch << 'end'
these lines
are the ones
to be
deleted
end
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 231"
$bin/aegis -nt > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
exit 0
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 243"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 250"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 257"
$bin/aegis -t -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 264"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 271"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 278"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 285"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 289"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate test
#
activity="test 296"
$bin/aegis -t -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 303"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------
#
# create a second change
#
activity="new change 311"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f $tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start work on change 2
#
activity="develop begin 325"
$bin/aegis -db -c 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# copy a file into the change
#
activity="copy file 332"
$bin/aegis -cp $workchan/main.c -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="remove file 336"
$bin/aegis -rm $workchan/nothingmuch -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# change the file
#
cat > $workchan/main.c << 'end'
#include <stdio.h>

int
main(argc, argv)
    int argc;
    char **argv;
{
    if (argc != 1)
    {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(1);
    }
    test();
    exit(0);
    return 0;
}
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 366"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 373"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 380"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 387"
$bin/aegis -rpass -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 394"
$bin/aegis -ib 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 401"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 405"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate test
#
activity="test 412"
$bin/aegis -t -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 419"
$bin/aegis -intpass 2 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------
#
# End the branch.
#
AEGIS_PROJECT=foo ; export AEGIS_PROJECT

#
# finish development of the change
#
activity="develop end 432"
$bin/aegis -de 1 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 439"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 446"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 453"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate test
#
activity="test 460"
$bin/aegis -t -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 467"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------
#
# build a distribution set
#
# The bugs we are testing for would build an empty change set
# for a completed change on a completed branch.
#
activity="send 478"
$bin/aepatch -send -p foo.1 -c 2 -o test.out -naa -nocomp > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# compare the patch with expected
#
activity="compare with expected 485"
sed -e "s|{TAB}|$TAB|g" -e 's/X$//' > test.ok << 'end'
MIME-Version: 1.0X
Content-Type: application/aegis-patchX
Subject: foo.1 - The second changeX
Content-Name: foo.1.C002.patchX
Content-Disposition: attachment; filename=foo.1.C002.patchX
X-Aegis-Project-Name: foo.1X
X-Aegis-Change-Number: 2X
X
#X
#{TAB}Summary:
#{TAB}The second change
#X
#{TAB}Description:
#{TAB}The second changeX
#X
#X
Index: main.cX
--- main.cX
+++ main.cX
@@ -1,8 +1,15 @@X
+#include <stdio.h>X
+X
 intX
 main(argc, argv)X
     int argc;X
     char **argv;X
 {X
+    if (argc != 1)X
+    {X
+        fprintf(stderr, "usage: %s\n", argv[0]);X
+        exit(1);X
+    }X
     test();X
     exit(0);X
     return 0;X
Index: nothingmuchX
--- nothingmuchX
+++ nothingmuchX
@@ -1,4 +0,0 @@X
-these linesX
-are the onesX
-to beX
-deletedX
end
if test $? -ne 0 ; then no_result; fi

sed '/Aegis-Change-Set-Begin/,/Aegis-Change-Set-End/d' < test.out > test.out2
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out2
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass


# vim: set ts=8 sw=4 et :
