#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2002, 2004-2008 Peter Miller
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
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

no_result()
{
	set +x
	echo "NO RESULT for test of aedist functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of aedist functionality ($activity)" 1>&2
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

PATH=${bin}:${PATH}
export PATH

#
# make the directories
#
activity="working directory 114"
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
activity="new project 133"
$bin/aegis -npr foo -vers "" -dir $workproj -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 140"
cat > $tmp << 'end'
description = "A bogus project created to test the aedist functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption= true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 155"
$bin/aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=foo ; export AEGIS_PROJECT

# --------------------------------------------------------------------------

#
# create a new change
#
activity="new change 170"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
$bin/aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new file 188"
$bin/aegis -nf $workchan/main.c $workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.c << 'end'
int
main()
{
	test();
	exit(0);
	return 0;
}
end
if test $? -ne 0 ; then no_result; fi

activity="new file 202"
$bin/aegis -nf $workchan/to_be_modified.c -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/to_be_modified.c << 'end'
int
main()
{
	test();
	exit(0);
	return 0;
}
end
if test $? -ne 0 ; then no_result; fi

activity="new file 216"
$bin/aegis -nf $workchan/to_be_deleted.c -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/to_be_deleted.c << 'end'
int
main()
{
	test();
	exit(0);
	return 0;
}
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
test_command = "exit 0; # $filename";
link_integration_directory = true;
create_symlinks_before_build = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
history_put_trashes_file = warn;
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 253"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 260"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# develop end
#
activity="develop end 267"
$bin/aegis -de 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# review pass
#
activity="review pass 274"
$bin/aegis -rpass 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# integrate
#
activity="integrate begin 281"
$bin/aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate diff 284"
$bin/aegis -diff 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate build 287"
$bin/aegis -b 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="integrate pass 290"
$bin/aegis -ipass 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------

#
# create a new change
#
activity="new change 299"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f $tmp -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a second change
#
$bin/aegis -db 2 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# modify a file
#
activity="modify usage of main.c 317"
$bin/aegis -c 2 -cp $workchan/main.c -nl > log 2>&1
cp $workchan/main.c $workchan/main.c.ORIG 2>&1
$bin/aegis -c 2 -cpu $workchan/main.c -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis 2 -rm $workchan/main.c -nowhiteout -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -build 2 -nl >log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -c 2 -nt $workchan/main.c -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cp $workchan/main.c.ORIG $workchan/main.c 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/main.c << 'end'
int
main()
{
	return 0;
}
end
if test $? -ne 0 ; then no_result; fi

$bin/aegis -cp 2 $workchan/to_be_modified.c -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $tmp <<EOF
Thi file must be modified
EOF
if test $? -ne 0 ; then no_result; fi
cp $tmp $workchan/to_be_modified.c
if test $? -ne 0 ; then no_result; fi

$bin/aegis -rm 2 $workchan/to_be_deleted.c -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


#
# build the change
#
activity="build 355"
$bin/aegis -build -c 2 -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 362"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 369"
$bin/aegis -test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $tmp <<EOF
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
EOF
$bin/aegis -ca -f $tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# develop end
#
activity="develop end 383"
$bin/aegis -de 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# review pass
#
activity="review pass 390"
$bin/aegis -rpass 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Build a patch with this change in it.
#
activity="send patch 397"
$bin/aedist -send 2 -o $work/c2.ae -naa -nocomp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# Receive the patch.
#
activity="recevie patch 404"
$bin/aedist -rec -f $work/c2.ae -c 3 -dir $work/c3dir -trojan > log.aedist 2>&1
if test $? -ne 0 ; then cat log.aedist; fail; fi

#
# Make sure the patch is received accurately.
#
activity="check file list 411"
diff $workchan/main.c $work/c3dir/main.c
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -list cf -c 2 -unf > $work/c2.list
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -list cf -c 3 -unf > $work/c3.list
if test $? -ne 0 ; then cat log; no_result; fi
diff $work/c2.list $work/c3.list
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass
