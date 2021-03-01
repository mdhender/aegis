#!/bin/sh
#
# aegis - The "aegis" program.
# Copyright (C) 2008, 2010 Walter Franzini
# Copyright (C) 2008 Peter Miller
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

TEST_SUBJECT="aeipass vs. aeipass-option:assign-file-uuid"

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
                -e 's|; charset=us-ascii||' \
		< $2 > $work/sed.out
	if test $? -ne 0; then no_result; fi
	diff $1 $work/sed.out
	if test $? -ne 0; then fail; fi
}

#
# test the aeipass functionality
#
workproj=$work/proj
workchan=$work/chan

AEGIS_PROJECT=example ; export AEGIS_PROJECT

#
# make a new project
#
activity="new project 54"
aegis -npr $AEGIS_PROJECT -vers "" -dir $workproj > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# change project attributes
#
activity="project attributes 61"
cat > tmp << 'end'
description = "A bogus project created to test the aeipass functionality.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
end
if test $? -ne 0 ; then no_result; fi
aegis -pa -f tmp > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add the staff
#
activity="staff 77"
aegis -nd $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# create a new change
#
activity="new change 88"
cat > tmp << 'end'
brief_description = "The first change";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
aegis -nc 1 -f tmp -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# begin development of a change
#
aegis -db 1 -dir $workchan > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# add a new files to the change
#
activity="new files 106"
aegis -nf  -no-uuid $workchan/bogus1 -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nf  $workchan/aegis.conf -nl > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $workchan/bogus1 << 'end'
OLD CONTENT
bogus1, line 1
bogus1, line 2
end
if test $? -ne 0 ; then no_result; fi

activity="check file-attributes 119"
cat > ok <<EOF
attribute =
[
	{
		name = "aeipass-option:assign-file-uuid";
		value = "false";
	},
	{
		name = "usage";
		value = "source";
	},
	{
		name = "content-type";
		value = "text/plain";
	},
];
EOF
if test $? -ne 0 ; then no_result; fi

aegis -file-attribute -list $workchan/bogus1 > bogus1.fileattr
if test $? -ne 0 ; then no_result; fi

check_it ok bogus1.fileattr
if test $? -ne 0 ; then fail; fi


cat > $workchan/aegis.conf << 'end'
build_command = "exit 0";
link_integration_directory = true;

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; $diff $orig $i > $out; test $$? -le 1";
merge_command = "(diff3 -e $i $orig $mr | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $i > $out";
patch_diff_command = "set +e; $diff -C0 -L $index -L $index $orig $i > $out; \
test $$? -le 1";
end
if test $? -ne 0 ; then no_result; fi

#
# build the change
#
activity="finish the change 168"
aefinish -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# start integrating
#
activity="integrate begin 175"
aegis -ib 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# finish integration
#
activity="integrate the change 182"
aefinish -c 1 > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > ok <<EOF
src =
[
	{
		file_name = "aegis.conf";
		uuid = "UUID";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = config;
	},
	{
		file_name = "bogus1";
		action = create;
		edit =
		{
			revision = "1";
			encoding = none;
			uuid = "UUID";
		};
		usage = source;
	},
];
EOF
if test $? -ne 0 ; then no_result; fi

check_it ok $workproj/info/change/0/001.fs

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
