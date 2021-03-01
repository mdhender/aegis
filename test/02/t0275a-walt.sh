#!/bin/sh
#
# aegis - The "aegis" program.
# Copyright (C) 2008 Walter Franzini
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

TEST_SUBJECT=".pfs filtering at aeipass"

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
# test the functionality
#
#
# make a new project
#
activity="new project 47"
aegis -newpro foo -version "" -dir $work/proj -lib $work/lib
if test $? -ne 0 ; then fail; fi

AEGIS_PROJECT=foo
export AEGIS_PROJECT

#
# change project attributes
#
activity="project attributes 57"
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

activity="new developer 80"
aegis -newdev $USER
test $? -eq 0 || no_result
aegis -new-reviewer $USER
test $? -eq 0 || no_result
aegis -new-integrator $USER
test $? -eq 0 || no_result

activity="create branch 88"
aegis -new-br 1 -p $AEGIS_PROJECT > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

AEGIS_PROJECT=foo.1

#
# create a new change
#
activity="new change 97"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result
aegis -new_change 1 -f cattr -project $AEGIS_PROJECT
test $? -eq 0 || no_result

activity="develop begin 106"
aegis -dev-begin 1 -dir $work/dd
test $? -eq 0 || no_result

#
# add a new files to the change
#
activity="new file 113"
aegis -new-file $work/dd/aegis.conf
test $? -eq 0 || no_result

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
cache_project_file_list_for_each_delta = true;
fubar
test $? -eq 0 || no_result

activity="new file 139"
aegis -new-file $work/dd/fred
test $? -eq 0 || no_result

cat > $work/dd/fred <<EOF
Hi, I'm Fred
EOF
test $? -eq 0 || no_result

activity="new file 148"
aegis -new-file $work/dd/barney
test $? -eq 0 || no_result

cat > $work/dd/fred <<EOF
Hi, I'm Barney
EOF
test $? -eq 0 || no_result

#
# finish development of the change
#
activity="finish dev 160"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

#
# integrate the change
#
activity="finis int 167"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi


#
# create a new branch
#
activity="new branch 175"
aegis -nbr 10 -p $AEGIS_PROJECT
test $? -eq 0 || no_result

AEGIS_PROJECT=foo.1.10

activity="new change 181"
cat > cattr << 'end'
brief_description = "First change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result

aegis -nc 1 -f cattr -p $AEGIS_PROJECT
test $? -eq 0 || no_result

aegis -db 1 -dir $work/dd1
test $? -eq 0 || no_result

activity="modify fred 194"
aegis -cp 1 $work/dd1/fred
test $? -eq 0 || no_result

cat > $work/dd1/fred <<EOF
Hi,
I am Fred Flintstone.
EOF
test $? -eq 0 || no_result

activity="develop end 204"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi


#
# Do not integrate the change, yet.
#
activity="2nd change 212"
cat > cattr << 'end'
brief_description = "Second change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result

aegis -nc 2 -f cattr -p $AEGIS_PROJECT
test $? -eq 0 || no_result

aegis -db 2 -dir $work/dd
test $? -eq 0 || no_result

activity="modify barney 225"
aegis -cp $work/dd/barney
test $? -eq 0 || no_result

cat > $work/dd/barney <<EOF
Hi,
I'm Barney!
EOF
test $? -eq 0 || no_result

activity="finish the change 235"
aefinish 2 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="integrate the 2nd change 239"
aefinish 2 > LOG 2>&1
if test $? -ne 0; then cat LOG; exit 2; fi

activity="check branch fstate 243"
cat > ok <<EOF
src =
[
	{
		file_name = "barney";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "2";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
	{
		file_name = "fred";
		uuid = "UUID";
		action = transparent;
		edit =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
		locked_by = 1;
		about_to_be_copied_by = 1;
	},
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/001.branch/0/010.fs

activity="check 2nd change pfstate 303"
cat > ok <<EOF
src =
[
	{
		file_name = "barney";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "2";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/001.branch/0/010.branch/0/002.pfs

activity="the 3rd change 343"
cat > cattr << 'end'
brief_description = "Third change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result

aegis -nc 3 -f cattr -p $AEGIS_PROJECT
test $? -eq 0 || no_result

aegis -db 3 -dir $work/dd
test $? -eq 0 || no_result

activity="make barney transparent 356"
aegis -mt $work/dd/barney
test $? -eq 0 || no_result

activity="finish the change 360"
aefinish 3 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="integrate the change 364"
aefinish 3 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="check branch fstate 368"
cat > ok <<EOF
src =
[
	{
		file_name = "barney";
		uuid = "UUID";
		action = transparent;
		edit =
		{
			revision = "2";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
	},
	{
		file_name = "fred";
		uuid = "UUID";
		action = transparent;
		edit =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
		locked_by = 1;
		about_to_be_copied_by = 1;
	},
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/001.branch/0/010.fs

activity="check 3rd change pfstate 416"
cat > ok <<EOF
src =
[
	{
		file_name = "barney";
		uuid = "UUID";
		action = transparent;
		edit =
		{
			revision = "2";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
	},
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/001.branch/0/010.branch/0/003.pfs

activity="the 4th change 444"
cat > cattr << 'end'
brief_description = "Fourth change.";
cause = internal_enhancement;
end
test $? -eq 0 || no_result

aegis -nc 4 -f cattr -p $AEGIS_PROJECT
test $? -eq 0 || no_result

aegis -db 4 -dir $work/dd
test $? -eq 0 || no_result

activity="modify barney again 457"
aegis -cp $work/dd/barney
test $? -eq 0 || no_result

cat > $work/dd/barney <<EOF
I'm NOT Barney. Really!
EOF
test $? -eq 0 || no_result

activity="dev_end the change 466"
aefinish 4 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="check branch fstate file 470"
cat > ok <<EOF
src =
[
	{
		file_name = "barney";
		uuid = "UUID";
		action = transparent;
		edit =
		{
			revision = "2";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
		locked_by = 4;
	},
	{
		file_name = "fred";
		uuid = "UUID";
		action = transparent;
		edit =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
		locked_by = 1;
		about_to_be_copied_by = 1;
	},
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/001.branch/0/010.fs

activity="integrate the 1st change 519"
aefinish 1 > LOG 2>&1
if test $? -ne 0; then cat LOG; no_result; fi

activity="check branch fstate 523"
cat > ok <<EOF
src =
[
	{
		file_name = "barney";
		uuid = "UUID";
		action = transparent;
		edit =
		{
			revision = "2";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
		locked_by = 4;
	},
	{
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "2";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/001.branch/0/010.fs

activity="check 1st change pfstate 582"
cat > ok <<EOF
src =
[
	{
		file_name = "barney";
		uuid = "UUID";
		action = transparent;
		edit =
		{
			revision = "2";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
	},
	{
		file_name = "fred";
		uuid = "UUID";
		action = modify;
		edit =
		{
			revision = "2";
			encoding = none;
			uuid = "UUID";
		};
		edit_origin =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
		file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
		diff_file_fp =
		{
			youngest = TIME;
			oldest = TIME;
			crypto = "GUNK";
		};
	},
];
EOF
test $? -eq 0 || no_result

check_it ok $work/proj/info/change/0/001.branch/0/010.branch/0/001.pfs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
