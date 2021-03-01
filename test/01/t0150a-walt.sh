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
# MANIFEST: Test the aed functionality
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
AEGIS_THROTTLE=2
export AEGIS_THROTTLE
AEGIS_PROJECT=foo
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin=$here/${1-.}/bin

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
activity="new project 133"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 140"
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
activity="new change 154"
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
activity="staff 166"
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
activity="new file 183"
$bin/aegis -c 1 -nf $workchan/main.c $workchan/test.c $workchan/Makefile \
	$workchan/config -nl > log 2>&1
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
cat > $workchan/config << 'end'
build_command = "make -f \\${source Makefile}";
create_symlinks_before_build = true;
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
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";

end
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 233"
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
activity="build 245"
$bin/aegis -c 1 -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 252"
$bin/aegis -diff -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 259"
$bin/aegis -t -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the 1st change
#
activity="develop end 266"
$bin/aegis -de -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review of the 1st change
#
activity="review pass 273"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 280"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 287"
$bin/aegis -b -c 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate test
#
activity="test 294"
$bin/aegis -t -c 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 301"
$bin/aegis -intpass -c 1 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create the second change
#
activity="new change 308"
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

activity="new file 330"
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
activity="new build file 349"
$bin/aegis -c 2 -nf -build $workchan.2/foo
if test $? -ne 0 ; then cat log; no_result; fi

#
# build the 2nd change
#
activity="build 356"
$bin/aegis -c 2 -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# difference the change
#
activity="diff 363"
$bin/aegis -diff -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# 3rd change
#
activity="new change 370"
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
activity="new build file 391"
$bin/aegis -c 3 -nf -build $workchan.3/foo
if test $? -ne 0 ; then cat log; no_result; fi

#
# build the 3nd change
#
activity="build 398"
$bin/aegis -c 3 -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the 2nd change
#
activity="develop end 405"
$bin/aegis -de -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result ; fi

activity="review pass 2nd 409"
$bin/aegis -rpass -c 2 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="integrate begin 2nd 413"
$bin/aegis -ib -c 2 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result; fi

activity="integrate build 2nd 417"
$bin/aegis -b -c 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

activity="integrate diff 2nd 421"
$bin/aegis -diff -c 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

activity="integrate pass 2nd 425"
$bin/aegis -ipass -c 2 > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

#
# go ahead with the 3rd change
#

activity="diff 3rd change 433"
$bin/aegis -diff -c 3 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log ; no_result ; fi

activity="develop end 3rd 437"
$bin/aegis -de -c 3 > log 2>&1
if test $? -ne 0 ; then cat log ; fail ; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
