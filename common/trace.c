/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1997 Peter Miller;
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
 * MANIFEST: functions for execution trace
 */

#include <stdio.h>
#include <ac/string.h>
#include <ac/stddef.h>
#include <ac/stdarg.h>

#include <error.h>
#include <mem.h>
#include <progname.h>
#include <str.h>
#include <trace.h>


#define INDENT 2

typedef struct known_ty known_ty;
struct known_ty
{
	string_ty	*filename;
	int		flag;
	int		*flag_p;
	known_ty	*next;
};

static	string_ty	*file_name;
static	int		line_number;
static	int		page_width;
static	known_ty	*known;
static	int		depth;


static string_ty *base_name _((char *));

static string_ty *
base_name(file)
	char		*file;
{
	char		*cp1;
	char		*cp2;

	cp1 = strrchr(file, '/');
	if (cp1)
		++cp1;
	else
		cp1 = file;
	cp2 = strrchr(cp1, '.');
	if (!cp2)
		cp2 = cp1 + strlen(cp1);
	if (cp2 > cp1 + 6)
		return str_n_from_c(cp1, 6);
	return str_n_from_c(cp1, cp2 - cp1);
}


int
trace_pretest(file, result)
	char		*file;
	int		*result;
{
	string_ty	*s;
	known_ty	*kp;

	s = base_name(file);
	for (kp = known; kp; kp = kp->next)
	{
		if (str_equal(s, kp->filename))
		{
			str_free(s);
			break;
		}
	}
	if (!kp)
	{
		kp = (known_ty *)mem_alloc(sizeof(known_ty));
		kp->filename = s;
		kp->next = known;
		kp->flag = 2; /* disabled */
		known = kp;
	}
	kp->flag_p = result;
	*result = kp->flag;
	return *result;
}


void
trace_where(file, line)
	char		*file;
	int		line;
{
	string_ty	*s;

	/*
	 * take new name fist, because will probably be same as last
	 * thus saving a free and a malloc (which are slow)
	 */
	s = base_name(file);
	if (file_name)
		str_free(file_name);
	file_name = s;
	line_number = line;
}


static void trace_putchar _((int));

static void
trace_putchar(c)
	int		c;
{
	static char	buffer[200];
	static char	*cp;
	static int	in_col;
	static int	out_col;

	if (!page_width)
	{
		/* don't use last column, many terminals are dumb */
		page_width = 79;
		/* allow for progname, filename and line number (8 each) */
		page_width -= 24;
		if (page_width < 16)
			page_width = 16;
	}
	if (!cp)
	{
		strcpy(buffer, progname_get());
		cp = buffer + strlen(buffer);
		if (cp > buffer + 6)
			cp = buffer + 6;
		*cp++ = ':';
		*cp++ = '\t';
		strcpy(cp, file_name->str_text);
		cp += file_name->str_length;
		*cp++ = ':';
		*cp++ = '\t';
		sprintf(cp, "%d:\t", line_number);
		cp += strlen(cp);
		in_col = 0;
		out_col = 0;
	}
	switch (c)
	{
	case '\n':
		*cp++ = '\n';
		*cp = 0;
		fflush(stdout);
		fputs(buffer, stderr);
		fflush(stderr);
		if (ferror(stderr))
			nfatal("(stderr)");
		cp = 0;
		break;

	case ' ':
		if (out_col)
			++in_col;
		break;

	case '\t':
		if (out_col)
			in_col = (in_col/INDENT + 1) * INDENT;
		break;

	case /*{*/'}':
	case /*(*/')':
	case /*[*/']':
		if (depth > 0)
			--depth;
		/* fall through */

	default:
		if (!out_col)
		{
			if (c != '#')
				/* modulo so never too long */
				in_col = (INDENT * depth) % page_width;
			else
				in_col = 0;
		}
		if (in_col >= page_width)
		{
			trace_putchar('\n');
			trace_putchar(c);
			return;
		}
		while (((out_col + 8) & -8) <= in_col && out_col + 1 < in_col)
		{
			*cp++ = '\t';
			out_col = (out_col + 8) & -8;
		}
		while (out_col < in_col)
		{
			*cp++ = ' ';
			++out_col;
		}
		if (c == '{'/*}*/ || c == '('/*)*/ || c == '['/*]*/)
			++depth;
		*cp++ = c;
		in_col++;
		out_col++;
		break;
	}
}


void
trace_printf(s sva_last)
	char		*s;
	sva_last_decl
{
	va_list		ap;
	char		buffer[3000];

	sva_init(ap, s);
	vsprintf(buffer, s, ap);
	va_end(ap);
	for (s = buffer; *s; ++s)
		trace_putchar(*s);
}


void
trace_enable(file)
	char		*file;
{
	string_ty	*s;
	known_ty	*kp;

	s = base_name(file);
	for (kp = known; kp; kp = kp->next)
	{
		if (str_equal(s, kp->filename))
		{
			str_free(s);
			break;
		}
	}
	if (!kp)
	{
		kp = (known_ty *)mem_alloc(sizeof(known_ty));
		kp->filename = s;
		kp->flag_p = 0;
		kp->next = known;
		known = kp;
	}
	kp->flag = 3; /* enabled */
	if (kp->flag_p)
		*kp->flag_p = kp->flag;

	/*
	 * this silences a warning...
	 */
#ifdef DEBUG
	trace_pretest_result = 1;
#endif
}


void
trace_char_real(name, vp)
	char		*name;
	char		*vp;
{
	trace_printf("%s = '", name);
	if (*vp < ' ' || *vp > '~' || strchr("(){}[]", *vp))
	{
		char	*s;

		s = strchr("\bb\nn\tt\rr\ff", *vp);
		if (s)
		{
			trace_putchar('\\');
			trace_putchar(s[1]);
		}
		else
			trace_printf("\\%03o", (unsigned char)*vp);
	}
	else
	{
		if (strchr("'\\", *vp))
			trace_putchar('\\');
		trace_putchar(*vp);
	}
	trace_printf("'; /* 0x%02X, %d */\n", (unsigned char)*vp, *vp);
}


void
trace_char_unsigned_real(name, vp)
	char		*name;
	unsigned char	*vp;
{
	trace_printf("%s = '", name);
	if (*vp < ' ' || *vp > '~' || strchr("(){}[]", *vp))
	{
		char	*s;

		s = strchr("\bb\nn\tt\rr\ff", *vp);
		if (s)
		{
			trace_putchar('\\');
			trace_putchar(s[1]);
		}
		else
			trace_printf("\\%03o", *vp);
	}
	else
	{
		if (strchr("'\\", *vp))
			trace_putchar('\\');
		trace_putchar(*vp);
	}
	trace_printf("'; /* 0x%02X, %d */\n", *vp, *vp);
}


void
trace_int_real(name, vp)
	char		*name;
	int		*vp;
{
	trace_printf("%s = %d;\n", name, *vp);
}


void
trace_int_unsigned_real(name, vp)
	char		*name;
	unsigned int	*vp;
{
	trace_printf("%s = %u;\n", name, *vp);
}


void
trace_long_real(name, vp)
	char		*name;
	long		*vp;
{
	trace_printf("%s = %ld;\n", name, *vp);
}


void
trace_long_unsigned_real(name, vp)
	char		*name;
	unsigned long	*vp;
{
	trace_printf("%s = %lu;\n", name, *vp);
}


void
trace_pointer_real(name, vptrptr)
	char		*name;
	void		*vptrptr;
{
	void		**ptr_ptr = vptrptr;
	void		*ptr;

	ptr = *ptr_ptr;
	if (!ptr)
		trace_printf("%s = NULL;\n", name);
	else
		trace_printf("%s = 0x%08lX;\n", name, ptr);
}


void
trace_short_real(name, vp)
	char		*name;
	short		*vp;
{
	trace_printf("%s = %hd;\n", name, *vp);
}


void
trace_short_unsigned_real(name, vp)
	char		*name;
	unsigned short	*vp;
{
	trace_printf("%s = %hu;\n", name, *vp);
}


void
trace_string_real(name, vp)
	char		*name;
	char		*vp;
{
	char		*s;
	long		count;

	trace_printf("%s = ", name);
	if (!vp)
	{
		trace_printf("NULL;\n");
		return;
	}
	trace_printf("\"");
	count = 0;
	for (s = vp; *s; ++s)
	{
		switch (*s)
		{
		case '('/*)*/:
		case '['/*]*/:
		case '{'/*}*/:
			++count;
			break;

		case /*(*/')':
		case /*[*/']':
		case /*{*/'}':
			--count;
			break;
		}
	}
	if (count > 0)
		count = -count;
	else
		count = 0;
	for (s = vp; *s; ++s)
	{
		int	c;

		c = *s;
		if (c < ' ' || c > '~')
		{
			char	*cp;

			cp = strchr("\bb\ff\nn\rr\tt", c);
			if (cp)
				trace_printf("\\%c", cp[1]);
			else
			{
				escape:
				trace_printf("\\%03o", (unsigned char)c);
			}
		}
		else
		{
			switch (c)
			{
			case '('/*)*/:
			case '['/*]*/:
			case '{'/*}*/:
				++count;
				if (count <= 0)
					goto escape;
				break;
	
			case /*(*/')':
			case /*[*/']':
			case /*{*/'}':
				--count;
				if (count < 0)
					goto escape;
				break;

			case '\\':
			case '"':
				trace_printf("\\");
				break;
			}
			trace_printf("%c", c);
		}
	}
	trace_printf("\";\n");
}


void
trace_indent_reset()
{
	depth = 0;
}
