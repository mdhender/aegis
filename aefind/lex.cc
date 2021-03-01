//
//	aegis - project change supervisor
//	Copyright (C) 1997-1999, 2002-2008 Peter Miller
//	Copyright (C) 2007 Walter Franzini
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdarg.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/progname.h>
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/sub.h>

#include <aefind/lex.h>
#include <aefind/tree.h>
#include <aefind/shorthand/stat.h>
#include <aefind/cmdline.gen.h> // must be last

static arglex_table_ty argtab[] =
{
    { "!", arglex_token_not, },
    { "!=", arglex_token_ne, },
    { "#", arglex_token_stringize, },
    { "##", arglex_token_join, },
    { "%", arglex_token_mod, },
    { "&", arglex_token_bit_and, },
    { "&&", arglex_token_and, },
    { "(", arglex_token_left_paren, },
    { ")", arglex_token_right_paren, },
    { "+", arglex_token_plus, },
    { ", ", arglex_token_comma, },
    { "-Acess_Time", arglex_token_atime, },
    { "-And", arglex_token_and, },
    { "-Change_Time", arglex_token_ctime, },
    { "-Debug", arglex_token_debug, },
    { "-EXecute", arglex_token_execute, },
    { "-Equals", arglex_token_eq, },
    { "-Greater_Than", arglex_token_gt, },
    { "-Greater_than_or_Equal", arglex_token_ge, },
    { "-Less_Than", arglex_token_lt, },
    { "-Less_than_or_Equal", arglex_token_le, },
    { "-Modify_Time", arglex_token_mtime, },
    { "-NEWer", arglex_token_newer, },
    { "-Name", arglex_token_namekw, },
    { "-Not", arglex_token_not, },
    { "-Not_Equal", arglex_token_ne, },
    { "-Or", arglex_token_or, },
    { "-PAth", arglex_token_path, },
    { "-PRInt", arglex_token_print, },
    { "-Resolve", arglex_token_resolve, },
    { "-Not_Resolve", arglex_token_resolve_not, },
    { "-Size", arglex_token_size, },
    { "-Type", arglex_token_type, },
    { ".EQ.", arglex_token_eq, },
    { ".GE", arglex_token_ge, },
    { ".GT.", arglex_token_gt, },
    { ".LE.", arglex_token_le, },
    { ".LT.", arglex_token_lt, },
    { ".NE.", arglex_token_ne, },
    { "/", arglex_token_div, },
    { ";", arglex_token_semicolon, },
    { "<", arglex_token_lt, },
    { "<<", arglex_token_shift_left, },
    { "<=", arglex_token_le, },
    { "=", arglex_token_eq, },
    { "==", arglex_token_eq, },
    { ">", arglex_token_gt, },
    { ">=", arglex_token_ge, },
    { ">>", arglex_token_shift_right, },
    { "\\*", arglex_token_mul, },
    { "^", arglex_token_bit_xor, },
    { "_FAlse", arglex_token_false, },
    { "_NOW", arglex_token_now, },
    { "_TRue", arglex_token_true, },
    { "{}", arglex_token_this, },
    { "{+}", arglex_token_this_resolved, },
    { "{-}", arglex_token_this_unresolved, },
    { "|", arglex_token_bit_or, },
    { "||", arglex_token_or, },
    { "~", arglex_token_tilde, },
    ARGLEX_END_MARKER
};

static int number_of_errors;


void
cmdline_lex_open(int argc, char **argv)
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
	trace(("arglex_token_eoln\n"));
	return 0;

    case arglex_token_number:
	trace(("arglex_token_number\n"));
	cmdline_lval.lv_number = arglex_value.alv_number;
	return NUMBER;

#if 0
    case arglex_token_double:
	trace(("arglex_token_double\n"));
	cmdline_lval.lv_real = arglex_value.alv_double;
	return REAL;
#endif

    case arglex_token_string:
	trace(("arglex_token_string\n"));
	cmdline_lval.lv_string = str_from_c(arglex_value.alv_string);
	return STRING;

    case arglex_token_and:
	trace(("arglex_token_and\n"));
	return ANDAND;

    case arglex_token_atime:
	trace(("arglex_token_atime\n"));
	return ATIME;

    case arglex_token_baseline:
	trace(("arglex_token_baseline\n"));
	return BASELINE;

    case arglex_token_base_relative:
	trace(("arglex_token_base_relative\n"));
	return BASE_REL;

    case arglex_token_bit_and:
	trace(("arglex_token_bit_and\n"));
	return BIT_AND;

    case arglex_token_bit_or:
	trace(("arglex_token_bit_or\n"));
	return BIT_OR;

    case arglex_token_bit_xor:
	trace(("arglex_token_bit_xor\n"));
	return BIT_XOR;

    case arglex_token_branch:
	trace(("arglex_token_branch\n"));
	return BRANCH;

    case arglex_token_change:
	trace(("arglex_token_change\n"));
	return CHANGE;

    case arglex_token_comma:
	trace(("arglex_token_comma\n"));
	return COMMA;

    case arglex_token_ctime:
	trace(("arglex_token_ctime\n"));
	return CTIME;

    case arglex_token_current_relative:
	trace(("arglex_token_current_relative\n"));
	return CUR_REL;

    case arglex_token_debug:
	trace(("arglex_token_debug\n"));
	return DEBUG_keyword;

    case arglex_token_div:
	trace(("arglex_token_div\n"));
	return DIV;

    case arglex_token_eq:
	trace(("arglex_token_eq\n"));
	return EQ;

    case arglex_token_execute:
	trace(("arglex_token_execute\n"));
	return EXECUTE;

    case arglex_token_false:
	trace(("arglex_token_false\n"));
	return FALSE_keyword;

    case arglex_token_ge:
	trace(("arglex_token_ge\n"));
	return GE;

    case arglex_token_grandparent:
	trace(("arglex_token_grandparent\n"));
	return GRANDPARENT;

    case arglex_token_gt:
	trace(("arglex_token_gt\n"));
	return GT;

    case arglex_token_help:
        trace(("arglex_token_help\n"));
        return HELP;

    case arglex_token_stringize:
	trace(("arglex_token_stringize\n"));
	return STRINGIZE;

    case arglex_token_join:
	trace(("arglex_token_join\n"));
	return JOIN;

    case arglex_token_le:
	trace(("arglex_token_le\n"));
	return LE;

    case arglex_token_left_paren:
	trace(("arglex_token_left_paren\n"));
	return LPAREN;

    case arglex_token_library:
	trace(("arglex_token_library\n"));
	return LIBRARY;

    case arglex_token_lt:
	trace(("arglex_token_lt\n"));
	return LT;

    case arglex_token_mod:
	trace(("arglex_token_mod\n"));
	return MOD;

    case arglex_token_mtime:
	trace(("arglex_token_mtime\n"));
	return MTIME;

    case arglex_token_mul:
	trace(("arglex_token_mul\n"));
	return MUL;

    case arglex_token_namekw:
	trace(("arglex_token_namekw\n"));
	return NAME;

    case arglex_token_ne:
	trace(("arglex_token_ne\n"));
	return NE;

    case arglex_token_newer:
	trace(("arglex_token_newer\n"));
	return NEWER;

    case arglex_token_not:
	trace(("arglex_token_not\n"));
	return NOT;

    case arglex_token_now:
	trace(("arglex_token_now\n"));
	return NOW;

    case arglex_token_or:
	trace(("arglex_token_or\n"));
	return OROR;

    case arglex_token_path:
	trace(("arglex_token_path\n"));
	return PATH;

    case arglex_token_plus:
	trace(("arglex_token_plus\n"));
	return PLUS;

    case arglex_token_print:
	trace(("arglex_token_print\n"));
	return PRINT;

    case arglex_token_project:
	trace(("arglex_token_project\n"));
	return PROJECT;

    case arglex_token_resolve:
	trace(("arglex_token_resolve\n"));
	return RESOLVE;

    case arglex_token_resolve_not:
	trace(("arglex_token_resolve_not\n"));
	return RESOLVE_NOT;

    case arglex_token_right_paren:
	trace(("arglex_token_right_paren\n"));
	return RPAREN;

    case arglex_token_semicolon:
	trace(("arglex_token_semicolon\n"));
	return SEMICOLON;

    case arglex_token_size:
	trace(("arglex_token_size\n"));
	return SSIZE;

    case arglex_token_stdio:
	trace(("arglex_token_stdio\n"));
	return MINUS;

    case arglex_token_this:
	trace(("arglex_token_this\n"));
	cmdline_lval.lv_number = -1;
	return THIS;

    case arglex_token_this_unresolved:
	trace(("arglex_token_this\n"));
	cmdline_lval.lv_number = 0;
	return THIS;

    case arglex_token_this_resolved:
	trace(("arglex_token_this\n"));
	cmdline_lval.lv_number = 1;
	return THIS;

    case arglex_token_tilde:
	trace(("arglex_token_tilde\n"));
	return TILDE;

    case arglex_token_trace:
	trace(("arglex_token_trace\n"));
	return TRACE;

    case arglex_token_true:
	trace(("arglex_token_true\n"));
	return TRUE_keyword;

    case arglex_token_trunk:
	trace(("arglex_token_trunk\n"));
	return TRUNK;

    case arglex_token_type:
	trace(("arglex_token_type\n"));
	return TYPE;

    case arglex_token_verbose:
	trace(("arglex_token_verbose\n"));
	return DEBUG_keyword;

    case arglex_token_version:
	return VERSION;

    default:
	trace(("JUNK\n"));
	return JUNK;
    }
}


void
cmdline_lex_error(sub_context_ty *scp, const char *message)
{
    error_intl(scp, message);
    ++number_of_errors;
}


void
cmdline_error(const char *message)
{
    // this function is for yacc
    cmdline_lex_error(0, message);
}


void
usage(void)
{
    fprintf(stderr, "Usage: %s <path>... <expression>\n", progname_get());
    fprintf(stderr, "       %s -Help\n", progname_get());
    exit(1);
}


void
cmdline_lex_close(void)
{
    if (number_of_errors)
	usage();
}
