#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2002 Peter Miller;
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
# MANIFEST: Test the aeb -minimum functionality
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
worklib=$work/lib
workproj=$work/foo.proj
workchan=$work/foo.chan
tmp=$work/tmp
AEGIS_PATH=$worklib ; export AEGIS_PATH
AEGIS_PROJECT=foo ; export AEGIS_PROJECT
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
	echo "FAILED test of the aeb -minimum  functionality " 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
  echo "NO RESULT when testing the aeb -minimum  functionality ($activity)" 1>&2
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
# If the C compiler is called something other than ``cc'', as discovered
# by the configure script, create a shell script called ``cc'' which
# invokes the correct C compiler.  Make sure the current directory is in
# the path, so that it will be invoked.
#
if test "$CC" != "" -a "$CC" != "cc"
then
	cat >> $work/cc << fubar
#!/bin/sh
exec $CC \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx $work/cc
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# test the aeb -minimum functionality
#
#
# make a new project
#
activity="new project 129"
$bin/aegis -npr foo -vers "" -dir $workproj  > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 131"
cat > $tmp << 'end'
description = "A bogus project created to test the aeb -minimum functionality.";
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
activity="new change 143"
cat > $tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true ;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc 10 -f $tmp -p foo > log 2>&1
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

#
# begin development of a change
#
$bin/aegis -db 10 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/config << 'end'
build_command = "echo Hello > non-source-file && exit 0";
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
create_symlinks_before_build = true;
remove_symlinks_after_integration_build = false;
end
if test $? -ne 0 ; then no_result; fi

activity="New file 194"
$bin/aegis -nf $workchan/config > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Build 198"
$bin/aegis -b > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Diff 202"
$bin/aegis -diff > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


activity="Develop end 207"
$bin/aegis -de > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Review Pass 214"
$bin/aegis -rpass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Begin 218"
$bin/aegis -ib  10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Build 222"
$bin/aegis -b 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Pass 227"
$bin/aegis -ipass 10 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


activity="new branch 233"
$bin/aegis -nbr  1 -p foo > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


# The second change
activity="new change 239"
cat > $tmp << 'end'
brief_description = "The second change";
cause = internal_bug;
test_exempt = true;
test_baseline_exempt = true ;
regression_test_exempt = true;
end
if test $? -ne 0 ; then no_result; fi


activity="new change 243"
$bin/aegis -nc 11 -f $tmp -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Develop Begin 247"
$bin/aegis -db 11 -dir $workchan -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Change Dir 252"
devdir=`$bin/aegis -cd 11  -p foo.1`
if test $? -ne 0 ; then no_result; fi
cd $devdir
if test $? -ne 0 ; then no_result; fi

activity="Copy File 258"
$bin/aegis -cp config  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# Change the build command to not create any non source files
activity="Mutate config 262"
grep -v 'build_command' config > tmpConfig
if test $? -ne 0 ; then cat log; no_result; fi
echo 'build_command = "exit 0";' >> tmpConfig
if test $? -ne 0 ; then cat log; no_result; fi
mv tmpConfig config
if test $? -ne 0 ; then cat log; no_result; fi

activity="Build 263"
$bin/aegis -b 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Diff 269"
$bin/aegis -diff 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Develop End 273"
$bin/aegis -de 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Review Pass 277"
$bin/aegis -rpass 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Begin 276"
$bin/aegis -ib -minimum 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="Aesub  280"
intdir=`$bin/aesub '$intdir' -c 11 -p foo.1`
if test $? -ne 0 ; then cat log; no_result; fi

activity="Integrate Build 289"
$bin/aegis -b -minimum 11  -p foo.1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


# Non source files should not be present after a -minimum
test ! -h $intdir/non-source-file
if test $? -ne 0 ; then  fail ; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
