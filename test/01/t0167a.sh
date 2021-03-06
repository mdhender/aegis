#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 2002, 2004-2008, 2011, 2012 Peter Miller
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

if test -z "`diff -v 2>&1 | grep GNU`"
then
    echo ''
    echo '        This test depends on GNU Diff, which you do not seem to'
    echo '        have installed.  This test is declared to pass by default.'
    echo ''
    exit 0
fi

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
    echo "NO RESULT for test of aepatch -send (uni) functionality ($activity)" 1>&2
    cd $here
    find $work -type d -user $USER -exec chmod u+w {} \;
    rm -rf $work
    exit 2
}
fail()
{
    set +x
    echo "FAILED test of aepatch -send (uni) functionality ($activity)" 1>&2
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
activity="working directory 121"
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
activity="new project 140"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 147"
cat > $tmp << 'end'
description = "A bogus project created to test the aepatch -send (uni) functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 161"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 173"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 190"
$bin/aegis -nf $workchan/main.c -nl \
    -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf $workchan/test.c -nl \
    -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf $workchan/Makefile -nl \
    -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf $workchan/aegis.conf -nl \
    -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd4 > log 2>&1
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
    /* cough */
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
patch_diff_command = "set +e; diff -U0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 251"
$bin/aegis -nt -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd6 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
exit 0
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 263"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 270"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 277"
$bin/aegis -t -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd7 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 287"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 294"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 301"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 308"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate test
#
activity="test 315"
$bin/aegis -t -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 322"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change, to generate a patch
#
activity="new change 329"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="copy file 347"
$bin/aegis -cp $workchan/main.c -c 2 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.c << 'end'
int
main(argc, argv)
    int argc;    /* changed */
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
# Now use the change to generate a patch.
#
aepatch -send 2 -o the.patch -compat=4.21 -cte=none -nocomp > log 2>&1
if test $? -ne 0 ; then no_result; fi

#
# And check the the patch is OK.
#
TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`
sed -e "s|{TAB}|$TAB|g" -e 's/X$//' > ok << 'end'
MIME-Version: 1.0X
Content-Type: application/aegis-patchX
Subject: foo - The second changeX
Content-Name: foo.C002.patchX
Content-Disposition: attachment; filename=foo.C002.patchX
X-Aegis-Project-Name: fooX
X-Aegis-Change-Number: 2X
X
#{TAB}X
#{TAB}Summary:
#{TAB}The second change
#{TAB}X
#{TAB}Description:
#{TAB}The second changeX
#{TAB}X
# Aegis-Change-Set-BeginX
# H4sIAAAAAAAAA42PwUoFMQxF9/2KMmtHEFwIw/sLdyJDJs30Bdp0aNqnIv67Kc5G3HghWeTcX
# NLdbZdrXQIqVj8ZF/MVPz1fySlgkeLyCRJoW9w8LQlcyyNKoCqR163FxjbSt9E75aMZ2SErnX
# cAOlxEJ/aKVYSdVOrb+3W+2GtUEbd+ANuLFEi3+jVI5M0oxW9Bf34j7dzolWgTy8UwaWe7SUX
# vXMYAzg1b6b5cbQnRJytMPzoweyA55dzCbx/2L5CHC9q6RUtzdede13cN97PYDNIAQAAX
# Aegis-Change-Set-EndX
#X
Index: main.cX
--- main.c
+++ main.c
@@ -3 +3 @@
-    int argc;
+    int argc;    /* changed */
@@ -5,0 +6,5 @@
+    if (argc != 1)
+    {
+        fprintf(stderr, "usage: %s\n", argv[0]);
+        exit(1);
+    }
@@ -9 +13,0 @@
-    /* cough */
end
if test $? -ne 0 ; then no_result; fi

diff -b ok the.patch
if test $? -ne 0 ; then fail; fi

#
# receive the patch
#
activity="aepatch receive 419"
$bin/aepatch -receive -dir ${workchan}3 -f the.patch -trojan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Make sure the patch is applied properly.
#
activity="verify 426"
diff -b $workchan/main.c ${workchan}3/main.c
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass


# vim: set ts=8 sw=4 et :
