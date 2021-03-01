#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2005-2008 Peter Miller
#	Copyright (C) 2004, 2005 Walter Franzini;
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
	log_file_preference = never;"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

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

activity="create test directory 93"
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
activity="new project 149"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 156"
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
activity="staff 172"
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
activity="new change 185"
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
# add new files to the change
#
activity="new file 204"
$bin/aegis -nf  $workchan/bogus1 -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/bogus2 -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/aegis.conf -nl \
        --uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddddd3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus1 << 'end'
bogus1, line 1
bogus1, line 2
bogus1, line 3
bogus1, line 4
bogus1, line 5
bogus1, line 6
bogus1, line 7
bogus1, line 8
bogus1, line 9
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/bogus2 << 'end'
line one
line two
line three
line four
line five
line six
line seven
line eight
line nine
line ten
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

diff_command = "set +e; $diff $orig $i > $out; test $$? -le 1";
merge_command = "(diff3 -e $i $orig $mr | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $i > $out";
patch_diff_command = "set +e; $diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 264"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 271"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 278"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 285"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 292"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 299"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 306"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 313"
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
activity="develop begin 325"
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Use the second change to move bogus2 to bogus3 and bogus1 to bogus4
#
activity="move files 332"
$bin/aegis -c 2 -mv -baserel bogus2 bogus3 bogus1 bogus4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Modify bogus4
#
activity="modify bogus4 339"
cat > $workchan/bogus4 <<EOF
bogus1, line 1
bogus1, line 2
bogus1, line 3
bogus1, line 4
bogus1, line 5
bogus1, line 6
bogus1, line 7
bogus1, line 8
BOGUS1, LINE 90
this file is now named bogus4
EOF
if test $? -ne 0 ; then no_result; fi

#
# difference the change
#
activity="aed 357"
$bin/aegis --diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# build the change
#
activity="build 364"
$bin/aegis -build 2 -nl -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="send the outstanding change 368"
$bin/aedist -send -c 2 -ndh -out $work/c02dev.ae > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

# -----------------------------------------------------------------

activity="remove 2nd change 374"
$bin/aegis -dbu -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 381"
cat > tmp << 'end'
brief_description = "The third change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 390"
$bin/aegis -db 3 -dir $workchan.3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="file copy 394"
$bin/aegis -cp -c 3 $workchan.3/bogus1 $workchan.3/bogus2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify bogus1 398"
cat > $workchan.3/bogus1 <<EOF
bogus1, LINE 10
bogus1, line 2
bogus1, line 3
bogus1, line 4
bogus1, line 5
bogus1, line 6
bogus1, line 7
bogus1, line 8
bogus1, line 9
EOF
if test $? -ne 0 ; then no_result; fi

activity="modify bogus2 412"
cat > $workchan.3/bogus2 <<EOF
line one
line two
line three
line four
line five
line six
line seven
line eight
line nine
LINE TEN
EOF
if test $? -ne 0 ; then no_result; fi

activity="build the change 427"
$bin/aegis -build 3 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 431"
$bin/aegis -diff 3 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 435"
$bin/aegis -dev_end 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 439"
$bin/aegis -rpass 3 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 443"
$bin/aegis -ibegin 3 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build the change 447"
$bin/aegis -build 3 -verb -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 451"
$bin/aegis -diff 3 -verb -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate the change 455"
$bin/aegis -ipass 3 -verb > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="receive the change 459"
$bin/aedist -r -f $work/c02dev.ae -c 4 -dir $workchan.4 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check bogus3 463"
cat > ok <<EOF
line one
line two
line three
line four
line five
line six
line seven
line eight
line nine
LINE TEN
EOF
diff -b ok $workchan.4/bogus3 > log
if test $? -ne 0; then cat log ; fail; fi

activity="check bogus4 479"
cat > ok <<EOF
bogus1, LINE 10
bogus1, line 2
bogus1, line 3
bogus1, line 4
bogus1, line 5
bogus1, line 6
bogus1, line 7
bogus1, line 8
BOGUS1, LINE 90
this file is now named bogus4
EOF
diff -b ok $workchan.4/bogus4 > log
if test $? -ne 0; then cat log ; fail; fi


#
# Only definite negatives are possibe.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
