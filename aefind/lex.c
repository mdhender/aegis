/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 1998 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to perform lexical analysis
 */

#include <ac/stdarg.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex2.h>
#include <cmdline.gen.h>
#include <lex.h>
#include <progname.h>
#include <sub.h>

static arglex_table_ty argtab[] =
{
	{ "!",			arglex_token_not,	},
	{ "!=",			arglex_token_ne,	},
	{ "#",			arglex_token_stringize,	},
	{ "##",			arglex_token_join,	},
	{ "%",			arglex_token_mod,	},
	{ "&",			arglex_token_bit_and,	},
	{ "&&",			arglex_token_and,	},
	{ "(",			arglex_token_left_paren, },
	{ ")",			arglex_token_right_paren, },
	{ "+",			arglex_token_plus,	},
	{ ",",			arglex_token_comma,	},
	{ "-Acess_Time",	arglex_token_atime,	},
	{ "-And",		arglex_token_and,	},
	{ "-Change_Time",	arglex_token_ctime,	},
	{ "-Debug",		arglex_token_debug,	},
	{ "-Equals",		arglex_token_eq,	},
	{ "-Greater_Than",	arglex_token_gt,	},
	{ "-Greater_than_or_Equal", arglex_token_ge,	},
	{ "-Less_Than",		arglex_token_lt,	},
	{ "-Less_than_or_Equal", arglex_token_le,	},
	{ "-Modify_Time",	arglex_token_mtime,	},
	{ "-NEWer",		arglex_token_newer,	},
	{ "-Name",		arglex_token_namekw,	},
	{ "-Not",		arglex_token_not,	},
	{ "-Not_Equal",		arglex_token_ne,	},
	{ "-Or",		arglex_token_or,	},
	{ "-PAth",		arglex_token_path,	},
	{ "-PRint",		arglex_token_print,	},
	{ "-Resolve",		arglex_token_resolve,	},
	{ "-Size",		arglex_token_size,	},
	{ "-Type",		arglex_token_type,	},
	{ ".EQ.",		arglex_token_eq,	},
	{ ".GE",		arglex_token_ge,	},
	{ ".GT.",		arglex_token_gt,	},
	{ ".LE.",		arglex_token_le,	},
	{ ".LT.",		arglex_token_lt,	},
	{ ".NE.",		arglex_token_ne,	},
	{ "/",			arglex_token_div,	},
	{ "<",			arglex_token_lt,	},
	{ "<<",			arglex_token_shift_left, },
	{ "<=",			arglex_token_le,	},
	{ "=",			arglex_token_eq,	},
	{ "==",			arglex_token_eq,	},
	{ ">",			arglex_token_gt,	},
	{ ">=",			arglex_token_ge,	},
	{ ">>",			arglex_token_shift_right, },
	{ "\\*",		arglex_token_mul,	},
	{ "^",			arglex_token_bit_xor,	},
	{ "_FAlse",		arglex_token_false,	},
	{ "_NOW",		arglex_token_now,	},
	{ "_TRue",		arglex_token_true,	},
	{ "{}",			arglex_token_this,	},
	{ "|",			arglex_token_bit_or,	},
	{ "||",			arglex_token_or,	},
	{ "~",			arglex_token_tilde,	},
	ARGLEX_END_MARKER
};

static int number_of_errors;


void
cmdline_lex_open(argc, argv)
	int		argc;
	char		**argv;
{
	arglex2_init3(argc, argv, argtab);
	number_of_errors = 0;
}


int
cmdline_lex()
{
	switch (arglex())
	{
	case arglex_token_eoln:
		return 0;

	case arglex_token_number:
		cmdline_lval.lv_number = arglex_value.alv_number;
		return NUMBER;

#if 0
	case arglex_token_double:
		cmdline_lval.lv_real = arglex_value.alv_double;
		return REAL;
#endif

	case arglex_token_string:
		cmdline_lval.lv_string = str_from_c(arglex_value.alv_string);
		return STRING;

	case arglex_token_and:		return ANDAND;
	case arglex_token_atime:	return ATIME;
	case arglex_token_baseline:	return BASELINE;
	case arglex_token_base_relative: return BASE_REL;
	case arglex_token_bit_and:	return BIT_AND;
	case arglex_token_bit_or:	return BIT_OR;
	case arglex_token_bit_xor:	return BIT_XOR;
	case arglex_token_branch:	return BRANCH;
	case arglex_token_change:	return CHANGE;
	case arglex_token_comma:	return COMMA;
	case arglex_token_ctime:	return CTIME;
	case arglex_token_current_relative: return CUR_REL;
	case arglex_token_debug:	return DEBUG_keyword;
	case arglex_token_div:		return DIV;
	case arglex_token_eq:		return EQ;
	case arglex_token_false:	return FALSE_keyword;
	case arglex_token_ge:		return GE;
	case arglex_token_grandparent:	return GRANDPARENT;
	case arglex_token_gt:		return GT;
	case arglex_token_stringize:	return STRINGIZE;
	case arglex_token_join:		return JOIN;
	case arglex_token_le:		return LE;
	case arglex_token_left_paren:	return LPAREN;
	case arglex_token_library:	return LIBRARY;
	case arglex_token_lt:		return LT;
	case arglex_token_mod:		return MOD;
	case arglex_token_mtime:	return MTIME;
	case arglex_token_mul:		return MUL;
	case arglex_token_namekw:	return NAME;
	case arglex_token_ne:		return NE;
	case arglex_token_newer:	return NEWER;
	case arglex_token_not:		return NOT;
	case arglex_token_now:		return NOW;
	case arglex_token_or:		return OROR;
	case arglex_token_path:		return PATH;
	case arglex_token_plus:		return PLUS;
	case arglex_token_print:	return PRINT;
	case arglex_token_project:	return PROJECT;
	case arglex_token_resolve:	return RESOLVE;
	case arglex_token_right_paren:	return RPAREN;
	case arglex_token_size:		return SSIZE;
	case arglex_token_stdio:	return MINUS;
	case arglex_token_this:		return THIS;
	case arglex_token_tilde:	return TILDE;
	case arglex_token_trace:	return TRACE;
	case arglex_token_true:		return TRUE_keyword;
	case arglex_token_trunk:	return TRUNK;
	case arglex_token_type:		return TYPE;
	case arglex_token_verbose:	return DEBUG_keyword;

	default:
		return JUNK;
	}
}


void
cmdline_lex_error(scp, message)
	sub_context_ty	*scp;
	char		*message;
{
	error_intl(scp, message);
	++number_of_errors;
}


void
cmdline_error(message)
	char		*message;
{
	/* this function is for yacc */
	cmdline_lex_error(0, message);
}


void
usage()
{
	fprintf(stderr, "Usage: %s <path>... <expression>\n", progname_get());
	fprintf(stderr, "       %s -Help\n", progname_get());
	exit(1);
}


void
cmdline_lex_close()
{
	if (number_of_errors)
		usage();
}
