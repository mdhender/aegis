#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994-1998, 2001, 2005-2008 Peter Miller
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

LINES=66
export LINES
COLS=80
export COLS

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

if test "$1" != "" ; then AEGIS_DATADIR=$here/$1/lib; else AEGIS_DATADIR="$here/lib"; fi
export AEGIS_DATADIR

no_result()
{
	set +x
	echo 'NO RESULT for test of the report generator functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo 'FAILED test of the report generator functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	echo PASSED
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
}
trap \"no_result\" 1 2 3 15

mkdir $work $work/lib $work/lib/report
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

AEGIS_PATH=$work/lib
export AEGIS_PATH

#
# test do and while loops
#
cat > test.in << 'fubar'
title("(Do)While Statement", "delete this line");
columns("Results");
auto j;
j = 0;
while (j < 10)
	print(++j);
do
	print(--j);
while
	(j > 0);
print("=====");
while (j < 10)
	print(j++);
do
	print(j--);
while
	(j > 0);
fubar
if test $? -ne 0 ; then no_result; fi
cat > test.ok << 'fubar'
1
2
3
4
5
6
7
8
9
10
9
8
7
6
5
4
3
2
1
0
=====
0
1
2
3
4
5
6
7
8
9
10
9
8
7
6
5
4
3
2
1
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -rpt -f test.in -unf -o test.out
if test $? -ne 0 ; then fail; fi
diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test bit and logical operators
#
cat > test.in << 'fubar'
title("Bit and Logical operators", "delete this line");
columns("a", "b", "~a", "a&b", "a^b", "a|b", "!a", "a&&b", "a||b");
auto a, b;
for (a = -3; a <= 3; ++a)
	for (b = -3; b <= 3; ++b)
		print(a, b, ~a, a&b, a^b, a|b, !a, a&&b, a||b);
fubar
if test $? -ne 0 ; then no_result; fi
cat > test.ok << 'fubar'
-3 -3 2 -3 0 -3 false true true
-3 -2 2 -4 3 -1 false true true
-3 -1 2 -3 2 -1 false true true
-3 0 2 0 -3 -3 false false true
-3 1 2 1 -4 -3 false true true
-3 2 2 0 -1 -1 false true true
-3 3 2 1 -2 -1 false true true
-2 -3 1 -4 3 -1 false true true
-2 -2 1 -2 0 -2 false true true
-2 -1 1 -2 1 -1 false true true
-2 0 1 0 -2 -2 false false true
-2 1 1 0 -1 -1 false true true
-2 2 1 2 -4 -2 false true true
-2 3 1 2 -3 -1 false true true
-1 -3 0 -3 2 -1 false true true
-1 -2 0 -2 1 -1 false true true
-1 -1 0 -1 0 -1 false true true
-1 0 0 0 -1 -1 false false true
-1 1 0 1 -2 -1 false true true
-1 2 0 2 -3 -1 false true true
-1 3 0 3 -4 -1 false true true
0 -3 -1 0 -3 -3 true false true
0 -2 -1 0 -2 -2 true false true
0 -1 -1 0 -1 -1 true false true
0 0 -1 0 0 0 true false false
0 1 -1 0 1 1 true false true
0 2 -1 0 2 2 true false true
0 3 -1 0 3 3 true false true
1 -3 -2 1 -4 -3 false true true
1 -2 -2 0 -1 -1 false true true
1 -1 -2 1 -2 -1 false true true
1 0 -2 0 1 1 false false true
1 1 -2 1 0 1 false true true
1 2 -2 0 3 3 false true true
1 3 -2 1 2 3 false true true
2 -3 -3 0 -1 -1 false true true
2 -2 -3 2 -4 -2 false true true
2 -1 -3 2 -3 -1 false true true
2 0 -3 0 2 2 false false true
2 1 -3 0 3 3 false true true
2 2 -3 2 0 2 false true true
2 3 -3 2 1 3 false true true
3 -3 -4 1 -2 -1 false true true
3 -2 -4 2 -3 -1 false true true
3 -1 -4 3 -4 -1 false true true
3 0 -4 0 3 3 false false true
3 1 -4 1 2 3 false true true
3 2 -4 2 1 3 false true true
3 3 -4 3 0 3 false true true
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -rpt -f test.in -o test.out -unf
if test $? -ne 0 ; then fail; fi
diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test shift operators
#
cat > test.in << 'fubar'
title("shift operators", "delete this line");
columns
(
	{ name = "Index";   width = 5; },
	{ name = "    Left"; width = 8; },
	{ name = "   Right"; width = 8; }
);
auto a;
for (a = 0; a < 32; ++a)
{
	print
	(
		a,
		sprintf("%X", 1 << a),
		sprintf("%X", 0x80000000 >> a)
	);
}
fubar
if test $? -ne 0 ; then no_result; fi
cat > test.ok << 'fubar'
0 1 80000000
1 2 40000000
2 4 20000000
3 8 10000000
4 10 8000000
5 20 4000000
6 40 2000000
7 80 1000000
8 100 800000
9 200 400000
10 400 200000
11 800 100000
12 1000 80000
13 2000 40000
14 4000 20000
15 8000 10000
16 10000 8000
17 20000 4000
18 40000 2000
19 80000 1000
20 100000 800
21 200000 400
22 400000 200
23 800000 100
24 1000000 80
25 2000000 40
26 4000000 20
27 8000000 10
28 10000000 8
29 20000000 4
30 40000000 2
31 80000000 1
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -rpt -f test.in -o test.out -unf
if test $? -ne 0 ; then fail; fi
diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test join operator
#
cat > test.in << 'fubar'
title("join operator", "delete this line");
columns("a", "b", "a ## b");
auto a, b, ab;
for (a in ["a", 42, 1.3, [1, 2, "3"]])
{
	auto ta, ca, tb, cb, tab, cab;
	ta = typeof(a);
	ca = ta == "list" ? ta : a;
	for (b in ["a", 42, 1.3, [1, 2, "3"]])
	{
		tb = typeof(b);
		cb = tb == "list" ? tb : b;
		ab = a ## b;
		tab = typeof(ab);
		if (tab == "list")
			cab = tab;
		else
			cab = ab;
		print(ca, cb, cab);
	}
}
fubar
if test $? -ne 0 ; then no_result; fi
cat > test.ok << 'fubar'
a a aa
a 42 a42
a 1.3 a1.3
a list list
42 a 42a
42 42 4242
42 1.3 421.3
42 list list
1.3 a 1.3a
1.3 42 1.342
1.3 1.3 1.31.3
1.3 list list
list a list
list 42 list
list 1.3 list
list list list
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -rpt -f test.in -o test.out -unf
if test $? -ne 0 ; then fail; fi
diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test assignment operators
#
cat > test.in << 'fubar'
title("assignment operators", "delete this line");
columns({ name = "Result"; width = 40; });
auto a;
a = 0;		print(a);
a += 2;		print(a);
a -= 6;		print(a);
a *= -5.6;	print(a);
a /= 4.2;	print(a);
a %= 2;		print(a);
a &= 1;		print(a);
a ^= 9;		print(a);
a |= 2;		print(a);
a <<= 3;	print(a);
a >>= 1;	print(a);
a **= 2.1;	print(a);
a ##= "boo!";	print(a);
fubar
if test $? -ne 0 ; then no_result; fi
cat > test.ok << 'fubar'
0
2
-4
22.4
5.33333
1.33333
1
8
10
80
40
2313.8
2313.8boo!
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -rpt -f test.in -o test.out -unf
if test $? -ne 0 ; then fail; fi
diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test report search path
#
cat > $work/lib/report.index << 'fubar'
where =
[
	{
		name = "Test";
		description = "look for this line";
		filename = "test.rpt";
	}
];
fubar
if test $? -ne 0 ; then no_result; fi
cat > $work/lib/test.rpt << 'fubar'
title("report searching", "delete this line");
columns("x", "sqrt(x)", "sqr(x)");
auto a;
for (a = 0; a < 10; ++a)
	print(a, a ** 0.5, a ** 2);
fubar
if test $? -ne 0 ; then no_result; fi
cat > test.ok << 'fubar'
0 0 0
1 1 1
2 1.41421 4
3 1.73205 9
4 2 16
5 2.23607 25
6 2.44949 36
7 2.64575 49
8 2.82843 64
9 3 81
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -rpt t -o test.out -unf
if test $? -ne 0 ; then fail; fi
diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test report listing
#
cat > test.ok << fubar
Test look for this line
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -rpt -list -unf > test.out
if test $? -ne 0 ; then fail; fi
grep 'look for this line' test.out > test.out2
if test $? -ne 0 ; then no_result; fi
sed -e 's| /.*||' < test.out2 > test.out3
if test $? -ne 0 ; then no_result; fi
diff -b test.ok test.out3
if test $? -ne 0 ; then fail; fi

#
# test getuid() function
#
cat > test.in << 'fubar'
title("getuid function", "delete this line");
columns({ width = 1000; });
auto who_am_i;
print(passwd[getuid()].pw_name);
fubar
if test $? -ne 0 ; then no_result; fi
echo $USER > test.ok
if test $? -ne 0 ; then no_result; fi
$bin/aegis -rpt -f test.in -o test.out -unf
if test $? -ne 0 ; then fail; fi
diff -b test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
