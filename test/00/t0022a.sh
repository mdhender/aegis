#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994-1999, 2005-2008 Peter Miller
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

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

no_result()
{
	set +x
	echo "NO RESULT for test of basic report language ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of basic report language ($activity)" 1>&2
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

activity="working directory 81"
mkdir $work
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

mkdir $work/lib $work/lib/report
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi

AEGIS_PATH=$work/lib
export AEGIS_PATH

LINES=66
export LINES
COLS=65
export COLS

#
# create a project for later tests
#
$bin/aegis -new_proj foo -dir $work/proj -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

#
# test the basic report language
#
activity="basic report language 109"
cat > test.in << 'fubar'
title("Just A Test", "delete this line");
columns({name = "First"; width = 15;}, {name = "Second"; });
print(1, "two");
print(2.3);
print(3.4e-2);
print();
print("hello", "world");
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -report -file test.in -o test.out.raw -pw=64
if test $? -ne 0 ; then fail; fi

sed '/delete this line/d' < test.out.raw > test.out
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'



Just A Test						  Page 1

First		Second
1		two
2.3
0.034

hello		world
fubar
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the binary + operator
#
activity="binary + operator 147"
cat > test.in << 'fubar'
title("Binary Plus", "delete this line");
columns("r+r\n-------", "r+i\n-------", "i+r\n-------", "i+i\n-------");
print("6.1" + 7.2, 3.4 + 5, "1" + 0.2, 1 + 2 + 3);
print("6.1" * 7.2, 3.4 * 5, "12" * 0.2, 2 * 3);
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -report -file test.in -o test.out.raw -pw=64
if test $? -ne 0 ; then fail; fi

sed '/delete this line/d' < test.out.raw > test.out
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'



Binary Plus						  Page 1

r+r	r+i	i+r	i+i
------- ------- ------- -------
13.3	8.4	1.2	6
43.92	17	2.4	6
fubar
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the variable functionality
#
activity="variables 180"
cat > test.in << 'fubar'
title("Variables", "delete this line");
columns("N\n-------", "N^2\n-------", "<", "<=", ">", ">=", "==", "!=",
	{ name = "~~"; width = 9;});
auto j;
for (j = -16; j < 10; j = j + 1)
	print(j, j * j, j < 3, j <= 3, j > 3, j >= 3, j == 3, j != 3, j ~~ "A1");
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -report -file test.in -o test.out.raw -pw=64
if test $? -ne 0 ; then fail; fi

sed '/delete this line/d' < test.out.raw > test.out
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'



Variables						  Page 1

N	N^2	<	<=	>	>=	==	!=	~~
------- -------
-16	256	true	true	false	false	false	true	0.4
-15	225	true	true	false	false	false	true	0.4
-14	196	true	true	false	false	false	true	0.4
-13	169	true	true	false	false	false	true	0.4
-12	144	true	true	false	false	false	true	0.4
-11	121	true	true	false	false	false	true	0.4
-10	100	true	true	false	false	false	true	0.4
-9	81	true	true	false	false	false	true	0
-8	64	true	true	false	false	false	true	0
-7	49	true	true	false	false	false	true	0
-6	36	true	true	false	false	false	true	0
-5	25	true	true	false	false	false	true	0
-4	16	true	true	false	false	false	true	0
-3	9	true	true	false	false	false	true	0
-2	4	true	true	false	false	false	true	0
-1	1	true	true	false	false	false	true	0.5
0	0	true	true	false	false	false	true	0
1	1	true	true	false	false	false	true	0.666667
2	4	true	true	false	false	false	true	0
3	9	false	true	false	true	true	false	0
4	16	false	false	true	true	false	true	0
5	25	false	false	true	true	false	true	0
6	36	false	false	true	true	false	true	0
7	49	false	false	true	true	false	true	0
8	64	false	false	true	true	false	true	0
9	81	false	false	true	true	false	true	0
fubar
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the sprintf functionality
#
activity="sprintf function 239"
cat > test.in << 'fubar'
title("sprintf", "delete this line");
columns("%2d", "%c", "%5.2f", { name = "%.5e"; width = 15;}, "%-2s%2s", "%2X");
auto j;
for (j = 1; j < 32; j = j + 1)
	print
	(
		sprintf("%2d", j),
		sprintf("%c", j + 64),
		sprintf("%5.2f", j),
		sprintf("%.5e", (j * j) * (j * j)),
		sprintf("%-2s%2s", j, j-5),
		sprintf("%2X", j)
	);
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -report -file test.in -o test.out.raw -pw=64
if test $? -ne 0 ; then fail; fi

sed '/delete this line/d' < test.out.raw > test.out
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'



sprintf							  Page 1

%2d	%c	%5.2f	%.5e		%-2s%2s %2X
 1	A	 1.00	1.00000e+00	1 -4	 1
 2	B	 2.00	1.60000e+01	2 -3	 2
 3	C	 3.00	8.10000e+01	3 -2	 3
 4	D	 4.00	2.56000e+02	4 -1	 4
 5	E	 5.00	6.25000e+02	5  0	 5
 6	F	 6.00	1.29600e+03	6  1	 6
 7	G	 7.00	2.40100e+03	7  2	 7
 8	H	 8.00	4.09600e+03	8  3	 8
 9	I	 9.00	6.56100e+03	9  4	 9
10	J	10.00	1.00000e+04	10 5	 A
11	K	11.00	1.46410e+04	11 6	 B
12	L	12.00	2.07360e+04	12 7	 C
13	M	13.00	2.85610e+04	13 8	 D
14	N	14.00	3.84160e+04	14 9	 E
15	O	15.00	5.06250e+04	1510	 F
16	P	16.00	6.55360e+04	1611	10
17	Q	17.00	8.35210e+04	1712	11
18	R	18.00	1.04976e+05	1813	12
19	S	19.00	1.30321e+05	1914	13
20	T	20.00	1.60000e+05	2015	14
21	U	21.00	1.94481e+05	2116	15
22	V	22.00	2.34256e+05	2217	16
23	W	23.00	2.79841e+05	2318	17
24	X	24.00	3.31776e+05	2419	18
25	Y	25.00	3.90625e+05	2520	19
26	Z	26.00	4.56976e+05	2621	1A
27	[	27.00	5.31441e+05	2722	1B
28	\	28.00	6.14656e+05	2823	1C
29	]	29.00	7.07281e+05	2924	1D
30	^	30.00	8.10000e+05	3025	1E
31	_	31.00	9.23521e+05	3126	1F
fubar
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# test the passwd functionality
#
activity="passwd array 309"
cat > test.in << 'fubar'
title("The Password File", "");
columns
(
	{ name = "Login\n--------"; width = 8; },
	{ name = "Full Name\n----------"; width = 18; },
	{ name = "Directory\n-----------"; width = 23; },
	{ name = "Shell\n-------"; width = 11; },
	{ name = "Weight\n------"; }
);
auto j, p, w;
for (j in sort(keys(passwd)))
{
	try
		p = passwd[j];
	catch (p)
		continue;
	w = p.pw_name ~~ "pmiller";
	if (w < 0.8)
		continue;
	print
	(
		p.pw_name,
		p.pw_comment,
		p.pw_dir,
		p.pw_shell,
		sprintf("%4.2f", w)
	);
}
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -report -file test.in > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# test the "projects variable" functionality
#
activity="projects variable 347"
cat > test.in << 'fubar'
title("List of Projects", "");
columns
(
	{ name = "Name\n------"; width = 11; },
	{ name = "Directory\n-----------"; width = 19; },
	{ name = "Description\n-------------"; width = 23; }
);
auto j, p, n;
for (j in sort(keys(project)))
{
	p = project[j];
	print(p.name, p.directory, p.state.description);
	n = n + 1;
}
if (n == 0)
	print("", "", "Found no projects.");
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -report -file test.in > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
