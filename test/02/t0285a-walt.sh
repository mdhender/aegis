#!/bin/sh
#
# aegis - The "aegis" program.
# Copyright (C) 2010 Walter Franzini
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.
#

TEST_SUBJECT="aenf --no-uuid"

# load up standard prelude and test functions
. test_funcs

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

#
# make a new project
#
activity="new project 44"
aegis -newpro foo -version "" -dir $work/proj -lib $work/lib
if test $? -ne 0 ; then fail; fi

AEGIS_PROJECT=foo
export AEGIS_PROJECT

#
# change project attributes
#
activity="project attributes 54"
cat > atts << 'fubar'
description = "The \"foo\" program.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
developers_may_create_changes = true;
umask = 022;
default_test_exemption = true;
default_test_regression_exemption = true;
minimum_change_number = 10;
reuse_change_numbers = true;
minimum_branch_number = 1;
skip_unlucky = false;
compress_database = false;
develop_end_action = goto_awaiting_integration;
protect_development_directory = false;
fubar
test $? -eq 0 || no_result

aegis -proatt -f atts -proj foo -lib $work/lib
test $? -eq 0 || no_result

activity="new developer 77"
aegis -newdev $USER
test $? -eq 0 || no_result
aegis -new-reviewer $USER
test $? -eq 0 || no_result
aegis -new-integrator $USER
test $? -eq 0 || no_result


#
# create a new change
#
activity="new change 89"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 1 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 98"
aegis -dev-begin 1 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 105"
aegis -new-file -no-uuid $work/dd/aegis.conf
test $? -eq 0 || fail

cat > $work/dd/aegis.conf << 'fubar'
build_command = "exit 0";
diff_command =
    "set +e;"
    "diff -a ${quote $original} ${quote $input}"
    " > ${quote $output};"
    "test $? -le 1";
merge_command = "set +e; "
    "merge -p -L baseline -L Historical -L C$c "
    "${quote $mostrecent} ${quote $original} ${quote $input} "
    "> ${quote $output}; "
    "test $? -le 1";
history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;
fubar
test $? -eq 0 || no_result

activity="create barney 130"
aegis -new-file -no-uuid $work/dd/barney
test $? -eq 0 || fail

cat > $work/dd/barney <<EOF
Hi my name is Barney
EOF
test $? -eq 0 || no_result

activity="create test 139"
aegis -new-test -no-uuid
test $? -eq 0 || fail

activity="check fstate 143"
cat > ok <<EOF
src =
[
	{
		file_name = "aegis.conf";
		action = create;
		usage = config;
		attribute =
		[
			{
				name = "aeipass-option:assign-file-uuid";
				value = "false";
			},
		];
	},
	{
		file_name = "barney";
		action = create;
		usage = source;
		attribute =
		[
			{
				name = "aeipass-option:assign-file-uuid";
				value = "false";
			},
		];
	},
	{
		file_name = "test/00/t0001a.sh";
		action = create;
		usage = test;
		attribute =
		[
			{
				name = "aeipass-option:assign-file-uuid";
				value = "false";
			},
		];
	},
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/001.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
