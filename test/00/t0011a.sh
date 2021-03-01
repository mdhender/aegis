#! /bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991, 1992, 1993 Peter Miller.
#	All rights reserved.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# MANIFEST: Test documentation example.
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

PAGER=cat
export PAGER
COLS=65
export COLS
work=${AEGIS_TMP:-/tmp}/$$

fail()
{
	set +x
	echo FAILED test of documentation example 1>&2
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
pass()
{
	set +x
	echo PASSED test of documentation example 1>&2
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
}
trap "fail" 1 2 3 15

#
# some variable to make things earier to read
#
worklib=$work/lib
workproj=$work/example.proj
workchan=$work/example.chan
tmp=$work/tmp

#
# echo commands so we can tell what failed
#
set -x

#
# make the directories
#
mkdir $work
if test $? -ne 0 ; then fail; fi
here=`pwd`
if test $? -ne 0 ; then fail; fi

AEGIS=$worklib
export AEGIS
PATH=$here/bin:$PATH
export PATH
AEGIS_PROJECT=example
export AEGIS_PROJECT

#
# make a new project
#
$here/bin/aegis -newpro example -dir $workproj -v -lib $worklib
if test $? -ne 0 ; then fail; fi

#
# change project attributes
#
cat > $tmp << 'TheEnd'
description = "aegis user's guide";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
TheEnd
if test $? -ne 0 ; then fail; fi

$here/bin/aegis -proatt $tmp -proj example -v
if test $? -ne 0 ; then fail; fi

#
# create a new change
#
cat > $tmp << 'TheEnd'
brief_description = "Place under aegis";
description = "A simple calculator using native floating point precision.  \
The four basic arithmetic operators to be provided, \
using conventional infix notation.  \
Parentheses and negation also required.";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then fail; fi

$here/bin/aegis -new_change $tmp -project example -v
if test $? -ne 0 ; then fail; fi

#
# add a new developer
#
$here/bin/aegis -newdev $USER -v
if test $? -ne 0 ; then fail; fi

#
# begin development of the change
#
$here/bin/aegis -devbeg -l -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -devbeg 1 -dir $workchan -v
if test $? -ne 0 ; then fail; fi

#
# add the new files to the change
#
$here/bin/aegis -new_file $workchan/Howto.cook $workchan/config $workchan/gram.y \
	$workchan/lex.l $workchan/main.c -nl -v
if test $? -ne 0 ; then fail; fi

cat > $workchan/Howto.cook << 'TheEnd'
if [match_mask %1C%2 [version]] then
{
	baseline = [collect aegis -cd -bl -p [project]];
	search_list = . [baseline];
}
change_files = [collect aegis -l chafil -ter -p [project] -c [change]];
project_files = [collect aegis -l profil -ter -p [project] -c [change]];
source_files = [sort [stringset [project_files] [change_files]]];
obj_files =
	[fromto %.y %.o [match_mask %.y [source_files]]]
	[fromto %.l %.o [match_mask %.l [source_files]]]
	[fromto %.c %.o [match_mask %.c [source_files]]]
	;

cc = cc;
cc_flags = -O;
cc_include_flags = ;

%.o: %.c: [collect c_incl -nc -s [prepost "-I" "" [search_list]]
	[cc_include_flags] [resolve %.c]]
{
	if [exists %.o] then
		rm %.o
			set clearstat;
	[cc] [prepost "-I" "" [search_list]] [cc_include_flags] [cc_flags]
		-c [resolve %.c];
}


/*
 * How to use yacc sources.
 */
yacc = yacc;
yacc_flags = ;

%.c %.h: %.y
{
	if [exists %.list] then
		rm %.list
			set clearstat;
	if [exists y.output] then
		rm y.output
			set clearstat;
	if [exists %.c] then
		rm %.c
			set clearstat;
	if [exists %.h] then
		rm %.h
			set clearstat;
	[yacc] -d [yacc_flags] [resolve %.y];
	mv y.tab.c %.c;
	mv y.tab.h %.h;
	if [exists y.output] then
		mv y.output %.list
			set clearstat;
}


/*
 * How to use lex sources.
 */
lex = lex;
lex_flags = ;

%.c: %.l
{
	if [exists %.c] then
		rm %.c
			set clearstat;
	[lex] [lex_flags] [resolve %.l];
	mv lex.yy.c %.c;
}


/*
 * The default recipe
 */
all: example;

/*
 * build the program from the objects
 */

example: [obj_files]
{
	if [exists [target]] then
		rm [target]
			set clearstat;
	[cc] -o [target] [resolve [obj_files]] -ll -lm;
}
TheEnd
if test $? -ne 0 ; then fail; fi

cat > $workchan/config << 'TheEnd'
build_command = "cook -b ${s Howto.cook} project=$p change=$c version=$v -nl";
link_integration_directory = true;

history_get_command =
	"co -u'$e' -p $h,v > $o";
history_create_command =
	"ci -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_put_command =
	"ci -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_query_command =
	"rlog -r $h,v | awk '/^head:/ {print $$2}'";

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";

diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
	echo '1,$$p' ) | ed - $mr > $out";
TheEnd
if test $? -ne 0 ; then fail; fi

cat > $workchan/lex.l << 'TheEnd'
%{
#include <math.h>
#include <gram.h>
%}
%%
[0-9]+(\.[0-9]*)?([eE][+-]?[0-9]+)? {
		yylval.lv_double = atof(yytext);
		return DOUBLE;
	}
[a-z]	{
		yylval.lv_int = yytext[0] - 'a';
		return NAME;
	}
[ \t]+	;
\n	|
.	{ return yytext[0]; }
TheEnd
if test $? -ne 0 ; then fail; fi

cat > $workchan/gram.y << 'TheEnd'
%{
#include <stdio.h>
%}
%token DOUBLE
%token NAME
%union
{
	double	lv_double;
	int	lv_int;
};

%type <lv_double> DOUBLE expr
%type <lv_int> NAME
%left '+' '-'
%left '*' '/'
%right UNARY
%%
example
	: /* empty */
	| example command '\n'
		{ yyerrflag = 0; fflush(stderr); fflush(stdout); }
	;
command
	: expr
		{ printf("%g\n", $1); }
	| error
	;
expr
	: DOUBLE
	| '(' expr ')'
		{ $$ = $2; }
	| '-' expr
		%prec UNARY
		{ $$ = -$2; }
	| expr '*' expr
		{ $$ = $1 * $3; }
	| expr '/' expr
		{ $$ = $1 / $3; }
	| expr '+' expr
		{ $$ = $1 + $3; }
	| expr '-' expr
		{ $$ = $1 - $3; }
	;
TheEnd
if test $? -ne 0 ; then fail; fi

cat > $workchan/main.c << 'TheEnd'
#include <stdio.h>

void
usage()
{
	fprintf(stderr, "usage: example\n");
	exit(1);
}

void
main(argc, argv)
	int	argc;
	char	**argv;
{
	if (argc != 1)
		usage();
	yyparse();
	exit(0);
}

void
yyerror(s)
	char	*s;
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}
TheEnd
if test $? -ne 0 ; then fail; fi

#
# create a new test
#
$here/bin/aegis -nt -v
if test $? -ne 0 ; then fail; fi

sed -e 's/^X//' > $workchan/test/00/t0001a.sh << 'TheEnd'
X:
Xhere=`pwd`
Xif test $? -ne 0 ; then exit 1; fi
Xtmp=/tmp/$$
Xmkdir $tmp
Xif test $? -ne 0 ; then exit 1; fi
Xcd $tmp
Xif test $? -ne 0 ; then exit 1; fi
X
Xfail()
X{
X	echo SHUZBUTT 1>&2
X	cd $here
X	chmod u+w `find $tmp -type d -print`
X	rm -rf $tmp
X	exit 1
X}
Xpass()
X{
X	cd $here
X	chmod u+w `find $tmp -type d -print`
X	rm -rf $tmp
X	exit 0
X}
Xtrap "fail" 1 2 3 15
X
Xcat > test.in << 'end'
X1
X(4 - 2)
X-(3 - 6)
X8 * 0.5
X1.5 / 0.3
X4.5 + 1.5
X10.1 - 3.1
Xend
Xif test $? -ne 0 ; then fail; fi
X
Xcat > test.ok << 'end'
X1
X2
X3
X4
X5
X6
X7
Xend
Xif test $? -ne 0 ; then fail; fi
X
X$here/example < test.in > test.out 2>&1
Xif test $? -ne 0 ; then fail; fi
X
Xdiff test.ok test.out
Xif test $? -ne 0 ; then fail; fi
X
X# it probably worked
Xpass
TheEnd
if test $? -ne 0 ; then fail; fi

#
# let the clock tick over, so the build will be happy
#
sleep 1

#
# build the change
#
$here/bin/aegis -build -nl -v
if test $? -ne 0 ; then fail; fi

#
# difference the change
#
$here/bin/aegis -diff -v -nl
if test $? -ne 0 ; then fail; fi

#
# test the change
#
$here/bin/aegis -test -nl -v
if test $? -ne 0 ; then fail; fi

#
# finish development of the change
#
$here/bin/aegis -dev_end -v
if test $? -ne 0 ; then fail; fi

#
# add a new reviewer
#
$here/bin/aegis -newrev $USER -v
if test $? -ne 0 ; then fail; fi

#
# pass the review
#
$here/bin/aegis -review_pass -list -proj example -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -review_pass -chan 1 -proj example -v
if test $? -ne 0 ; then fail; fi

#
# add an integrator
#
$here/bin/aegis -newint $USER -v
if test $? -ne 0 ; then fail; fi

#
# start integrating
#
$here/bin/aegis -intbeg -list -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -intbeg 1 -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -list cd -v
if test $? -ne 0 ; then fail; fi

#
# integrate build and test
#
sleep 1
$here/bin/aegis -build -nl -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -nl -v
if test $? -ne 0 ; then fail; fi

#
# pass the integration
#
$here/bin/aegis -intpass -nl -v
if test $? -ne 0 ; then fail; fi

# -- 2 -----------------------------------------------------------------

#
# create the second and subsequent changes
#
cat > $tmp << 'fubar'
brief_description = "file names on command line";
description = "Optional input and output files may be \
specified on the command line.";
cause = internal_bug;
fubar
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -new_change $tmp -project example -v
if test $? -ne 0 ; then fail; fi

cat > $tmp << 'fubar'
brief_description = "add powers";
description = "Enhance the grammar to allow exponentiation.  \
No error checking required.";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -new_change $tmp -project example -v
if test $? -ne 0 ; then fail; fi

cat > $tmp << 'fubar'
brief_description = "add variables";
description = "Enhance the grammar to allow variables.  \
Only single letter variable names are required.";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -new_change $tmp -project example -v
if test $? -ne 0 ; then fail; fi

#
# begin development of the change
#
$here/bin/aegis -devbeg -l -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -devbeg 2 -dir $workchan -v
if test $? -ne 0 ; then fail; fi

#
# add the new files to the change
#
$here/bin/aegis -copy_file $workchan/main.c -nl -v
if test $? -ne 0 ; then fail; fi

cat > $workchan/main.c << 'TheEnd'
#include <stdio.h>

void
usage()
{
	fprintf(stderr, "usage: example [ <infile> [ <outfile> ]]\n");
	exit(1);
}

void
main(argc, argv)
	int	argc;
	char	**argv;
{
	char	*in = 0;
	char	*out = 0;
	int	j;

	for (j = 1; j < argc; ++j)
	{
		char *arg = argv[j];
		if (arg[0] == '-')
			usage();
		if (!in)
			in = arg;
		else if (!out)
			out = arg;
		else
			usage();
	}
	if (in && !freopen(in, "r", stdin))
	{
		perror(in);
		exit(1);
	}
	if (out && !freopen(out, "w", stdout))
	{
		perror(out);
		exit(1);
	}
	yyparse();
	exit(0);
}

void
yyerror(s)
	char	*s;
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}
TheEnd
if test $? -ne 0 ; then fail; fi

#
# create a new test
#
$here/bin/aegis -nt -v
if test $? -ne 0 ; then fail; fi

sed -e 's/^X//' > $workchan/test/00/t0002a.sh << 'TheEnd'
X:
Xhere=`pwd`
Xif test $? -ne 0 ; then exit 1; fi
Xtmp=/tmp/$$
Xmkdir $tmp
Xif test $? -ne 0 ; then exit 1; fi
Xcd $tmp
Xif test $? -ne 0 ; then exit 1; fi
X
Xfail()
X{
X	echo SHUZBUTT 1>&2
X	cd $here
X	chmod u+w `find $tmp -type d -print`
X	rm -rf $tmp
X	exit 1
X}
Xpass()
X{
X	cd $here
X	chmod u+w `find $tmp -type d -print`
X	rm -rf $tmp
X	exit 0
X}
Xtrap "fail" 1 2 3 15
X
Xcat > test.in << 'end'
X1
X(4 - 2)
X-(3 - 6)
X8 * 0.5
X1.5 / 0.3
X4.5 + 1.5
X10.1 - 3.1
Xend
Xif test $? -ne 0 ; then fail; fi
X
Xcat > test.ok << 'end'
X1
X2
X3
X4
X5
X6
X7
Xend
Xif test $? -ne 0 ; then fail; fi
X
X$here/example test.in < /dev/null > test.out 2>&1
Xif test $? -ne 0 ; then fail; fi
X
Xdiff test.ok test.out
Xif test $? -ne 0 ; then fail; fi
X
X$here/example test.in test.out.2 < /dev/null
Xif test $? -ne 0 ; then fail; fi
X
Xdiff test.ok test.out.2
Xif test $? -ne 0 ; then fail; fi
X
X# it probably worked
Xpass
TheEnd
if test $? -ne 0 ; then fail; fi

#
# let the clock tick over, so the build will be happy
#
sleep 1

#
# build the change
#
$here/bin/aegis -build -nl -v
if test $? -ne 0 ; then fail; fi

#
# difference the change
#
$here/bin/aegis -diff -v -nl
if test $? -ne 0 ; then fail; fi

#
# test the change
#
$here/bin/aegis -test -nl -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -bl -nl -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -reg -nl -v
if test $? -ne 0 ; then fail; fi

#
# finish development of the change
#
$here/bin/aegis -dev_end -v
if test $? -ne 0 ; then fail; fi

#
# pass the review
#
$here/bin/aegis -review_pass -list -proj example -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -review_pass -chan 2 -proj example -v
if test $? -ne 0 ; then fail; fi

#
# start integrating
#
$here/bin/aegis -intbeg -list -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -intbeg 2 -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -list cd -v
if test $? -ne 0 ; then fail; fi

#
# integrate build and test
#
sleep 1
$here/bin/aegis -build -nl -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -nl -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -bl -nl -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -reg -nl -v
if test $? -ne 0 ; then fail; fi

#
# pass the integration
#
$here/bin/aegis -intpass -nl -v
if test $? -ne 0 ; then fail; fi

# --- 3 ----------------------------------------------------------------

#
# begin development of the change
#
$here/bin/aegis -devbeg -l -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -devbeg 3 -dir $workchan.3 -v
if test $? -ne 0 ; then fail; fi

#
# add the new files to the change
#
$here/bin/aegis -copy_file $workchan.3/gram.y -nl -v
if test $? -ne 0 ; then fail; fi

cat > $workchan.3/gram.y << 'TheEnd'
%{
#include <stdio.h>
#include <math.h>
%}
%token DOUBLE
%token NAME
%union
{
	double	lv_double;
	int	lv_int;
};

%type <lv_double> DOUBLE expr
%type <lv_int> NAME
%left '+' '-'
%left '*' '/'
%right '^'
%right UNARY
%%
example
	: /* empty */
	| example command '\n'
		{ yyerrflag = 0; fflush(stderr); fflush(stdout); }
	;
command
	: expr
		{ printf("%g\n", $1); }
	| error
	;
expr
	: DOUBLE
	| '(' expr ')'
		{ $$ = $2; }
	| '-' expr
		%prec UNARY
		{ $$ = -$2; }
	| expr '^' expr
		{ $$ = pow($1, $3); }
	| expr '*' expr
		{ $$ = $1 * $3; }
	| expr '/' expr
		{ $$ = $1 / $3; }
	| expr '+' expr
		{ $$ = $1 + $3; }
	| expr '-' expr
		{ $$ = $1 - $3; }
	;
TheEnd
if test $? -ne 0 ; then fail; fi

$here/bin/aegis -diff -v -nl
if test $? -ne 0 ; then fail; fi
cat $workchan.3/gram.y,D
if test $? -ne 0 ; then fail; fi

#
# create a new test
#
$here/bin/aegis -nt -v
if test $? -ne 0 ; then fail; fi

sed -e 's/^X//' > $workchan.3/test/00/t0003a.sh << 'TheEnd'
X:
Xhere=`pwd`
Xif test $? -ne 0 ; then exit 1; fi
Xtmp=/tmp/$$
Xmkdir $tmp
Xif test $? -ne 0 ; then exit 1; fi
Xcd $tmp
Xif test $? -ne 0 ; then exit 1; fi
X
Xfail()
X{
X	echo SHUZBUTT 1>&2
X	cd $here
X	chmod u+w `find $tmp -type d -print`
X	rm -rf $tmp
X	exit 1
X}
Xpass()
X{
X	cd $here
X	chmod u+w `find $tmp -type d -print`
X	rm -rf $tmp
X	exit 0
X}
Xtrap "fail" 1 2 3 15
X
Xcat > test.in << 'end'
X5.3 ^ 0
X4 ^ 0.5
X27 ^ (1/3)
Xend
Xif test $? -ne 0 ; then fail; fi
X
Xcat > test.ok << 'end'
X1
X2
X3
Xend
Xif test $? -ne 0 ; then fail; fi
X
X$here/example test.in < /dev/null > test.out 2>&1
Xif test $? -ne 0 ; then fail; fi
X
Xdiff test.ok test.out
Xif test $? -ne 0 ; then fail; fi
X
X$here/example test.in test.out.2 < /dev/null
Xif test $? -ne 0 ; then fail; fi
X
Xdiff test.ok test.out.2
Xif test $? -ne 0 ; then fail; fi
X
X# it probably worked
Xpass
TheEnd
if test $? -ne 0 ; then fail; fi

#
# let the clock tick over, so the build will be happy
#
sleep 1

#
# build the change
#
$here/bin/aegis -build -nl -v
if test $? -ne 0 ; then fail; fi

#
# difference the change
#
$here/bin/aegis -diff -v -nl
if test $? -ne 0 ; then fail; fi

#
# test the change
#
$here/bin/aegis -test -nl -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -bl -nl -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -reg -nl -v
if test $? -ne 0 ; then fail; fi

# --- 4 interrupts 3 ---------------------------------------------------

#
# begin development of the change
#
$here/bin/aegis -devbeg -l -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -devbeg 4 -dir $workchan.4 -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -l cd -v -c 4
if test $? -ne 0 ; then fail; fi

#
# add the new files to the change
#
$here/bin/aegis -copy_file $workchan.4/gram.y -nl -v -c 4
if test $? -ne 0 ; then fail; fi

cat > $workchan.4/gram.y << 'TheEnd'
%{
#include <stdio.h>
%}
%token DOUBLE
%token NAME
%union
{
	double	lv_double;
	int	lv_int;
};

%type <lv_double> DOUBLE expr
%type <lv_int> NAME
%left '+' '-'
%left '*' '/'
%right UNARY
%%
example
	: /* empty */
	| example command '\n'
		{ yyerrflag = 0; fflush(stderr); fflush(stdout); }
	;
command
	: expr
		{ printf("%g\n", $1); }
	| NAME '=' expr
		{ assign($1, $3); }
	| error
	;
expr
	: DOUBLE
	| NAME
		{ extern double recall(); $$ = recall($1); }
	| '(' expr ')'
		{ $$ = $2; }
	| '-' expr
		%prec UNARY
		{ $$ = -$2; }
	| expr '*' expr
		{ $$ = $1 * $3; }
	| expr '/' expr
		{ $$ = $1 / $3; }
	| expr '+' expr
		{ $$ = $1 + $3; }
	| expr '-' expr
		{ $$ = $1 - $3; }
	;
TheEnd
if test $? -ne 0 ; then fail; fi

$here/bin/aegis -new_file $workchan.4/var.c -nl -v -c 4
if test $? -ne 0 ; then fail; fi

cat > $workchan.4/var.c << 'TheEnd'
static double memory[26];

void
assign(name, value)
	int	name;
	double	value;
{
	memory[name] = value;
}

double
recall(name)
	int	name;
{
	return memory[name];
}
TheEnd
if test $? -ne 0 ; then fail; fi

#
# create a new test
#
$here/bin/aegis -nt -v -c 4
if test $? -ne 0 ; then fail; fi

sed -e 's/^X//' > $workchan.4/test/00/t0004a.sh << 'TheEnd'
X:
Xhere=`pwd`
Xif test $? -ne 0 ; then exit 1; fi
Xtmp=/tmp/$$
Xmkdir $tmp
Xif test $? -ne 0 ; then exit 1; fi
Xcd $tmp
Xif test $? -ne 0 ; then exit 1; fi
X
Xfail()
X{
X	echo SHUZBUTT 1>&2
X	cd $here
X	chmod u+w `find $tmp -type d -print`
X	rm -rf $tmp
X	exit 1
X}
Xpass()
X{
X	cd $here
X	chmod u+w `find $tmp -type d -print`
X	rm -rf $tmp
X	exit 0
X}
Xtrap "fail" 1 2 3 15
X
Xcat > test.in << 'end'
Xa = 1
Xa + 1
Xc = a * 40 + 5
Xc / (a + 4)
Xend
Xif test $? -ne 0 ; then fail; fi
X
Xcat > test.ok << 'end'
X2
X9
Xend
Xif test $? -ne 0 ; then fail; fi
X
X$here/example test.in < /dev/null > test.out 2>&1
Xif test $? -ne 0 ; then fail; fi
X
Xdiff test.ok test.out
Xif test $? -ne 0 ; then fail; fi
X
X$here/example test.in test.out.2 < /dev/null
Xif test $? -ne 0 ; then fail; fi
X
Xdiff test.ok test.out.2
Xif test $? -ne 0 ; then fail; fi
X
X# it probably worked
Xpass
TheEnd
if test $? -ne 0 ; then fail; fi

#
# let the clock tick over, so the build will be happy
#
sleep 1

#
# build the change
#
$here/bin/aegis -build -nl -v -c 4
if test $? -ne 0 ; then fail; fi

#
# difference the change
#
$here/bin/aegis -diff -v -nl -c 4
if test $? -ne 0 ; then fail; fi
cat $workchan.4/gram.y,D
if test $? -ne 0 ; then fail; fi

#
# test the change
#
$here/bin/aegis -test -nl -v -c 4
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -bl -nl -v -c 4
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -reg -nl -v -c 4
if test $? -ne 0 ; then fail; fi

#
# finish development of the change
#
$here/bin/aegis -dev_end -v -c 4
if test $? -ne 0 ; then fail; fi

#
# pass the review
#
$here/bin/aegis -review_pass -list -proj example -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -review_pass -chan 4 -proj example -v
if test $? -ne 0 ; then fail; fi

#
# start integrating
#
$here/bin/aegis -intbeg -list -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -intbeg 4 -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -list cd -v -c 4
if test $? -ne 0 ; then fail; fi

#
# integrate build and test
#
sleep 1
$here/bin/aegis -build -nl -v -c 4
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -nl -v -c 4
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -bl -nl -v -c 4
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -reg -nl -v -c 4
if test $? -ne 0 ; then fail; fi

#
# pass the integration
#
$here/bin/aegis -intpass -nl -v -c 4
if test $? -ne 0 ; then fail; fi

# --- 3, again ---------------------------------------------------------

#
# finish development of the change
#
$here/bin/aegis -dev_end -v -c 3
if test $? -ne 1 ; then fail; fi

#
# need a new difference
#
$here/bin/aegis -diff -nl -v -c 3
if test $? -ne 0 ; then fail; fi

#
# look at the merge file
#
cat $workchan.3/gram.y,D
if test $? -ne 0 ; then fail; fi
mv $workchan.3/gram.y,D $workchan.3/gram.y
if test $? -ne 0 ; then fail; fi

#
# need a new build
#
sleep 1
$here/bin/aegis -build -nl -v -c 3
if test $? -ne 0 ; then fail; fi

#
# test it again
#
$here/bin/aegis -test -nl -v -c 3
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -bl -nl -v -c 3
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -reg -nl -v -c 3
if test $? -ne 0 ; then fail; fi

#
# try to finish development of the change
#
$here/bin/aegis -dev_end -v -c 3
if test $? -ne 1 ; then fail; fi

#
# diff again
#
$here/bin/aegis -diff -nl -v -c 3
if test $? -ne 0 ; then fail; fi

#
# finish development of the change
#
$here/bin/aegis -dev_end -v -c 3
if test $? -ne 0 ; then fail; fi

#
# pass the review
#
$here/bin/aegis -review_pass -list -proj example -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -review_pass -chan 3 -proj example -v
if test $? -ne 0 ; then fail; fi

#
# start integrating
#
$here/bin/aegis -intbeg -list -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -intbeg 3 -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -list cd -v
if test $? -ne 0 ; then fail; fi

#
# integrate build and test
#
sleep 1
$here/bin/aegis -build -nl -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -nl -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -bl -nl -v
if test $? -ne 0 ; then fail; fi
$here/bin/aegis -test -reg -nl -v
if test $? -ne 0 ; then fail; fi

#
# pass the integration
#
$here/bin/aegis -intpass -nl -v
if test $? -ne 0 ; then fail; fi

# ----------------------------------------------------------------------

#
# the things tested in this test, worked
# the things not tested in this test, may or may not work
#
pass
