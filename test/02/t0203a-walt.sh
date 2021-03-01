#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004 Walter Franzini;
#	All rights reserved.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: Test the aedist (aemv) functionality
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
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

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
	echo "FAILED test of the aedist functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
        echo $work
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

activity="create test directory 92"
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
# If the C++ compiler is called something other than ``c++'', as
# discovered by the configure script, create a shell script called
# ``c++'' which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
#
if test "$CXX" != "" -a "$CXX" != "c++"
then
	cat >> $work/c++ << fubar
#!/bin/sh
exec $CXX \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx $work/c++
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi


#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the aedist/aemv functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 148"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 155"
cat > tmp << 'end'
description = "A bogus project created to test the aedist/aemv "
    "functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 171"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# create a new change
#
activity="new change 184"
cat > tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


#
# add a new files to the change
#
activity="new files 203"
$bin/aegis -nf  $workchan/bogus1 -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/bogus2 -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/config -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus1 << 'end'
bogus1, line 1
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/bogus2 << 'end'
line one
line two
line three
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/config << 'end'
build_command = "exit 0";
link_integration_directory = true;
history_get_command =
	"co -u'$e' -p $h,v > $o";
history_create_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_put_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_query_command =
	"rlog -r $h,v | awk '/^head:/ {print $$2}'";
diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
merge_command = "(diff3 -e $i $orig $mr | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $i > $out";
patch_diff_command = "set +e; diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 248"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 255"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 262"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 269"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 276"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 283"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 290"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 297"
cat > tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
activity="develop begin 309"
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Use the second change to move bogus2 to bogus3 and bogus1 to bogus4
#
activity="move files 316"
$bin/aegis -c 2 -mv -baserel bogus2 bogus3 bogus1 bogus4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Modify bogus4
#
activity="modify bogus4 323"
cat > $workchan/bogus4 <<EOF
this file is now named bogus4
EOF
if test $? -ne 0 ; then no_result; fi

#
# difference the change
#
activity="aed 332"
$bin/aegis --diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# build the change
#
activity="build 339"
$bin/aegis -build 2 -nl -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Set the change uuid
#
activity="change uuid 346"
$bin/aegis -change-attr \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd0 -c 2 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="end the change 351"
$bin/aegis -de 2 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="send the outstanding change 355"
$bin/aedist -send -c 2 -ndh -out $work/c02dev.ae > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="review pass 359"
$bin/aegis -rpass 2 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

acctivity="integrate begin"
$bin/aegis -ibegin 2 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build the change 367"
$bin/aegis -build 2 -verb -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 371"
$bin/aegis -diff 2 -verb -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate the change 375"
$bin/aegis -ipass 2 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="send the completed change 379"
$bin/aedist -send -c 2 -ndh -out $work/c02.ae > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

cat > $work/ok.ae <<EOF
MIME-Version: 1.0
Content-Type: application/aegis-change-set
Content-Transfer-Encoding: base64
Subject: example - The second change
Content-Name: example.C002.ae
Content-Disposition: attachment; filename=example.C002.ae

H4sIAAAAAAAAA7WUS2+jMBCAueJf4fQY1SomtIkU9bCHHirtqepttULGTMArsJFtmkqr/vfa
QKIG9QGNOlKYMfP68NiJ1tE6oriXTm8oS4Z1NNJ0tN58EDfS9JgHll81Wv0DbolkNQQInlnd
VICitxzxTI54Ikf8loOXTBZAZFtnoIMYoVOG1SyGVcymMeDdOwwGbOD6Z1rALs3BcC0aK5TE
t/jisQRsgCuZ4z76YosmhHDWGnBOIS1oyao0a4stsmBsCs9QN9b5rG5heJcxA5WQMHZqKDQY
4xql7+Uay6zvwvZMWCELB/8ElWpqkHaL2lbkno8NQjInJPGPDeecuB/Pe4kcstEc36I/KPyP
wnAnKkj9EfEFMlW0hrqQMJxRs09gfNgmDbV6gq6GYYWva1SreffGe46NEp/3cvkRRzyXI/4m
x+pzjtWZHFyDG94EjvhzjuTMuUzmoAPH3+34riYz/y+m6vywbpjl5VW/7cFp7+t5vWk+sXd2
WLtrceyMkL+kWEnoDbtXg1FqADTel5t5bDf5GfuSBGi5XOLeRoSQg7k8lS6K4s5c9DH0Encf
Qbs0it2TOJ8thcH+vGGnpdpjf+zyQ9nxt65nzgG+PYfEz+EruBO26IfO55Ht8eHX/e+7h8Vi
4dleAc4uwz3UBwAA
EOF

activity="test archive of outstanding change 403"
cmp $work/c02dev.ae $work/ok.ae > log
if test $? -ne 0 ; then cat log; fail ; fi

activity="test archive of complete change 407"
cmp $work/c02.ae $work/ok.ae > log
if test $? -ne 0 ; then cat log; fail ; fi

#
# Only definite negatives are possibe.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
