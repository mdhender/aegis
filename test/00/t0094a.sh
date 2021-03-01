#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2006-2008 Peter Miller
#	Copyright (C) 2002 John Darrington
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
	echo 'FAILED test of the history_label_command functionality' 1>&2
	echo $activity
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x

	echo 'NO RESULT when testing the history_label_command functionality' 1>&2
	echo $activity
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT

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
activity="new project 111"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 118"
cat > $tmp << 'end'
description = "A bogus project created to test the label_history \
functionality.";
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
activity="new change 133"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a second change
#
activity="new change 145"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 159"
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
activity="new file 176"
$bin/aegis -nf $workchan/main.c $workchan/test.c $workchan/Makefile \
	$workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.c << 'end'
int main() { test(); exit(0); return 0; }
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
build_command = "exit 0";
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

history_label_command =
/*	"rcs -n${SUBST \\\\. _ ${Label}}:${Edit} ${History}"; */
        "echo Labeling ${History} edit ${Edit} with t${SUBST \\\\. _ ${Label}}";

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
end
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 222"
$bin/aegis -nt > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
exit 0
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 235"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 242"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 249"
$bin/aegis -t -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 256"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 263"
$bin/aegis -rpass -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 270"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 277"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate test
#
activity="test 284"
$bin/aegis -t -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 291"
$bin/aegis -intpass -nl  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

grep "Labeling .* edit 1 with t_D001" log > /dev/null
if test $? -ne 0 ; then cat log; fail; fi

#
# start work on change 2
#
activity="develop begin 301"
$bin/aegis -db -c 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# copy a file into the change
#
activity="copy file 308"
$bin/aegis -cp $workchan/main.c -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# change the file
#
cat > $workchan/main.c << 'end'
#include <stdio.h>

int
main(argc, argv)
	int	argc;
	char	**argv;
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
activity="build 339"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 346"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish development of the change
#
activity="develop end 353"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 360"
$bin/aegis -rpass -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 367"
$bin/aegis -ib 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 374"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


#
# pass the integration
#
activity="integrate pass 382"
$bin/aegis -intpass -nl  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

grep "Labeling .* edit 1 with t_D002" log > /dev/null
if test $? -ne 0 ; then cat log; fail; fi
grep "Labeling .* edit 2 with t_D002" log > /dev/null
if test $? -ne 0 ; then cat log; fail; fi

#
# label the HEAD of the history
#
activity="label history head 394"
$bin/aegis -delta_name XyZZY  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

grep "Labeling .* edit 1 with tXyZZY" log > /dev/null
if test $? -ne 0 ; then cat log; fail; fi
grep "Labeling .* edit 2 with tXyZZY" log > /dev/null
if test $? -ne 0 ; then cat log; fail; fi

#
# label Delta 1  of the history
#
activity="label history delta 1 406"
$bin/aegis -delta_name 1 Foobar > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

grep "Labeling .* edit 1 with tFoobar" log > /dev/null
if test $? -ne 0 ; then cat log; fail; fi

#
# The things tested by this test passed.
# No other assertions are made.
#
pass
