#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2006-2008 Peter Miller
#	Copyright (C) 2005 Jerry Pendergraft
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

TEST_SUBJECT="multi-architecture testing functionality"

# load up standard prelude and test functions
. test_funcs

# oft used operations
chg_diff()
{
   aegis -diff -p example -c $1 -lib $worklib -v > log 2>&1
   if test $? -ne 0 ; then cat log; no_result; fi
}

chg_build()
{
    aegis -build -p example -c $1 -lib $worklib -v > log 2>&1
    if test $? -ne 0 ; then cat log; no_result; fi
}

chk_required()
{
  chg=$1
  label=$2
  # List details unformatted
  activity="${label}: list details 118"
  aegis -list cd -p example -c $chg -lib $worklib -unf > $tmp.cd 2> log
  if test $? -ne 0 ; then cat log; no_result; fi

  # Now then grab only the build/test details
  activity="${label}: get architecture results 123"
  grep '^m[1-9]' $tmp.cd > $tmp.cdt 2> log
  if test $? -ne 0 ; then cat log; no_result; fi

  # If any of the m? architecture fields are not date/time then it failes
  activity="${label}: check is recorded 128"
  # This check method requires aeca set non check test exempt
  grep 'required' $tmp.cd

  if test $? -eq 0; then fail; fi
}

test_results()
{
  exit_status=$1
  shift
  (
    echo 'test_result = ['
    while [ $# -gt 0 ]
    do
      tf=$1
      shift
      cat << EOF
{
	file_name = "$tf";
	exit_status = $exit_status;
	architecture = "m1-O1-V.1";
},
{
	file_name = "$tf";
	exit_status = $exit_status;
	architecture = "m2-O2-V.2";
},
{
	file_name = "$tf";
	exit_status = $exit_status;
	architecture = "m3-O3-V.3";
},
EOF
    done
    echo '];'
  ) > $work/res.dat
  if test $? -ne 0
  then
    echo "failed to make res.dat"
    no_result
  fi
}

#
# some variables to make things earier to read
#
worklib=$work/lib
workproj=$work/example.proj
workchan=$work/example.chan
tmp=$work/tmp ;export tmp

#
# make a new project
#
activity="new project 213"
aegis -newpro example -version "" -dir $workproj -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# change project attributes
#
activity="project attributes 220"
cat > $tmp << 'TheEnd'
description = "aegis multi arch testing";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
develop_end_action = goto_awaiting_integration;
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -proatt -f $tmp -proj example -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# add a new developer
#
activity="new developer 236"
aegis -newdev $USER -proj example -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="new integrator 240"
aegis -newint $USER -proj example -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# create a new change
#
activity="new change 247"
cat > $tmp << 'TheEnd'
brief_description = "Parallel testing";
description = "Batch test multiple architectures in one run.";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -new_change 1 -f $tmp -project example -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# begin development of the change
#
activity="develop begin 261"
aegis -devbeg -p example -c 1 -dir $workchan -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the new files to the change
#
activity="new file 268"
aegis -new_file $workchan/aegis.conf -nl -p example -c 1 -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# Add the config file content. Make up some very unlikely arch names as optional
cat > $workchan/aegis.conf << 'fubar'
build_command = "exit 0";
history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
merge_command = "exit 0";
build_covers_all_architectures = true;
batch_test_command = "${sh} ${Source Mtest.sh} ${Output} ${File_names}";
architecture =
[
    {
        name = "any-platform";
        pattern = "*";
        mode = optional;
    },
    {
        name = "m1-O1-V.1";
        pattern = "m1-*-O1*";
        mode = optional;
    },
    {
        name = "m2-O2-V.2";
        pattern = "m2-*-O2*";
        mode = optional;
    },
    {
        name = "m3-O3-V.3";
        pattern = "m3-*-O3";
        mode = optional;
    },
];
fubar
if test $? -ne 0 ; then no_result; fi

activity="new file 312"
aegis -new_file $workchan/Mtest.sh -nl -p example -c 1 -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="run test script 316"
cat > $workchan/Mtest.sh << fubar
#!/bin/sh
out=\$1
shift
echo "This is Mtest.sh"
echo "running tests: \$*"
echo "curdir: \`pwd\`"
cp $work/res.dat \$out
exit \$?
fubar
if test $? -ne 0 ; then no_result; fi

# Now add some arch names to change attributes - only the unlikely ones
activity="attributes file 330"
cat > $tmp.ca <<'fubar'
brief_description = "Parallel testing";
description = "Batch test multiple architectures in one run.";
cause = internal_enhancement;
test_exempt = false;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
        "m1-O1-V.1",
        "m2-O2-V.2",
        "m3-O3-V.3",
];
fubar
if test $? -ne 0 ; then cat log; no_result; fi

activity="change attributes 347"
aegis -chanatt -f $tmp.ca -proj example -c 1 -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# create a new test
# must specify the name so nt does not add user name?
# test/00/t0001a.sh
#
activity="new test 356"
aegis -new_test -p example -c 1 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $workchan/test/00/t0001a.sh << 'end'
#!/bin/sh
curdir=`pwd`
test $? -eq 0 || exit 2
echo "curdir: $curdir"
curdir=`dirname $curdir`
test $? -eq 0 || exit 2
echo "curdir: $curdir"
test "$curdir" != "baseline" || exit 1
exit 0
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="build 375"
chg_build 1

#
# test the change
#
activity="test 381"
test_results 0 test/00/t0001a.sh
aegis -test -p example -c 1 -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# Now see if the change attributes show all architectures done.
chk_required 1 "$activity"

# Now end and integrate this change
activity="diff 390"
chg_diff 1

activity="develop end 393"
aegis -devend -p example -c 1 -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 397"
aegis -ibegin -p example -c 1 -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integration diff 401"
chg_diff 1

activity="integration build 404"
chg_build 1

activity="integration test 407"
aegis -test -p example -c 1 -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integration pass 411"
aegis -ipass -p example -c 1 -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create another new change
#
activity="new change 418"
cat > $tmp << 'TheEnd'
brief_description = "Parallel testing - again";
description = "Batch test -bl and -reg multiple architectures in one run.";
cause = internal_enhancement;
architecture =
[
        "m1-O1-V.1",
        "m2-O2-V.2",
        "m3-O3-V.3",
];
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -new_change 2 -f $tmp -project example -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# begin development of the change
#
workchan=$work/example.chan.2
activity="develop begin 438"
aegis -devbeg -p example -c 2 -dir $workchan -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new test to the change
# these must also work for baseline tests
#
activity="new test 446"
aegis -new_test -p example -c 2 -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/test/00/t0002a.sh << 'end'
#!/bin/sh
# Check if we are in baseline
# If so - should fail
echo "running: $0"
curdir=`pwd`
test $? -eq 0 || exit 2
echo "curdir: $curdir"
curdir=`dirname $curdir`
test $? -eq 0 || exit 2
echo "curdir: $curdir"
test "$curdir" != "baseline" || exit 1
exit 0
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
chg_build 2

#
# make exempt test requirements for -bl and -reg
#
aegis -ca -list -p example -c 2 -lib $worklib 2> log |
 sed -e 's/test_baseline_exempt = .*/test_baseline_exempt = true;/' \
     -e 's/regression_test_exempt = .*/regression_test_exempt = true;/' \
 > $tmp.ca
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ca -p example -c 2 -lib $worklib -f $tmp.ca -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the change
#
activity="test 485"
test_results 0 test/00/t0002a.sh
aegis -test -p example -c 2 -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# Now see if the change attributes show all architectures done for test.
activity="attributes check 491"
chk_required 2 "$activity"

#
# make exempt test requirements for -reg , and
# make test required for -bl
#
activity="change attributes 498"
aegis -ca -list -p example -c 2 -lib $worklib 2> log |
  sed -e 's/test_baseline_exempt = .*/test_baseline_exempt = false;/' \
      -e 's/regression_test_exempt = .*/regression_test_exempt = true;/' \
  > $tmp.ca
if test $? -ne 0 ; then cat log; no_result; fi

aegis -ca -p example -c 2 -lib $worklib -f $tmp.ca -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# activity="test -bl 493"
test_results 1 test/00/t0002a.sh
aegis -test -bl -p example -c 2 -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# Now see if the change attributes show all architectures done for test -bl.
chk_required 2 "$activity"

#
# make exempt test requirements for -bl only
#
test_results 0 test/00/t0002a.sh
aegis -ca -list -p example -c 2 -lib $worklib 2> log |
 sed -e 's/test_baseline_exempt = .*/test_baseline_exempt = true;/' \
     -e 's/regression_test_exempt = .*/regression_test_exempt = false;/' \
 > $tmp.ca
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ca -p example -c 2 -lib $worklib -f $tmp.ca -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="test -reg 528"
test_results 0 test/00/t0001a.sh
aegis -test -reg -p example -c 2 -lib $worklib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# Now see if the change attributes show all architectures done for test -reg.
chk_required 2 "$activity"

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
