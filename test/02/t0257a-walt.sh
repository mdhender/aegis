#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2007, 2008 Walter Franzini
#	Copyright (C) 2008 Peter Miller
#
#       This program is free software; you can redistribute it and/or
#       modify it under the terms of the GNU General Public License as
#       published by the Free Software Foundation; either version 3 of
#       the License, or (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public
#       License along with this program; if not, see
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
	echo "FAILED test of the loop detection functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the loop detection functionality " \
	    "($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory 115"
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
# test the ??? functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PATH=$work/lib ; export AEGIS_PATH
AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 162"
$bin/aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj -lib $work/lib \
    > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 170"
cat > tmp << 'end'
description = "A bogus project created to test aegis";
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
activity="staff 186"
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
activity="new change 199"
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
activity="new file 218"
$bin/aegis -nf $workchan/bogus1 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nf  $workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus1 << 'end'
bogus1, line 1
end
if test $? -ne 0 ; then no_result; fi


cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;
merge_command = "(diff3 -e $i $orig $mr | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $i > $out";
patch_diff_command = "set +e; $diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

activity="finish the change 248"
$bin/aefinish -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate begin 252"
$bin/aegis -ib -c 1 -v -nl > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the change 256"
$bin/aefinish -c 1 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="the second change 260"
sleep 1
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
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


#
# rename bogus1
#
activity="rename bogus1  280"
$bin/aegis -mv $workchan/bogus1 $workchan/bogus2 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat >> $work/bogus2 <<EOF
random stuff (c2)
EOF
if test $? -ne 0 ; then no_result; fi

activity="finish the change 289"
$bin/aefinish -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate begin 293"
$bin/aegis -ib -c 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the 2nd change 297"
$bin/aefinish -p $AEGIS_PROJECT -c 2 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# The 3rd change
#
activity="the third change 304"
cat > tmp << 'end'
brief_description = "The third change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 3 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


#
# rename bogus2
#
activity="rename bogus1  323"
$bin/aegis -mv $workchan/bogus2 $workchan/bogus3 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat >> $work/bogus3 <<EOF
random stuff (c3)
EOF
if test $? -ne 0 ; then no_result; fi

activity="finish the change 332"
$bin/aefinish -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate begin 336"
$bin/aegis -ib -c 3 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the 3rd change 340"
$bin/aefinish -p $AEGIS_PROJECT -c 3 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# The 4th change
#
activity="the third change 347"
cat > tmp << 'end'
brief_description = "The fourth change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 4 -f tmp -p example > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 4 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


#
# remove bogus3
#
activity="rename bogus1  366"
$bin/aegis -rm  $workchan/bogus3 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="finish the change 370"
$bin/aefinish -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate begin 374"
$bin/aegis -ib -c 4 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="integrate the 4th change 378"
$bin/aefinish -p $AEGIS_PROJECT -c 4 -v > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

#
# Set the maximum execution time to 30 seconds.
# If the ael cf command take more than 30 seconds we assume to have
# catch a loop.
#
ulimit -t 30

activity="list change files 389"
$bin/aegis -l cf -p $AEGIS_PROJECT -c 4 -unf > $work/list 2>log
if test $? -ne 0; then cat log; fail; fi

activity="expected output 393"
cat > $work/ok <<EOF
source remove 3 bogus3
EOF
if test $? -ne 0; then no_result; fi

activity="check ael cf output 399"
diff $work/ok $work/list
if test $? -ne 0; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
