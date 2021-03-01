/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1997, 1999 Peter Miller;
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
 * MANIFEST: lexical analyzer
 */

#include <ac/stdio.h>
#include <ac/stdarg.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/errno.h>

#include <error.h>
#include <lex.h>
#include <mem.h>
#include <str.h>
#include <symtab.h>
#include <type.h>
#include <str_list.h>
#include <parse.gen.h> /* must be last */


#define strel(a, op, b) (strcmp(a, b) op 0)

typedef struct file_ty file_ty;
struct file_ty
{
	FILE	*fp;
	int	line_number;
	char	*file_name;
	file_ty	*next;
};

static	file_ty		*file;
static	int		error_count;
extern	parse_STYPE	parse_lval;
static	string_list_ty		ifiles;
static	string_list_ty		include_path;
static symtab_ty	*keyword;


/*
 *  NAME
 *	lex_initialize - look for keywords
 *
 *  SYNOPSIS
 *	int lex_initialize(void);
 *
 *  DESCRIPTION
 *	The lex_initialize function adds all the keywords to the symbol table.
 *
 *  CAVEAT
 *	The keywords are intentionally case sensitive.
 *      Assumes that str_initialize has already been called.
 */

static void lex_initialize _((void));

static void
lex_initialize()
{
	typedef struct keyword_ty keyword_ty;
	struct keyword_ty
	{
		char	*k_name;
		int	k_token;
	};

	static keyword_ty table[] =
	{
		{ "include", INCLUDE, },
		{ "integer", INTEGER, },
		{ "real", REAL, },
		{ "string", STRING, },
		{ "time", TIME, },
		{ "type", TYPE, },
	};
	keyword_ty	*kp;

	if (keyword)
		return;
	keyword = symtab_alloc(SIZEOF(table));
	for (kp = table; kp < ENDOF(table); ++kp)
	{
		string_ty *s;

		s = str_from_c(kp->k_name);
		symtab_assign(keyword, s, &kp->k_token);
		str_free(s);
	}
}


void
lex_open(s)
	char	*s;
{
	file_ty	*f;

	f = (file_ty *)mem_alloc_clear(sizeof(file_ty));
	if (!file)
	{
		lex_initialize();
		f->file_name = mem_copy_string(s);
		f->fp = fopen(s, "r");
		if (!f->fp)
			nfatal("%s", s);
	}
	else
	{
		int	j;

		f->fp = 0;
		for (j = 0; j < include_path.nstrings; ++j)
		{
			char	buffer[2000];

			sprintf
			(
				buffer,
				"%s/%s",
				include_path.string[j]->str_text,
				s
			);
			f->fp = fopen(buffer, "r");
			if (f->fp)
			{
				f->file_name = mem_copy_string(buffer);
				break;
			}
			if (errno != ENOENT)
				nfatal("%s", buffer);
		}
		if (!f->fp)
		{
			f->fp = fopen(s, "r");
			if (!f->fp)
				nfatal("%s", s);
			f->file_name = mem_copy_string(s);
		}
		f->next = file;
		string_list_append_unique(&ifiles, str_from_c(s));
	}
	f->line_number = 1;
	file = f;
}


void
lex_close()
{
	if (error_count)
		exit(1);
	fclose(file->fp);
	free(file->file_name);
	free(file);
}


static int lex_getc _((void));

static int
lex_getc()
{
	int	c;

	for (;;)
	{
		file_ty	*old;

		c = fgetc(file->fp);
		if (c != EOF)
			break;
		if (ferror(file->fp))
			nfatal("%s", file->file_name);
		if (!file->next)
			break;
		old = file;
		file = old->next;
		fclose(old->fp);
		free(old->file_name);
		free(old);
	}
	if (c == '\n')
		file->line_number++;
	return c;
}


static void lex_getc_undo _((int));

static void
lex_getc_undo(c)
	int	c;
{
	switch (c)
	{
	case EOF:
		break;

	case '\n':
		file->line_number--;
		/* fall through... */

	default:
		ungetc(c, file->fp);
		break;
	}
}


int
parse_lex()
{
	int	line_number_start;
	char	buffer[1<<12];
	char	*cp;

	for (;;)
	{
		int	 c;

		c = lex_getc();
		switch (c)
		{
		case ' ':
		case '\t':
		case '\f':
		case '\n':
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': 
			parse_lval.lv_integer = 0;
			for (;;)
			{
				parse_lval.lv_integer =
					10 * parse_lval.lv_integer + c - '0';
				c = lex_getc();
				if (c < '0' || c > '9')
				{
					lex_getc_undo(c);
					break;
				}
			}
			return INTEGER_CONSTANT;

		case 'A': case 'B': case 'C': case 'D': case 'E':
		case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O':
		case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y':
		case 'Z': case '_': case 'a': case 'b': case 'c':
		case 'd': case 'e': case 'f': case 'g': case 'h':
		case 'i': case 'j': case 'k': case 'l': case 'm':
		case 'n': case 'o': case 'p': case 'q': case 'r':
		case 's': case 't': case 'u': case 'v': case 'w':
		case 'x': case 'y': case 'z': 
			{
				string_ty	*s;
				int		*data;
	
				cp = buffer;
				for (;;)
				{
					*cp++ = c;
					c = lex_getc();
					switch (c)
					{
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
					case '8': case '9': 
					case 'A': case 'B': case 'C': case 'D':
					case 'E': case 'F': case 'G': case 'H':
					case 'I': case 'J': case 'K': case 'L':
					case 'M': case 'N': case 'O': case 'P':
					case 'Q': case 'R': case 'S': case 'T':
					case 'U': case 'V': case 'W': case 'X':
					case 'Y': case 'Z': case '_': case 'a':
					case 'b': case 'c': case 'd': case 'e':
					case 'f': case 'g': case 'h': case 'i':
					case 'j': case 'k': case 'l': case 'm':
					case 'n': case 'o': case 'p': case 'q':
					case 'r': case 's': case 't': case 'u':
					case 'v': case 'w': case 'x': case 'y':
					case 'z': 
						continue;
					}
					lex_getc_undo(c);
					*cp = 0;
					break;
				}
				s = str_from_c(buffer);
				data = symtab_query(keyword, s);
				if (data)
				{
					str_free(s);
					return *data;
				}
				parse_lval.lv_string = s;
				return NAME;
			}

		case '/':
			c = lex_getc();
			if (c != '*')
			{
				lex_getc_undo(c);
				return '/';
			}
			for (;;)
			{
				for (;;)
				{
					c = lex_getc();
					if (c == EOF)
					{
						bad_comment:
						parse_error
						(
						    "end-of-file inside comment"
						);
						exit(1);
					}
					if (c == '*')
						break;
				}
				for (;;)
				{
					c = lex_getc();
					if (c == EOF)
						goto bad_comment;
					if (c != '*')
						break;
				}
				if (c == '/')
					break;
			}
			break;

		case '<':
	            line_number_start = file->line_number;
	            cp = buffer;
	            for (;;)
	            {
	                c = lex_getc();
	                if (c == EOF)
			    goto str_eof;
	                if (c == '\n')
			    goto str_eoln;
	                if (c == '>')
	                    break;
	                *cp++ = c;
	            }
	            *cp = 0;
	            parse_lval.lv_string = str_from_c(buffer);
	            return STRING_CONSTANT;

	        case '"':
	            line_number_start = file->line_number;
	            cp = buffer;
	            for (;;)
	            {
	                c = lex_getc();
	                if (c == EOF)
	                {
	                    str_eof:
	                    file->line_number = line_number_start;
	                    parse_error("end-of-file within string");
	                    break;
	                }
	                if (c == '\n')
	                {
			    str_eoln:
	                    file->line_number = line_number_start;
	                    parse_error("end-of-line within string");
	                    break;
	                }
	                if (c == '"')
	                    break;
	                if (c == '\\')
	                {
	                    c = lex_getc();
	                    switch (c)
	                    {
			    default:
				parse_error("unknown '\\%c' escape", c);
				break;
	
	                    case '\n':
	                        break;
	
	                    case EOF:
	                        goto str_eof;
	
	                    case 'b':
				*cp++ = '\b';
				break;
	
	                    case 'n':
				*cp++ = '\n';
				break;
	
	                    case 'r':
				*cp++ = '\r';
				break;
	
	                    case 't':
				*cp++ = '\t';
				break;
	
	                    case 'f':
				*cp++ = '\f';
				break;
	
			    case '"':
			    case '\\':
				*cp++ = c;
				break;
	
	                    case '0':
			    case '1':
			    case '2':
			    case '3':
	                    case '4':
			    case '5':
			    case '6':
			    case '7':
	                        {
	                            int             n;
	                            int             v;
	
				    v = 0;
	                            for (n = 0; n < 3; ++n)
	                            {
					v = v * 8 + c - '0';
					c = lex_getc();
					switch (c)
					{
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					    continue;
	
					default:
					    lex_getc_undo(c);
					    break;
					}
					break;
	                            }
				    *cp++ = v;
	                        }
	                        break;
	                    }
	                }
	                else
	                    *cp++ = c;
	            }
	            *cp = 0;
	            parse_lval.lv_string = str_from_c(buffer);
	            return STRING_CONSTANT;

		default:
			return c;
		}
	}
}


void
parse_error(s sva_last)
	char		*s;
	sva_last_decl
{
	va_list		ap;
	char		buffer[1000];

	sva_init(ap, s);
	vsprintf(buffer, s, ap);
	va_end(ap);
	error_raw("%s: %d: %s", file->file_name, file->line_number, buffer);
	if (++error_count >= 20)
		error_raw("%s: too many errors, bye!", file->file_name);
}


int
lex_in_include_file()
{
	return !!file->next;
}


void
lex_include_path(s)
	char	*s;
{
	string_list_append_unique(&include_path, str_from_c(s));
}
