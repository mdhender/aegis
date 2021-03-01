#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2005-2008 Peter Miller
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
	echo "FAILED test of the aerevml functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the aerevml functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

check_it()
{
	sed	-e "s|$work|...|g" \
		-e 's|= [0-9][0-9]*; /.*|= TIME;|' \
		-e "s/\"$USER\"/\"USER\"/g" \
		-e 's/uuid = ".*"/uuid = "UUID"/' \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

activity="create test directory 107"
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
# test the aerevml functionality
#
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="new project 129"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
	-lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT


activity="project attributes 138"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 151"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 159"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 169"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 173"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/fred \
	$work/test.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "exit 0";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/fred
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

$bin/aegis -file-attr -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd009 \
	$work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

activity="build 204"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 208"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="uuid set 212"
$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd000 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 216"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 220"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 224"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 228"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 232"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# second change
#
activity="new change 239"
cat > caf << 'fubar'
brief_description = "the second change";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 248"
$bin/aegis -db 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="copy file 252"
$bin/aegis -cp $work/test.C002/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

echo second > $work/test.C002/barney
if test $? -ne 0 ; then no_result; fi

$bin/aegis -ca -uuid aaaaaaaa-bbbb-4bbb-8ccc-ccccdddddead -c 2 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="build 262"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="diff 266"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 270"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 274"
$bin/aegis -ib 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 278"
$bin/aegis -b 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate diff 282"
$bin/aegis -diff -c 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 286"
$bin/aegis -ipass -c 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="aerevml -send 290"
$bin/aerevml -send -p $AEGIS_PROJECT -c 2 -o test.out2 -cte=none \
	-no-description-header -no-mime-header > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

sed \
	-e 's|^<time>.*</time>$|<time>XXX</time>|' \
	-e 's|^<rep_desc>.*</rep_desc>$|<rep_desc>XXX</rep_desc>|' \
	< test.out2 > test.out
if test $? -ne 0 ; then no_result; fi

activity="verify revml output 301"
cat > test.ok << 'fubar'
<?xml version="1.0"?>
<!DOCTYPE revml PUBLIC "-//-//DTD REVML 0.35//EN" "revml.dtd">
<revml version="0.35">
<time>XXX</time>
<rep_type>Aegis</rep_type>
<rep_desc>XXX</rep_desc>
<comment>the second change</comment>
<rev_root>test</rev_root>
<attribute><name>X-Aegis-change-number</name>
<value>2</value></attribute>
<attribute><name>X-Aegis-brief-description</name>
<value>the second change</value></attribute>
<attribute><name>X-Aegis-description</name>
<value>the second change</value></attribute>
<attribute><name>X-Aegis-cause</name>
<value>internal_enhancement</value></attribute>
<attribute><name>X-Aegis-test-exempt</name>
<value>true</value></attribute>
<attribute><name>X-Aegis-test-baseline-exempt</name>
<value>true</value></attribute>
<attribute><name>X-Aegis-regression-test-exempt</name>
<value>true</value></attribute>
<attribute><name>X-Aegis-uuid</name>
<value>aaaaaaaa-bbbb-4bbb-8ccc-ccccdddddead</value></attribute>
<rev id="aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd009">
<name>barney</name>
<source_name>barney</source_name>
<source_filebranch_id>no idea what this means</source_filebranch_id>
<source_repo_id>no idea what this means</source_repo_id>
<action>edit</action>
<type>text/plain; charset=us-ascii</type>
<rev_id>0</rev_id>
<source_rev_id>no idea what this means</source_rev_id>
<attribute><name>X-Aegis-usage</name>
<value>source</value></attribute>
<attribute><name>X-Aegis-action</name>
<value>modify</value></attribute>
<attribute><name>X-Aegis-executable</name>
<value>false</value></attribute>
<attribute><name>X-Aegis-uuid</name>
<value>aaaaaaaa-bbbb-4bbb-8ccc-ccccddddd009</value></attribute>
<delta type="diff-u" encoding="none">--- barney
+++ barney
@@ -1 +1 @@
-one
+second
</delta>
</rev>
</revml>
fubar
if test $? -ne 0 ; then cat log; no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
