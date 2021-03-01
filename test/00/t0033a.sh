#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1995-1999, 2006-2008 Peter Miller
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
	echo "FAILED test of the report relational operators ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT for test of the report relational operators ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create working directory 69"
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
# test the report relational operators
#
activity="files 102"
cat > test.in << 'fubar'
title("Test of Relational Operators", "Various Operand Type Combinations");
columns
(
	{ name = "LHS\n-------"; width = 8; },
	{ name = "RHS\n-------"; width = 8; },
	{ name = "<\n-------"; width = 8; },
	{ name = "<=\n-------"; width = 8; },
	{ name = ">\n-------"; width = 8; },
	{ name = ">=\n-------"; width = 8; },
	{ name = "==\n-------"; width = 8; },
	{ name = "!=\n-------"; width = 8; }
);
auto list, lhs, rhs;
list = ["", "0", 0, false, "1", 1, true, "bogus"];
for (lhs in list)
{
	for (rhs in list)
	{
		print
		(
			typeof(lhs) == "string" ? sprintf("\"%s\"", lhs) : lhs,
			typeof(rhs) == "string" ? sprintf("\"%s\"", rhs) : rhs,
			lhs < rhs, lhs <= rhs,
			lhs > rhs, lhs >= rhs,
			lhs == rhs, lhs != rhs
		);
	}
}
fubar
if test $? -ne 0 ; then no_result; fi
cat > test.ok << 'fubar'

Test of Relational Operators                                             Page 1
Various Operand Type Combinations                      XXX XXX XX XX:XX:XX XXXX

LHS      RHS      <        <=       >        >=       ==       !=
-------  -------  -------  -------  -------  -------  -------  -------
""       ""       false    true     false    true     true     false
""       "0"      false    true     false    true     true     false
""       0        false    true     false    true     true     false
""       false    false    true     false    true     true     false
""       "1"      true     true     false    false    false    true
""       1        true     true     false    false    false    true
""       true     true     true     false    false    false    true
""       "bogus"  true     true     false    false    false    true

Test of Relational Operators                                             Page 2
Various Operand Type Combinations                      XXX XXX XX XX:XX:XX XXXX

LHS      RHS      <        <=       >        >=       ==       !=
-------  -------  -------  -------  -------  -------  -------  -------
"0"      ""       false    true     false    true     true     false
"0"      "0"      false    true     false    true     true     false
"0"      0        false    true     false    true     true     false
"0"      false    false    true     false    true     true     false
"0"      "1"      true     true     false    false    false    true
"0"      1        true     true     false    false    false    true
"0"      true     true     true     false    false    false    true
"0"      "bogus"  true     true     false    false    false    true

Test of Relational Operators                                             Page 3
Various Operand Type Combinations                      XXX XXX XX XX:XX:XX XXXX

LHS      RHS      <        <=       >        >=       ==       !=
-------  -------  -------  -------  -------  -------  -------  -------
0        ""       false    true     false    true     true     false
0        "0"      false    true     false    true     true     false
0        0        false    true     false    true     true     false
0        false    false    true     false    true     true     false
0        "1"      true     true     false    false    false    true
0        1        true     true     false    false    false    true
0        true     true     true     false    false    false    true
0        "bogus"  true     true     false    false    false    true

Test of Relational Operators                                             Page 4
Various Operand Type Combinations                      XXX XXX XX XX:XX:XX XXXX

LHS      RHS      <        <=       >        >=       ==       !=
-------  -------  -------  -------  -------  -------  -------  -------
false    ""       false    true     false    true     true     false
false    "0"      false    true     false    true     true     false
false    0        false    true     false    true     true     false
false    false    false    true     false    true     true     false
false    "1"      true     true     false    false    false    true
false    1        true     true     false    false    false    true
false    true     true     true     false    false    false    true
false    "bogus"  false    false    true     true     false    true

Test of Relational Operators                                             Page 5
Various Operand Type Combinations                      XXX XXX XX XX:XX:XX XXXX

LHS      RHS      <        <=       >        >=       ==       !=
-------  -------  -------  -------  -------  -------  -------  -------
"1"      ""       false    false    true     true     false    true
"1"      "0"      false    false    true     true     false    true
"1"      0        false    false    true     true     false    true
"1"      false    false    false    true     true     false    true
"1"      "1"      false    true     false    true     true     false
"1"      1        false    true     false    true     true     false
"1"      true     false    true     false    true     true     false
"1"      "bogus"  true     true     false    false    false    true

Test of Relational Operators                                             Page 6
Various Operand Type Combinations                      XXX XXX XX XX:XX:XX XXXX

LHS      RHS      <        <=       >        >=       ==       !=
-------  -------  -------  -------  -------  -------  -------  -------
1        ""       false    false    true     true     false    true
1        "0"      false    false    true     true     false    true
1        0        false    false    true     true     false    true
1        false    false    false    true     true     false    true
1        "1"      false    true     false    true     true     false
1        1        false    true     false    true     true     false
1        true     false    true     false    true     true     false
1        "bogus"  true     true     false    false    false    true

Test of Relational Operators                                             Page 7
Various Operand Type Combinations                      XXX XXX XX XX:XX:XX XXXX

LHS      RHS      <        <=       >        >=       ==       !=
-------  -------  -------  -------  -------  -------  -------  -------
true     ""       false    false    true     true     false    true
true     "0"      false    false    true     true     false    true
true     0        false    false    true     true     false    true
true     false    false    false    true     true     false    true
true     "1"      false    true     false    true     true     false
true     1        false    true     false    true     true     false
true     true     false    true     false    true     true     false
true     "bogus"  false    false    true     true     false    true

Test of Relational Operators                                             Page 8
Various Operand Type Combinations                      XXX XXX XX XX:XX:XX XXXX

LHS      RHS      <        <=       >        >=       ==       !=
-------  -------  -------  -------  -------  -------  -------  -------
"bogus"  ""       false    false    true     true     false    true
"bogus"  "0"      false    false    true     true     false    true
"bogus"  0        false    false    true     true     false    true
"bogus"  false    true     true     false    false    false    true
"bogus"  "1"      false    false    true     true     false    true
"bogus"  1        false    false    true     true     false    true
"bogus"  true     true     true     false    false    false    true
"bogus"  "bogus"  false    true     false    true     true     false
fubar
if test $? -ne 0 ; then no_result; fi
activity="run 249"
$bin/aegis -report -f test.in -o test.out -pl 15 -tw 1 -pw=79
if test $? -ne 0 ; then fail; fi
activity="compare 252"
sed -e 's/[A-Z][a-z][a-z] [A-Z][a-z][a-z] [ 123][0-9] [0-2][0-9]:[0-5][0-9]:[0-5][0-9] [12][0-9][0-9][0-9]/XXX XXX XX XX:XX:XX XXXX/' \
	< test.out > test.out2
if test $? -ne 0 ; then no_result; fi
diff test.ok test.out2
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
