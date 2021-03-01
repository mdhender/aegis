#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2007, 2008 Peter Miller
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

TEST_SUBJECT="aebisect functionality"

# load up standard prelude and test functions
. test_funcs

#
# some variables to make things earier to read
#
tmp=$work/tmp ;export tmp

worklib=$work/lib
workchan=$work/change-dir


AEGIS_PROJECT=example
export AEGIS_PROJECT

#
# test the aebisect functionality
#
activity="new project 133"
aegis -npr example -version - -lib $worklib -dir $work/proj -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 140"
cat > $tmp << 'TheEnd'
description = "bogosity";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -proatt -f $tmp -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="new developer 157"
aegis -newdev $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="new reviewer 160"
aegis -newrev $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
activity="new integrator 163"
aegis -newint $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# --------------------------------------------------------------------------
#
# Create a new change to get the project going.
#
activity="new change 171"
cat > $tmp << 'TheEnd'
brief_description = "c1";
description = "c1";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

AEGIS_CHANGE=10
export AEGIS_CHANGE

aegis -nc $AEGIS_CHANGE -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi


#
# begin development of the change
#
activity="develop begin 189"
aegis -devbeg $AEGIS_CHANGE -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a suitable aegis.conf file
#
activity="new files 196"
aegis -nf $workchan/aegis.conf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/aegis.conf << 'fubar'
build_command = "date >der.1 ; rm -f der.2; \
  if test -f fred ; then cat fred >der.2; fi; exit 0";
link_integration_directory = true;
development_directory_style = { source_file_link = true; };

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
merge_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
echo '1,$$p' ) | ed - $mr > $out";
history_put_trashes_file = warn;
fubar
if test $? -ne 0 ; then no_result; fi

activity="develop finish 220"
aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 224"
aegis -ib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate finish 228"
aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_CHANGE=11
activity="new change 233"
cat > $tmp << TheEnd
brief_description = "c${CHANGE}";
description = "c${CHANGE}";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -nc $AEGIS_CHANGE -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 244"
aegis -db $AEGIS_CHANGE -dir ${workchan} -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 248"
aegis -nf $workchan/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo fred > $workchan/fred
if test $? -ne 0 ; then no_result; fi

activity="develop finish 255"
aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 259"
aegis -ib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate finish 263"
aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch 267"
aegis -nbr -p $AEGIS_PROJECT -v 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=example.1

for CHANGE in 11 12 13
do

AEGIS_CHANGE=$CHANGE
activity="new change 277"
cat > $tmp << TheEnd
brief_description = "c${CHANGE}";
description = "c${CHANGE}";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -nc $AEGIS_CHANGE -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 288"
aegis -db $AEGIS_CHANGE -dir ${workchan} -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="modify file 292"
aegis -cp $workchan/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo "add by change $CHANGE" >>$workchan/fred
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop finish 299"
aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 303"
aegis -ib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate finish 307"
aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

done

AEGIS_PROJECT=example
activity="end branch 314"
aegis -de -c 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 318"
aegis -ib -v -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate finish 322"
aefinish -v -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new branch 326"
aegis -nbr -p $AEGIS_PROJECT -v 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=example.2

for CHANGE in 14 15
do

AEGIS_CHANGE=$CHANGE
activity="new change 336"
cat > $tmp << TheEnd
brief_description = "c${CHANGE}";
description = "c${CHANGE}";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -nc $AEGIS_CHANGE -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 347"
aegis -db $AEGIS_CHANGE -dir ${workchan} -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="copy files 351"
aegis -cp $workchan/fred -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="append text 355"
echo "add by change $CHANGE" >>$workchan/fred
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop finish 359"
aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 363"
aegis -ib -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate finish 367"
aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

done

AEGIS_CHANGE=16

activity="new change 375"
cat > $tmp << TheEnd
brief_description = "bisection";
description = "bisection sandbox";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

aegis -nc $AEGIS_CHANGE -f $tmp -project $AEGIS_PROJECT -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

# second branch defaults
activity="aebisect 387"
aebisect -c $AEGIS_CHANGE -br - -del 2 \
 -del 1 -dir $workchan -v -- grep  '"change 14"' der.2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check aebisect result 392"
sed 1d log >test.out
if test $? -ne 0 ; then cat log; no_result; fi

cat > test.in <<TheEnd
aebisect: -.D2 yields 1
aebisect: 2.D1 yields 0
aebisect: 1.D2 yields 1
aebisect: 1.D3 yields 1
Final bracketing deltas: 1.D3 and 2.D1
TheEnd
if test $? -ne 0 ; then cat log; no_result; fi

diff test.in test.out
if test $? -ne 0 ; then fail; fi

# test the -z flag while we can
activity="aebisect 409"
aebisect -c $AEGIS_CHANGE -br - -del 1 \
-br 2  -del 1 -dir $workchan -v -z -- grep '"change 14"' der.2 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="check aebisect result 414"
sed 1d log >test.out
if test $? -ne 0 ; then cat log; no_result; fi

cat > test.in <<TheEnd
aebisect: -.D1 yields 2
aebisect: 2.D1 yields 0
aebisect: 1.D1 yields 1
aebisect: 1.D2 yields 1
aebisect: 1.D3 yields 1
Final bracketing deltas: 1.D3 and 2.D1
TheEnd
if test $? -ne 0 ; then cat log; no_result; fi

diff test.in test.out
if test $? -ne 0 ; then fail; fi

pass
