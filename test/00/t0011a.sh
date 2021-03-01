#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991-1998, 2002-2004 Peter Miller;
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
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: Test documentation example.
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
unset LINES
unset COOK
umask 022

USER=${USER:-${LOGNAME:-`whoami`}}

PAGER=cat
export PAGER

AEGIS_FLAGS="delete_file_preference = no_keep; \
	lock_wait_preference = always; \
	diff_preference = automatic_merge; \
	pager_preference = never; \
	persevere_preference = all; \
	log_file_preference = never;"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

COLS=65
export COLS
work=${AEGIS_TMP:-/tmp}/$$

if cook -version > /dev/null 2>&1
then
	:
else
	echo ''
	echo '	The "cook" program is not in your command search PATH.'
	echo '	This is test is therefore -assumed- to pass.'
	echo ''
	exit 0
fi

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

no_result()
{
	set +x
	echo "NO RESULT for test of documentation example ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
fail()
{
	set +x
	echo "FAILED test of documentation example ($activity)" 1>&2
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
trap "no_result" 1 2 3 15

#
# some variable to make things earier to read
#
worklib=$work/lib
workproj=$work/example.proj
workchan=$work/example.chan
tmp=$work/tmp

#
# make the directories
#
activity="working directory 94"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

AEGIS_PATH=$worklib
export AEGIS_PATH
PATH=$bin:$PATH
export PATH
AEGIS_PROJECT=example
export AEGIS_PROJECT

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# If the C++ compiler is called something other than ``c++'', as
# discovered by the configure script, create a shell script called
# ``c++'' which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
#
if test "$CXX" != "" -a "$CXX" != "c++"
then
	cat >> c++ << fubar
#!/bin/sh
exec $CXX \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx c++
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# make a new project
#
activity="new project 134"
$bin/aegis -newpro example -version "" -dir $workproj -v -lib $worklib > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# change project attributes
#
activity="project attributes 141"
cat > $tmp << 'TheEnd'
description = "aegis user's guide";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -proatt -f $tmp -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# create a new change
#
activity="new change 156"
cat > $tmp << 'TheEnd'
brief_description = "Place under aegis";
description = "A simple calculator using native floating point precision.  \
The four basic arithmetic operators to be provided, \
using conventional infix notation.  \
Parentheses and negation also required.";
cause = internal_enhancement;
TheEnd
if test $? -ne 0 ; then no_result; fi

$bin/aegis -new_change 1 -f $tmp -project example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# add a new developer
#
activity="new developer 173"
$bin/aegis -newdev $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# begin development of the change
#
activity="develop begin 180"
$bin/aegis -devbeg -l -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -devbeg 1 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# add the new files to the change
#
activity="new file 189"
$bin/aegis -new_file $workchan/Howto.cook $workchan/config $workchan/gram.y \
	$workchan/lex.l $workchan/main.cc -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $workchan/Howto.cook << 'TheEnd'
set unlink mkdir;
if [match_mask %1C%2 x[version]] then
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
	[fromto %.cc %.o [match_mask %.cc [source_files]]]
	;

cc_flags = -O;
cc_include_flags = [prepost "-I" "" [search_list]];

%.o: %.cc
{
	c++ [cc_include_flags] [cc_flags]
		-c [resolve %.cc];
}

/*
 * How to use yacc sources.
 */
if [find_command bison] then
	yacc = bison -y;
else if [find_command yacc] then
	yacc = yacc;
else if [find_command byacc] then
	yacc = byacc;
else
	yacc = yacc;
yacc_flags = ;

%.cc %.h: %.y
{
	[yacc] -d [yacc_flags] [resolve %.y];
	mv y.tab.c %.cc;
	mv y.tab.h %.h;
}


/*
 * How to use lex sources.
 */
if [find_command flex] then
	lex = flex;
else if [find_command lex] then
	lex = lex;
else
{
	echo "'no lex command found on this machine'"
		set silent;
	fail;
}
lex_flags = ;

%.cc: %.l
{
	[lex] [lex_flags] [resolve %.l];
	mv lex.yy.c %.cc;
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
	c++ -o [target] [resolve [obj_files]] -lm;
}

lex.o: gram.h;
TheEnd
if test $? -ne 0 ; then fail; fi

cat > $workchan/config << 'TheEnd'
build_command = "cook -b ${s Howto.cook} project=$p change=$c version=$v -nl";
link_integration_directory = true;

history_get_command =
	"co -u'$e' -p $h,v > $o";
history_create_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_put_command =
	"ci -f -u -m/dev/null -t/dev/null $i $h,v; rcs -U $h,v";
history_query_command =
	"rlog -r $h,v | awk '/^head:/ {print $$2}'";

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";

merge_command =
    "(diff3 -e $i $orig $mr | sed -e '/^w$$/d' -e '/^q$$/d'; echo '1,$$p' ) "
    "| ed - $i > $out";
TheEnd
if test $? -ne 0 ; then fail; fi

cat > $workchan/lex.l << 'TheEnd'
%{
#include <math.h>
#include <gram.h>
extern double atof(); /* sometimes missing from math.h */
extern "C" int yywrap();
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
extern int yylex();
extern void yyerror(char *);
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
		{ fflush(stderr); fflush(stdout); }
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

cat > $workchan/main.cc << 'TheEnd'
#include <stdlib.h>
#include <stdio.h>

extern int yyparse();

void
usage()
{
	fprintf(stderr, "usage: example\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	if (argc != 1)
		usage();
	yyparse();
	return 0;
}

void
yyerror(char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}

extern "C" int
yywrap()
{
	return 1;
}
TheEnd
if test $? -ne 0 ; then fail; fi

#
# create a new test
#
activity="new test 416"
$bin/aegis -nt -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

sed -e 's/^X//' > $workchan/test/00/t0001a.sh << 'TheEnd'
X:
Xhere=`pwd`
Xif test $? -ne 0 ; then exit 2; fi
Xtmp=/tmp/$$
Xmkdir $tmp
Xif test $? -ne 0 ; then exit 2; fi
Xcd $tmp
Xif test $? -ne 0 ; then exit 2; fi
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
# build the change
#
activity="build 483"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 490"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# test the change
#
activity="test 497"
$bin/aegis -test -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# finish development of the change
#
activity="develop end 504"
$bin/aegis -dev_end -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# add a new reviewer
#
activity="new reviewer 511"
$bin/aegis -newrev $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 518"
$bin/aegis -review_pass -list -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -review_pass -chan 1 -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# add an integrator
#
activity="new integrator 527"
$bin/aegis -newint $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# start integrating
#
activity="integrate begin 534"
$bin/aegis -intbeg -list -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -intbeg 1 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -list cd -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# integrate build and test
#
activity="build 545"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -test -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the integration
#
activity="integrate pass 554"
$bin/aegis -intpass -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

# -- 2 -----------------------------------------------------------------

#
# create the second and subsequent changes
#
activity="new change 563"
cat > $tmp << 'fubar'
brief_description = "file names on command line";
description = "Optional input and output files may be \
specified on the command line.";
cause = internal_bug;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -new_change 2 -f $tmp -project example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="new change 574"
cat > $tmp << 'fubar'
brief_description = "add powers";
description = "Enhance the grammar to allow exponentiation.  \
No error checking required.";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -new_change 3 -f $tmp -project example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

activity="new change 585"
cat > $tmp << 'fubar'
brief_description = "add variables";
description = "Enhance the grammar to allow variables.  \
Only single letter variable names are required.";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then fail; fi
$bin/aegis -new_change 4 -f $tmp -project example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# begin development of the change
#
activity="develop begin 599"
$bin/aegis -devbeg -l -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -devbeg 2 -dir $workchan -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# add the new files to the change
#
activity="copy file 608"
$bin/aegis -copy_file $workchan/main.cc -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $workchan/main.cc << 'TheEnd'
#include <stdlib.h>
#include <stdio.h>

extern int yyparse();

void
usage()
{
	fprintf(stderr, "usage: example [ <infile> [ <outfile> ]]\n");
	exit(1);
}

int
main(int argc, char **argv)
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
	return 0;
}

void
yyerror(char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}

extern "C" int
yywrap()
{
	return 1;
}
TheEnd
if test $? -ne 0 ; then fail; fi

#
# create a new test
#
activity="new test 676"
$bin/aegis -nt -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

sed -e 's/^X//' > $workchan/test/00/t0002a.sh << 'TheEnd'
X:
Xhere=`pwd`
Xif test $? -ne 0 ; then exit 2; fi
Xtmp=/tmp/$$
Xmkdir $tmp
Xif test $? -ne 0 ; then exit 2; fi
Xcd $tmp
Xif test $? -ne 0 ; then exit 2; fi
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
# build the change
#
activity="build 749"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 756"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# test the change
#
activity="test 763"
$bin/aegis -test -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="test baseline 766"
$bin/aegis -test -bl -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="test regression 769"
$bin/aegis -test -reg -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# finish development of the change
#
activity="develop end 776"
$bin/aegis -dev_end -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 783"
$bin/aegis -review_pass -list -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -review_pass -chan 2 -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# start integrating
#
activity="integrate begin 792"
$bin/aegis -intbeg -list -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -intbeg 2 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -list cd -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# integrate build and test
#
activity="build 803"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="test 806"
$bin/aegis -test -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="test baseline 809"
$bin/aegis -test -bl -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="test regression 812"
$bin/aegis -test -reg -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the integration
#
activity="integrate pass 819"
$bin/aegis -intpass -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

# --- 3 ----------------------------------------------------------------

#
# begin development of the change
#
activity="develop begin 828"
$bin/aegis -devbeg -l -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -devbeg 3 -dir $workchan.3 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# add the new files to the change
#
activity="copy file 837"
$bin/aegis -copy_file $workchan.3/gram.y -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $workchan.3/gram.y << 'TheEnd'
%{
#include <stdio.h>
#include <math.h>
extern int yylex();
extern void yyerror(char *);
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
		{ fflush(stderr); fflush(stdout); }
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

activity="diff 892"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
if test ! -r $workchan.3/gram.y,D ; then fail; fi

#
# create a new test
#
activity="new test 900"
$bin/aegis -nt -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

sed -e 's/^X//' > $workchan.3/test/00/t0003a.sh << 'TheEnd'
X:
Xhere=`pwd`
Xif test $? -ne 0 ; then exit 2; fi
Xtmp=/tmp/$$
Xmkdir $tmp
Xif test $? -ne 0 ; then exit 2; fi
Xcd $tmp
Xif test $? -ne 0 ; then exit 2; fi
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
Xif test $? -ne 0 ; then cat test.out; fail; fi
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
# build the change
#
activity="build 965"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 972"
$bin/aegis -diff -v -nl > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# test the change
#
activity="test 979"
$bin/aegis -test -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="test baseline 982"
$bin/aegis -test -bl -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="test regression 985"
$bin/aegis -test -reg -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

# --- 4 interrupts 3 ---------------------------------------------------

#
# begin development of the change
#
activity="develop begin 994"
$bin/aegis -devbeg -l -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -devbeg 4 -dir $workchan.4 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -l cd -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# add the new files to the change
#
activity="copy file 1005"
$bin/aegis -copy_file $workchan.4/gram.y -nl -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $workchan.4/gram.y << 'TheEnd'
%{
#include <stdio.h>
extern int yylex();
extern void yyerror(char *);
extern void assign(int, double);
extern double recall(int);
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
		{ fflush(stderr); fflush(stdout); }
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
		{ $$ = recall($1); }
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

activity="new file 1060"
$bin/aegis -new_file $workchan.4/var.cc -nl -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $workchan.4/var.cc << 'TheEnd'
static double memory[26];

void
assign(int name, double value)
{
	memory[name] = value;
}

double
recall(int name)
{
	return memory[name];
}
TheEnd
if test $? -ne 0 ; then fail; fi

#
# create a new test
#
activity="new test 1087"
$bin/aegis -nt -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

sed -e 's/^X//' > $workchan.4/test/00/t0004a.sh << 'TheEnd'
X:
Xhere=`pwd`
Xif test $? -ne 0 ; then exit 2; fi
Xtmp=/tmp/$$
Xmkdir $tmp
Xif test $? -ne 0 ; then exit 2; fi
Xcd $tmp
Xif test $? -ne 0 ; then exit 2; fi
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
Xif test $? -ne 0 ; then cat test.out; fail; fi
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
# build the change
#
activity="build 1152"
$bin/aegis -build -nl -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# difference the change
#
activity="diff 1159"
$bin/aegis -diff -v -nl -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
if test ! -r $workchan.4/gram.y,D ; then fail; fi

#
# test the change
#
activity="test 1167"
$bin/aegis -test -nl -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -test -bl -nl -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -test -reg -nl -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# finish development of the change
#
activity="develop end 1178"
$bin/aegis -dev_end -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 1185"
$bin/aegis -review_pass -list -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -review_pass -chan 4 -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# start integrating
#
activity="integrate begin 1194"
$bin/aegis -intbeg -list -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -intbeg 4 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -list cd -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# integrate build and test
#
activity="build 1205"
$bin/aegis -build -nl -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="test 1208"
$bin/aegis -test -nl -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -test -bl -nl -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -test -reg -nl -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the integration
#
activity="integrate pass 1219"
$bin/aegis -intpass -nl -v -c 4 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

# --- 3, again ---------------------------------------------------------

#
# finish development of the change
#
activity="develop end 1228"
$bin/aegis -dev_end -v -c 3 > log 2>&1
if test $? -ne 1 ; then cat log; fail; fi

#
# need a new difference
#
activity="merge 1235"
$bin/aegis -diff --merge-only -nl -v -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# look at the merge file
#
if test ! -r $workchan.3/gram.y,B ; then fail; fi
if test ! -r $workchan.3/gram.y ; then fail; fi

#
# need a new build
#
activity="build 1249"
$bin/aegis -build -nl -v -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# test it again
#
activity="test 1256"
$bin/aegis -test -nl -v -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="test -bl 1259"
$bin/aegis -test -bl -nl -v -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="test -reg 1262"
$bin/aegis -test -reg -nl -v -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# try to finish development of the change
#
activity="develop end 1269"
$bin/aegis -dev_end -v -c 3 > log 2>&1
if test $? -ne 1 ; then cat log; fail; fi

#
# diff again
#
activity="diff 1276"
$bin/aegis -diff -nl -v -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# finish development of the change
#
activity="develop end 1283"
$bin/aegis -dev_end -v -c 3 > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the review
#
activity="review pass 1290"
$bin/aegis -review_pass -list -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -review_pass -chan 3 -proj example -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# start integrating
#
activity="integrate begin 1299"
$bin/aegis -intbeg -list -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -intbeg 3 -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -list cd -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# integrate build and test
#
activity="build 1310"
$bin/aegis -build -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
activity="test 1313"
$bin/aegis -test -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -test -bl -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi
$bin/aegis -test -reg -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

#
# pass the integration
#
activity="integrate pass 1324"
$bin/aegis -intpass -nl -v > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

# ----------------------------------------------------------------------

#
# the things tested in this test, worked
# the things not tested in this test, may or may not work
#
pass
