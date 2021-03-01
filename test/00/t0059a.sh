#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1997, 1998, 2004-2008 Peter Miller
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

no_result()
{
	set +x
	echo 'NO RESULT for test of the ${comment} functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo 'FAILED test of the ${comment} functionality' 1>&2
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
# If the C++ compiler is called something other than "c++", as
# discovered by the configure script, create a shell script called
# "c++" which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
#
if test "$CXX" != "c++"
then
	cat > $work/c++ << fubar
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
workproj=$work/proj
workchan=$work/chan
worklib=$work/lib
activity="new project 121"
$bin/aegis -newpro foo -dir $workproj -lib $worklib -vers ''
if test $? -ne 0 ; then no_result; fi

#
# change project attributes
#
activity="project attributes 128"
cat > tmp << 'end'
description = "A bogus project.";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -proatt -f tmp -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# create a new change
#
activity="new change 142"
cat > tmp << 'end'
brief_description = "change one";
cause = internal_bug;
end
$bin/aegis -new_change 1 -f tmp -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# add staff
#
activity="staff 153"
$bin/aegis -newdev $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# begin development of a change
#
activity="develop begin 160"
$bin/aegis -devbeg 1 -p foo -dir $workchan -lib $worklib
if test $? -ne 0 ; then no_result; fi
activity="new file 163"
$bin/aegis -new_file $workchan/aegis.conf $workchan/template -nl -lib $worklib -Pro foo
if test $? -ne 0 ; then no_result; fi

cat > $workchan/aegis.conf << 'end'
build_command =
    "rm -f foo; c++ -o foo -D'VERSION=\"$v\"' -D'PATH=\"$sp\"' main.cc";

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
		body = "${readfile ${source template abs}}";
	},
];
end
if test $? -ne 0 ; then no_result; fi

cat > $workchan/template << 'fubar'
$#
$# this comment should be throw away
$#
this is line one
${comment
	throw lots of stuff
	away from this comment
}
Insert a dollar sign ${$} here.
fubar
if test $? -ne 0 ; then no_result; fi

#
# create another new file,
# it should use the template
#
activity="new file with template 215"
$bin/aegis -nf $workchan/fred -lib $worklib -p foo
if test $? -ne 0 ; then fail; fi

#
# make sure the new file looks as expected
#
cat > test.ok << 'fubar'
this is line one

Insert a dollar sign $ here.
fubar
if test $? -ne 0 ; then no_result; fi

diff test.ok $workchan/fred
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
