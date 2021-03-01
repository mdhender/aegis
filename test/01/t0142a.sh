#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2004 John Darrington
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
	echo "FAILED test of the review policy functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the review policy functionality ($activity)" 1>&2
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
		-e 's/delta[0-9][0-9]*/delta/' \
		-e 's/19[0-9][0-9]/YYYY/' \
		-e 's/20[0-9][0-9]/YYYY/' \
		-e 's/node = ".*"/node = "NODE"/' \
		-e 's/crypto = ".*"/crypto = "GUNK"/' \
		< $1 > $work/sed.out
	if test $? -ne 0; then fail; fi
	diff $2 $work/sed.out
	if test $? -ne 0; then fail; fi
}


activity="create test directory 109"

mkdir -p $work $work/lib
if test $? -ne 0 ; then no_result; fi

chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi



cd $work
if test $? -ne 0 ; then no_result; fi

workproj=$work/proj
workchan=$work/chan
worklib=$work/lib

tmp=$work/temp

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
# make a new project
#
activity="new project 159"
$bin/aegis -newpro foo -version "" -dir $workproj -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# change project attributes
#
activity="project attributes 166"

cat > $tmp << 'end'
description = "A bogus project created to test things.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
minimum_change_number = 1;
default_test_exemption = true;
end

if test $? -ne 0 ; then no_result; fi
$bin/aegis -proatt -f $tmp -proj foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# create a new change
#
activity="new change 184"
cat > $tmp << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -newcha -list -pro foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi
$bin/aegis -new_change -f $tmp -project foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add a new developer
#
activity="new developer 199"
$bin/aegis -newdev $USER -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# begin development of a change
#
activity="develop begin 206"
$bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# add a new file to the change
#
activity="new file 213"
$bin/aegis -new_file $workchan/main.cc -nl -lib $worklib -Pro foo \
	-uuid aabbcccc-cccc-4dde-8eee-eeefff000001
if test $? -ne 0 ; then fail; fi



cat > $workchan/main.cc << 'end'
int
main(int argc, char **argv)
{
	return 0;
}
end
if test $? -ne 0 ; then no_result; fi



cat > $workchan/aegis.conf << 'end'
build_command = "rm -f foo; c++ -o foo -D'VERSION=\"$v\"' main.cc";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";

diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";

/*
 * file templates
 */
file_template =
[
	{
		pattern = [ "*" ];
		body = "hello\n";
	},
	{
		pattern = [ "test/*/*.sh" ];
		body = "#!/bin/sh\nexit 1\n";
	}
];
end
if test $? -ne 0 ; then no_result; fi

#
# A simple policy which requires the change to have been
# reviewed at least thrice
# As a side effect, this script writes the list of reviwers,developers
# to $work/{revs,devs}
#
cat > $workchan/revpol.sh << EOF
devs=\$1;
revs=\$2;

for d in \$devs; do echo \$d; done | sort > $work/devs
for r in \$revs; do echo \$r; done | sort > $work/revs
n=0
for r in \$revs ; do
    n=\`expr \$n + 1\`
done

if test \$n -ge 3 ; then
    exit 0;
fi

exit 1;
EOF
if test $? -ne 0 ; then no_result; fi

chmod +x $workchan/revpol.sh
if test $? -ne 0 ; then no_result; fi

cat >> $workchan/aegis.conf <<EOF
review_policy_command = "$workchan/revpol.sh \${quote \$Change_Developer_List}  \${quote \$Change_Reviewer_List }";
EOF
if test $? -ne 0 ; then no_result; fi


activity="new file 300"
$bin/aegis -new_file $workchan/aegis.conf -nl -lib $worklib -Pro foo \
	-uuid aabbcccc-cccc-4dde-8dee-eeefff000001
if test $? -ne 0 ; then fail; fi



#
# build the change
#
activity="build 310"
$bin/aegis -build -c 1 -nl -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out;fail; fi

#
# difference the change
#
activity="diff 317"
$bin/aegis -diff -lib $worklib -p foo > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail; fi


$bin/aegis -ca -uuid aabbcccc-cccc-4dde-8eee-eeefff000003 \
	-c 1 -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# finish development of the change
#
activity="develop end 329"
$bin/aegis -dev_end -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# add a new reviewer
#
activity="new reviewer 336"
$bin/aegis -newrev $USER -p foo -lib $worklib
if test $? -ne 0 ; then fail; fi
$bin/aegis -newrev -list -pr foo -lib $worklib > test.out
if test $? -ne 0 ; then fail; fi

#
# pass the review
#
activity="review pass 345"
$bin/aegis -review_pass -chan 1 -proj foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# check the the file states are as they should be
#
activity="review pass check 352"
cat > ok << 'fubar'
brief_description = "This change is used to test the aegis functionality with respect to change descriptions.";
description = "This change is used to test the aegis functionality with respect to change descriptions.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
copyright_years =
[
	YYYY,
];
state = being_reviewed;
given_test_exemption = true;
given_regression_test_exemption = true;
build_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
	},
];
development_directory = ".../chan";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "USER";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "USER";
	},
	{
		when = TIME;
		what = develop_end;
		who = "USER";
	},
	{
		when = TIME;
		what = review_pass_2br;
		who = "USER";
	},
];
uuid = "aabbcccc-cccc-4dde-8eee-eeefff000003";
fubar
if test $? -ne 0 ; then no_result; fi

check_it $workproj/info/change/0/001 ok

#
# Fake the meta-data so that the 2nd review doesn't
# get caught by the "duplicate review" check.
#
activity="Create fake metadata 414"
cat >  $workproj/info/change/0/001 <<EOF
brief_description = "This change is used to test the aegis functionality with respect to change descriptions.";
description = "This change is used to test the aegis functionality with respect to change descriptions.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
copyright_years =
[
	2004,
];
state = being_reviewed;
given_test_exemption = true;
given_regression_test_exemption = true;
build_time = 1092270147; /* Thu Aug 12 08:22:27 2004 */
architecture_times =
[
	{
		variant = "unspecified";
		node = "cornea";
		build_time = 1092270147; /* Thu Aug 12 08:22:27 2004 */
	},
];
development_directory = "$workchan";
history =
[
	{
		when = 1092270143; /* Thu Aug 12 08:22:23 2004 */
		what = new_change;
		who = "alf,,,,,";
	},
	{
		when = 1092270145; /* Thu Aug 12 08:22:25 2004 */
		what = develop_begin;
		who = "bert,,,,";
	},
	{
		when = 1092270152; /* Thu Aug 12 08:22:32 2004 */
		what = develop_end;
		who = "bert,,,,";
	},
	{
		when = 1092270153; /* Thu Aug 12 08:22:33 2004 */
		what = review_pass_2br;
		who = "charlie,";
	},
];
uuid = "aabbcccc-cccc-4dde-8eee-eeefff000003";
EOF
if test $? -ne 0 ; then no_result; fi

#
# pass the review a second time
#
activity="review pass 473"
$bin/aegis -review_pass -chan 1 -proj foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# Check that the 2nd review is logged
#
activity="review pass check 480"
cat > ok << 'fubar'
brief_description = "This change is used to test the aegis functionality with respect to change descriptions.";
description = "This change is used to test the aegis functionality with respect to change descriptions.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
copyright_years =
[
	YYYY,
];
state = being_reviewed;
given_test_exemption = true;
given_regression_test_exemption = true;
build_time = TIME;
architecture_times =
[
	{
		variant = "unspecified";
		node = "NODE";
		build_time = TIME;
	},
];
development_directory = ".../chan";
history =
[
	{
		when = TIME;
		what = new_change;
		who = "alf,,,,,";
	},
	{
		when = TIME;
		what = develop_begin;
		who = "bert,,,,";
	},
	{
		when = TIME;
		what = develop_end;
		who = "bert,,,,";
	},
	{
		when = TIME;
		what = review_pass_2br;
		who = "charlie,";
	},
	{
		when = TIME;
		what = review_pass_2br;
		who = "USER";
	},
];
uuid = "aabbcccc-cccc-4dde-8eee-eeefff000003";
fubar
if test $? -ne 0 ; then no_result; fi

check_it $workproj/info/change/0/001 ok

#
# Now create a dummy change result
#
activity="Create fake metadata 546"
cat >  $workproj/info/change/0/001 <<EOF
brief_description = "This change is used to test the aegis functionality with respect to change descriptions.";
description = "This change is used to test the aegis functionality with respect to change descriptions.";
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
architecture =
[
	"unspecified",
];
copyright_years =
[
	2004,
];
state = being_reviewed;
given_test_exemption = true;
given_regression_test_exemption = true;
build_time = 1092270147; /* Thu Aug 12 08:22:27 2004 */
architecture_times =
[
	{
		variant = "unspecified";
		node = "cornea";
		build_time = 1092270147; /* Thu Aug 12 08:22:27 2004 */
	},
];
development_directory = "$workchan";
history =
[
	{
		when = 1092270143; /* Thu Aug 12 08:22:23 2004 */
		what = new_change;
		who = "alf,,,,,";
	},
	{
		when = 1092270145; /* Thu Aug 12 08:22:25 2004 */
		what = develop_begin;
		who = "bert,,,,";
	},
	{
		when = 1092270152; /* Thu Aug 12 08:22:32 2004 */
		what = develop_end;
		who = "bert,,,,";
	},
	{
		when = 1092270152; /* Thu Aug 12 08:22:32 2004 */
		what = review_pass;
		who = "charlie,";
	},
	{
		when = 1092270152; /* Thu Aug 12 08:22:32 2004 */
		what = review_fail;
		who = "xerxes";
	},
	{
		when = 1092270152; /* Thu Aug 12 08:22:32 2004 */
		what = develop_end;
		who = "charlie";
	},
	{
		when = 1092270152; /* Thu Aug 12 08:22:32 2004 */
		what = review_pass;
		who = "david";
	},
	{
		when = 1092270152; /* Thu Aug 12 08:22:32 2004 */
		what = review_pass;
		who = "ernie";
	},
	{
		when = 1092270152; /* Thu Aug 12 08:22:32 2004 */
		what = review_pass_undo;
		who = "david";
	},
	{
		when = 1092270152; /* Thu Aug 12 08:22:32 2004 */
		what = review_pass;
		who = "fred";
	},
	{
		when = 1092270152; /* Thu Aug 12 08:22:32 2004 */
		what = review_pass;
		who = "david";
	},
	{
		when = 1092270152; /* Thu Aug 12 08:22:32 2004 */
		what = review_pass_undo;
		who = "fred";
	},
];
uuid = "aabbcccc-cccc-4dde-8eee-eeefff000003";
EOF
if test $? -ne 0 ; then no_result; fi

activity="review pass 642"
$bin/aegis -review_pass -chan 1 -proj foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# Check that the change has now passed to awaiting_integration
#
activity="review pass 649"
grep '^state.*awaiting_integration' $workproj/info/change/0/001 > /dev/null
if test $? -ne 0 ; then fail; fi

activity="test reviewer list 653"
sort <<EOF > rev.ok
ernie
david
$USER
EOF
if test $? -ne 0 ; then no_result; fi

diff rev.ok $work/revs
if test $? -ne 0 ; then fail; fi

#
# the things tested in this test, worked
#
pass
