#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994-1999, 2002, 2004-2008 Peter Miller
#       Copyright (C) 2005 Walter Franzini
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
unset LINES
unset COLS
umask 022
unset LANGUAGE
unset LC_ALL
LANG=C
export LANG

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

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		-e 's/uuid = ".*"/uuid = "UUID"/' \
                -e 's/value = ".*"/value = "UUID"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

no_result()
{
	set +x
	echo "NO RESULT for test of aedist -send -entire-source functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of aedist -send -entire-source functionality ($activity)" 1>&2
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
	log_file_preference = never; \
	default_development_directory=\"$work\";"
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
activity="working directory 131"
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
activity="new project 150"
$bin/aegis -npr foo -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 157"
cat > $tmp << 'end'
description = "A bogus project created to test the aedist -send "
    "-entire-source functionality.";
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
activity="new change 172"
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
activity="new change 184"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 197"
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
activity="new file 214"
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

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
end
if test $? -ne 0 ; then no_result; fi

#
# create a new test
#
activity="new test 254"
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
activity="build 266"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 273"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 280"
$bin/aegis -t -v > log 2>&1
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
# start work on change 2
#
activity="develop begin 329"
$bin/aegis -db -c 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# copy a file into the change
#
activity="copy file 336"
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
# build a distribution set
#
activity="send the change 366"
$bin/aedist -send -ndh -c 2 -o test.ae \
    --entire-source > log 2>&1
if test $? -ne 0 ; then fail; fi

#
# form a listing, rather than compare the binary
#
$bin/aedist -list -f test.ae -o test.out -pw=79 -pl=66
if test $? -ne 0 ; then fail; fi

sed -e '/[0-9][0-9][0-9][0-9]$/d' < test.out > test.out2
if test $? -ne 0 ; then no_result; fi

#
# compare the listing with expected
#
cat > test.ok << 'end'



Distribution Change Set							 Page 1

PROJECT
	foo, change 2

SUMMARY
	A bogus project created to test the aedist -send -entire-source
	functionality.

DESCRIPTION
	A bogus project created to test the aedist -send -entire-source
	functionality.

CAUSE
	This change was caused by internal_enhancement.

FILES
	Type	Action	 File Name
	------- -------- -----------
	source	create	 Makefile
	config	create	 aegis.conf
	source	modify	 main.c
	source	create	 test.c
	test	create	 test/00/t0001a.sh
end
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out2
if test $? -ne 0 ; then fail; fi

#----------------------------------------------------------------------

#
#
#
activity="move test.c test2.c 422"
$bin/aegis -mv -c 2 $workchan/test.c $workchan/test2.c \
        -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy Makefile 427"
$bin/aegis -cp -c 2 $workchan/Makefile -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/Makefile <<EOF
.c.o:
	date > $@

foo: main.o test2.o
	date > $@
EOF
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aedist -send -entire_source -ndh -c 2 -o $work/c2.ae > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

mkdir -p $work/c2.d

$bin/test_cpio -extract -change_directory $work/c2.d -f $work/c2.ae
if test $? -ne 0 ; then cat log; no_result; fi

cat > test.ok <<EOF
$work/c2.d/etc/change-number
$work/c2.d/etc/change-set
$work/c2.d/etc/project-name
$work/c2.d/src/Makefile
$work/c2.d/src/aegis.conf
$work/c2.d/src/main.c
$work/c2.d/src/test/00/t0001a.sh
$work/c2.d/src/test2.c
EOF
if test $? -ne 0 ; then cat log; no_result; fi

find $work/c2.d -type f -print | LANG=C sort > test.out
if test $? -ne 0 ; then cat log; no_result; fi

diff $work/test.ok $work/test.out
if test $? -ne 0 ; then cat log; no_result; fi

cat > test.ok <<EOF
brief_description = "A bogus project created to test the aedist -send -entire-source functionality.";
description = "A bogus project created to test the aedist -send -entire-source functionality.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
attribute =
[
	{
		name = "original-UUID";
		value = "UUID";
	},
];
state = awaiting_development;
src =
[
	{
		file_name = "Makefile";
		uuid = "UUID";
		action = modify;
		usage = source;
	},
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = create;
		usage = config;
	},
	{
		file_name = "main.c";
		uuid = "UUID";
		action = modify;
		usage = source;
	},
	{
		file_name = "test.c";
		uuid = "UUID";
		action = remove;
		usage = source;
		move = "test2.c";
	},
	{
		file_name = "test/00/t0001a.sh";
		uuid = "UUID";
		action = create;
		usage = test;
	},
	{
		file_name = "test2.c";
		uuid = "UUID";
		action = create;
		usage = source;
		move = "test.c";
	},
];
EOF

check_it $work/test.ok $work/c2.d/etc/change-set

$bin/aegis -build -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

$bin/aegis -diff -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="line  532"
$bin/aegis -dev_end -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 539"
$bin/aegis -rpass -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 546"
$bin/aegis -ib 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 553"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate test
#
activity="test 560"
$bin/aegis -t -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 567"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi



#-----------------------------------------------------------


#
# create a third change
#
activity="new change 579"
cat > $tmp << 'end'
brief_description = "The third change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start work on change 3
#

activity="develop begin 594"
$bin/aegis -db -c 3 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# copy a file into the change
#
activity="copy file 601"
$bin/aegis -mv $workchan/test2.c $workchan/test3.c \
    -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify file 606"
$bin/aegis -cp $workchan/Makefile
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/Makefile <<EOF
.c.o:
	date > $@

foo: main.o test3.o
	date > $@
EOF
if test $? -ne 0 ; then cat log; no_result; fi

#
# build
#
activity="build 622"
$bin/aegis -b -c 3 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff the change 626"
$bin/aegis -diff -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end  630"
$bin/aegis -dev_end -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the review
#
activity="review pass 637"
$bin/aegis -rpass -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 644"
$bin/aegis -ib 3 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate build
#
activity="build 651"
$bin/aegis -b -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate test
#
activity="test 658"
$bin/aegis -t -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# pass the integration
#
activity="integrate pass 665"
$bin/aegis -intpass -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# build a distribution set
#
activity="send the archive 672"
$bin/aedist -send -ndh -bl -o c3.ae \
    --entire-source -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

mkdir -p $work/c3.d

activity="extract the archive 679"
$bin/test_cpio -extract -change_directory $work/c3.d -f $work/c3.ae
if test $? -ne 0 ; then cat log; no_result; fi

activity="check the content 683"
cat > $work/test.ok <<EOF
$work/c3.d/etc/change-number
$work/c3.d/etc/change-set
$work/c3.d/etc/project-name
$work/c3.d/src/Makefile
$work/c3.d/src/aegis.conf
$work/c3.d/src/main.c
$work/c3.d/src/test/00/t0001a.sh
$work/c3.d/src/test3.c
EOF
if test $? -ne 0 ; then no_result; fi

find $work/c3.d -type f -print | sort > test.out
if test $? -ne 0 ; then cat log; no_result; fi

diff $work/test.ok $work/test.out > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check etc/change-set 702"
cat > test.ok <<EOF
brief_description = "A bogus project created to test the aedist -send -entire-source functionality.";
description = "A bogus project created to test the aedist -send -entire-source functionality.";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = false;
regression_test_exempt = true;
attribute =
[
	{
		name = "original-UUID";
		value = "UUID";
	},
	{
		name = "original-UUID";
		value = "UUID";
	},
	{
		name = "original-UUID";
		value = "UUID";
	},
];
state = awaiting_development;
src =
[
	{
		file_name = "Makefile";
		uuid = "UUID";
		action = create;
		usage = source;
	},
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = create;
		usage = config;
	},
	{
		file_name = "main.c";
		uuid = "UUID";
		action = create;
		usage = source;
	},
	{
		file_name = "test/00/t0001a.sh";
		uuid = "UUID";
		action = create;
		usage = test;
	},
	{
		file_name = "test2.c";
		uuid = "UUID";
		action = remove;
		usage = source;
		move = "test3.c";
	},
	{
		file_name = "test3.c";
		uuid = "UUID";
		action = create;
		usage = source;
		move = "test2.c";
	},
];
EOF

check_it $work/test.ok $work/c3.d/etc/change-set


activity="receive the archive 772"
$bin/aedist -receive -f c3.ae -c 4 -ignore_uuid -trojan -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# the things tested in this test, worked
#
pass
