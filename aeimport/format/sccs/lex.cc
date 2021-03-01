//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate lexs
//

#include <ac/ctype.h>
#include <ac/stdio.h>

#include <error.h>
#include <fopen_nfs.h>
#include <format/sccs/lex.h>
#include <input/file_text.h>
#include <str.h>
#include <stracc.h>
#include <sub.h>
#include <symtab.h>
#include <format/sccs/gram.gen.h> // needs to be after <str.h>

static input_ty *ip;
static int	error_count;
static int	start_of_line;
static stracc_t	getch_line_buffer;
static size_t	getch_line_pos;


void
sccs_lex_open(string_ty *fn)
{
    ip = input_file_text_open(fn);
    start_of_line = 1;
}


void
sccs_lex_close(void)
{
    if (error_count)
	quit(1);
    input_delete(ip);
    ip = 0;
}


static void
sccs_lex_error(sub_context_ty *scp, const char *s)
{
    string_ty       *msg;

    msg = subst_intl(scp, s);

    // re-use substitution context
    sub_var_set_string(scp, "Message", msg);
    sub_var_set_string(scp, "File_Name", input_name(ip));
    error_intl(scp, i18n("$filename: $message"));
    str_free(msg);

    if (++error_count >= 20)
    {
	// re-use substitution context
	sub_var_set_string(scp, "File_Name", input_name(ip));
	fatal_intl(scp, i18n("$filename: too many errors"));
    }
}


void
format_sccs_gram_error(const char *s)
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    sccs_lex_error(scp, s);
    sub_context_delete(scp);
}


static int
getch(void)
{
    int             c;

    if (getch_line_pos >= getch_line_buffer.length)
    {
	getch_line_buffer.length = 0;
	getch_line_pos = 0;
	for (;;)
	{
	    c = input_getc(ip);
	    if (c < 0)
	    {
		if (getch_line_buffer.length == 0)
	    	    return -1;
		break;
	    }
	    stracc_char(&getch_line_buffer, c);
	    if (c == '\n')
		break;
	}
    }
    start_of_line = !getch_line_pos;
    c = getch_line_buffer.buffer[getch_line_pos++];
    return c;
}


static void
ungetch(int c)
{
    if (c < 0)
	return;
    assert(getch_line_pos > 0);
    --getch_line_pos;
    assert(c == getch_line_buffer.buffer[getch_line_pos]);
}


#define yylval format_sccs_gram_lval


int
format_sccs_gram_lex(void)
{
    static stracc_t buffer;
    int             c;

    for (;;)
    {
	c = getch();
	if (c == EOF)
	    return 0;

	if (start_of_line)
	{
	    if (c != '\1')
	    {
		stracc_open(&buffer);
		for (;;)
		{
		    if (c == '\n')
			break;
		    stracc_char(&buffer, c);
		    c = getch();
		    if (c == EOF)
			break;
		}
		yylval.lv_string = stracc_close(&buffer);
		return TEXTLINE;
	    }

	    //
	    // we are looking for a keyword
	    //
	    c = getch();
	    switch (c)
	    {
	    case 'c':
		stracc_open(&buffer);
		c = getch();
		if (c != ' ')
		    stracc_char(&buffer, c);
		for (;;)
		{
		    c = getch();
		    if (c == EOF)
			break;
		    if (c == '\n')
		    {
			ungetch(c);
			break;
		    }
		    stracc_char(&buffer, c);
		}
		yylval.lv_string = stracc_close(&buffer);
		return COMMENT;

	    case 'd':
		return DELTA_BEGIN;

	    case 'D':
		return D_KEYWORD;

	    case 'e':
		return DELTA_END;

	    case 'E':
		return E_KEYWORD;

	    case 'f':
		return FLAGS;

	    case 'g':
		return MR_IGNORE;

	    case 'h':
		return HEADER;

	    case 'i':
		return MR_INCLUDE;

	    case 'I':
		return I_KEYWORD;

	    case 'm':
		return MR;

	    case 's':
		return SUMMARY;

	    case 't':
		return TITLE_BEGIN;

	    case 'T':
		return TITLE_END;

	    case 'u':
		return USERS_BEGIN;

	    case 'U':
		return USERS_END;

	    case 'x':
		return MR_EXCLUDE;
	    }
	    return JUNK;
	}

	//
	// Throw away white space.
	//
	if (isspace((unsigned char)c))
	    continue;

	//
	// "normal" processing
	//
	stracc_open(&buffer);
	for (;;)
	{
	    stracc_char(&buffer, c);
	    c = getch();
	    if (c == EOF)
		break;
	    if (isspace((unsigned char)c))
	    {
		ungetch(c);
		break;
	    }
	}
	yylval.lv_string = stracc_close(&buffer);
	return STRING;
    }
}
