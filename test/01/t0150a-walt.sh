#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004 Walter Franzini;
#	All rights reserved.
#       Copyright (C) 2007, 2008 Peter Miller
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
workchan=${work}/foo.chan
workproj=${work}/foo.proj
tmp=${work}/tmp
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
AEGIS_PROJECT=foo
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

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
	echo 'FAILED test of the aed functionality ($activity)' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the aed functionality ($activity)' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

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
# test the aed functionality
#


#
# make a new project
#
activity="new project 114"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 121"
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
# create a new change
#
activity="new change 135"
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
activity="staff 147"
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
activity="new file 164"
$bin/aegis -c 1 -nf $workchan/main.c $workchan/test.c $workchan/Makefile \
	$workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.c << 'end'
int
main(argc, argv)
	int	argc;
	char	**argv;
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
cat > $workchan/Makefile << 'end'
.c.o:
	date > $@

foo: main.o test.o
	date > $@
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
activity="new test 214"
$bin/aegis -c 1 -nt > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
exit 0
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 226"
$bin/aegis -c 1 -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 233"
$bin/aegis -diff -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 240"
$bin/aegis -t -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the 1st change
#
activity="develop end 247"
$bin/aegis -de -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review of the 1st change
#
activity="review pass 254"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 261"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 268"
$bin/aegis -b -c 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate test
#
activity="test 275"
$bin/aegis -t -c 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 282"
$bin/aegis -intpass -c 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create the second change
#
activity="new change 289"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 2 -dir $workchan.2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#

activity="new file 311"
$bin/aegis -c 2 -nf $workchan.2/dummy.c -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan.2/dummy.c << 'end'
int
dummy (argc, argv)
	int	argc;
	char	**argv;
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
activity="new build file 330"
$bin/aegis -c 2 -nf -build $workchan.2/foo
if test $? -ne 0 ; then cat log; no_result; fi

#
# build the 2nd change
#
activity="build 337"
$bin/aegis -c 2 -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 344"
$bin/aegis -diff -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# 3rd change
#
activity="new change 351"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of the 3rd change
#
$bin/aegis -db 3 -dir $workchan.3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# new build file to 3rd change
#
activity="new build file 372"
$bin/aegis -c 3 -nf -build $workchan.3/foo
if test $? -ne 0 ; then cat log; no_result; fi

#
# build the 3nd change
#
activity="build 379"
$bin/aegis -c 3 -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the 2nd change
#
activity="develop end 386"
$bin/aegis -de -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result ; fi

activity="review pass 2nd 390"
$bin/aegis -rpass -c 2 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="integrate begin 2nd 394"
$bin/aegis -ib -c 2 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="integrate build 2nd 398"
$bin/aegis -b -c 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

activity="integrate diff 2nd 402"
$bin/aegis -diff -c 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

activity="integrate pass 2nd 406"
$bin/aegis -ipass -c 2 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

#
# go ahead with the 3rd change
#

activity="diff 3rd change 414"
$bin/aegis -diff -c 3 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

activity="develop end 3rd 418"
$bin/aegis -de -c 3 > log 2>&1
if test $? -ne 0 ; then cat log ; fail ; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
