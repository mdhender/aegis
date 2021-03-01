/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1997, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions for columnized output
 */

#include <ac/ctype.h>
#include <ac/limits.h>
#include <ac/stdarg.h>
#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/wchar.h>
#include <ac/wctype.h>

#include <col.h>
#include <error.h>
#include <fopen_nfs.h>
#include <glue.h>
#include <language.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <pager.h>
#include <sub.h>
#include <trace.h>
#include <wstr.h>


#define PRINTER_THRESHOLD 33


typedef struct col_ty col_ty;
struct col_ty
{
	int		icol;
	int		ocol;
	size_t		text_length_max;
	size_t		text_length;
	wchar_t		*text;
	wchar_t		*current;
	int		min;
	int		max;
	wstring_ty	*heading;
	int		heading_required;
	wchar_t		*top_of_page_diverted;
};

static	size_t		ncols;
static	size_t		ncols_max;
static	col_ty		**col;
static	char		*filename;
static	FILE		*fp;
static	char		pager[] = "PAGER";
static	long		page_number;
static	long		page_line;
static	int		top_of_page;
static	int		page_width;
static	int		page_length;
static	int		is_a_printer;
static	wstring_ty	*title1;
static	wstring_ty	*title2;
static	time_t		page_time;
static	int		unf;
static	int		tab_width;


/*
 * NAME
 *	col_open
 *
 * SYNOPSIS
 *	void col_open(char *pathname);
 *
 * DESCRIPTION
 *	The col_open function is used to
 *	open a file for outputting columnar data.
 *
 * ARGUMENTS
 *	pathname	- name of file to write,
 *			pager is used if NULL pointer given.
 *
 * CAVEAT
 *	All other calls to col_ functions must be bracketed
 *	by col_open and col_close calls.
 */

void
col_open(s)
	char		*s;
{
	trace(("col_open(s = %08lX)\n{\n"/*}*/, s));
	os_become_must_not_be_active();
	trace_string(s);
	assert(!filename);
	assert(!fp);
	assert(!ncols);
	if (s)
	{
		filename = mem_copy_string(s);
		os_become_orig();
		fp = fopen_with_stale_nfs_retry(filename, "w");
		if (!fp)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set(scp, "File_Name", "%s", filename);
			fatal_intl(scp, i18n("open $filename: $errno"));
			/* NOTREACHED */
		}
		os_become_undo();
	}
	else
	{
		fp = pager_open();
		filename = pager;
	}
	page_number = 0;
	page_line = 0;
	top_of_page = 1;
	/* don't use the last column, many terminals are dumb */
	page_width = option_page_width_get() - 1;
	page_length = option_page_length_get();
	is_a_printer = (page_length > PRINTER_THRESHOLD);
	if (is_a_printer)
	{
		/* bottom margin, avoid the perforation */
		page_length -= 3;
	}
	else
	{
		/* leave the last line for the pager */
		page_length--;
	}
	time(&page_time);
	unf = option_unformatted_get();
	tab_width = option_tab_width_get();
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	col_error
 *
 * SYNOPSIS
 *	void col_error(void);
 *
 * DESCRIPTION
 *	The col_error function is used to
 *	report errors when they occur.
 *
 *	The error will either be on a file with a known name,
 *	or should be reported via the pager subsystem.
 */

static void col_error _((void));

static void
col_error()
{
	if (filename == pager)
		pager_error(fp);
	else
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", filename);
		fatal_intl(scp, i18n("write $filename: $errno"));
		/* NOTREACHED */
	}
}


/*
 * NAME
 *	col_close
 *
 * SYNOPSIS
 *	void col_close(void);
 *
 * DESCRIPTION
 *	The col_close function is used to
 *	terminate columnar output.
 *
 *	All dynamic memory consumed will be released.
 *
 * CAVEAT
 *	All other calls to col_ functions must be bracketed
 *	by col_open and col_close calls.
 */

void
col_close()
{
	int		j;
	col_ty		*cp;

	/*
	 * free the memory consumed by the columns
	 */
	trace(("col_close()\n{\n"/*}*/));
	if (ncols)
	{
		for (j = 0; j < ncols; ++j)
		{
			cp = col[j];

			if (cp->text_length_max)
				mem_free(cp->text);
			if (cp->heading)
				wstr_free(cp->heading);
		}
		mem_free((char *)col);
		ncols = 0;
		col = 0;
	}
	if (title1)
	{
		wstr_free(title1);
		title1 = 0;
	}
	if (title2)
	{
		wstr_free(title2);
		title2 = 0;
	}

	/*
	 * write the last of the output
	 */
	if (filename != pager)
	{
		if (glue_fflush(fp))
			col_error();
	}
	else
	{
		if (fflush(fp))
			col_error();
	}

	/*
	 * close the output
	 */
	if (filename != pager)
	{
		if (glue_fclose(fp))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set(scp, "File_Name", "%s", filename);
			fatal_intl(scp, i18n("close $filename: $errno"));
			/* NOTREACHED */
		}
		mem_free(filename);
	}
	else
		pager_close(fp);
	fp = 0;
	filename = 0;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	col_create
 *
 * SYNOPSIS
 *	void col_create(void);
 *
 * DESCRIPTION
 *	The col_create function is used to
 *	specify a range of locations for an output column.
 *
 * ARGUMENTS
 *	min	- the left-hand edge of the column
 *	max	- the right-hand edge of the column, plus one
 *		  zero means the rest of the line
 *
 * RETURNS
 *	int; a small non-negative integer, as a unique column identifier
 */

int 
col_create(min, max)
	int		min;
	int		max;
{
	col_ty		*cp;

	trace(("col_create(min = %d, max = %d)\n{\n"/*}*/, min, max));
	if (!max)
	{
		max = page_width;
		if (max <= min)
			max = min + 8;
	}

	/*
	 * try to get sensable behaviour out of narrow windows
	 */
	if (max > page_width)
		page_width = max;

	assert(min < max);
	cp = (col_ty *)mem_alloc(sizeof(col_ty));
	if (ncols >= ncols_max)
	{
		size_t		nbytes;

		ncols_max = ncols_max * 2 + 4;
		nbytes = ncols_max * sizeof(col_ty *);
		col = mem_change_size(col, nbytes);
	}
	col[ncols++] = cp;

	cp->min = min;
	cp->max = max;
	cp->text_length_max = 0;
	cp->text_length = 0;
	cp->text = 0;
	cp->icol = 0;
	cp->ocol = 0;
	cp->heading = 0;
	cp->heading_required = 0;
	cp->top_of_page_diverted = 0;
	trace(("return %d;\n", ncols - 1));
	trace((/*{*/"}\n"));
	return (ncols - 1);
}


/*
 * NAME
 *	col_save_char
 *
 * SYNOPSIS
 *	void col_save_char(col_ty *cp, wint_t c);
 *
 * DESCRIPTION
 *	The col_save_char function is used to
 *	append a character to the output buffered for a specified column.
 *
 * ARGUMENTS
 *	cp	- pointer to the column
 *	c	- the character to append
 */

static void col_save_wchar _((col_ty *, wint_t));

static void
col_save_wchar(cp, c)
	col_ty		*cp;
	wint_t		c;
{
	size_t		nbytes;

	if (cp->text_length >= cp->text_length_max)
	{
		/*
		 * always alloc one too large
		 * so col_eoln can put NUL on the end.
		 */
		cp->text_length_max = cp->text_length_max * 2 + 16;
		nbytes = (cp->text_length_max + 1) * sizeof(wchar_t);
		cp->text = mem_change_size(cp->text, nbytes);
	}
	cp->text[cp->text_length++] = c;
}


static void col_save_xchar _((col_ty *, int));

static void
col_save_xchar(cp, n)
	col_ty		*cp;
	int		n;
{
	col_save_wchar(cp, (wchar_t)("0123456789ABCDEF"[n & 15]));
}


/*
 * NAME
 *	col_put_wchar
 *
 * SYNOPSIS
 *	void col_put_wchar(col_ty *cp, wint_t c);
 *
 * DESCRIPTION
 *	The col_put_wchar function is used to
 *	append a character to the buffer stored for a specified column.
 *
 *	This function filters tab characters in order to expand them,
 *	and preserve the visual nature of 8-character spaces within a column,
 *	irrespective of the actual screen location of the column when output.
 *
 * ARGUMENTS
 *	cp	- pointer to the column
 *	c	- the character to append
 */

static void col_put_wchar _((col_ty *, wint_t));

static void
col_put_wchar(cp, c)
	col_ty		*cp;
	wint_t		c;
{
	c = (wchar_t)c;
	switch (c)
	{ 
	case ' ':
		col_save_wchar(cp, (wchar_t)' ');
		cp->icol++;
		break;

	case '\t':
		/*
		 * Internally, treat tabs as 8 characters wide.
		 */
		for (;;)
		{
			col_save_wchar(cp, (wchar_t)' ');
			cp->icol++;
			if (!(cp->icol & 7))
				break;
		}
		break;

	case '\n':
		col_save_wchar(cp, (wchar_t)'\n');
		cp->icol = 0;
		break;

	default:
		if (iswprint(c))
		{
			col_save_wchar(cp, c);
			cp->icol++;
		}
		else
		{
			/* Actually, it could be MUCH longer than this. */
			col_save_wchar(cp, (wchar_t)'\\');
			col_save_wchar(cp, (wchar_t)'x');
			col_save_xchar(cp, c >> 12);
			col_save_xchar(cp, c >>  8);
			col_save_xchar(cp, c >>  4);
			col_save_xchar(cp, c      );
			cp->icol += 4;
		}
		break;
	}
}


/*
 * NAME
 *	col_puts
 *
 * SYNOPSIS
 *	void col_puts(int cid, char *s);
 *
 * DESCRIPTION
 *	The col_puts function is used to append strings
 *	to columns for later output.
 *
 * ARGUMENTS
 *	cid	- column identifier
 *	s	- string to be appended
 *
 * CAVEAT
 *	This is the external user interface for placing characters in a column.
 */

static void col_wputs _((col_ty *cp, wchar_t *));

static void
col_wputs(cp, ws)
	col_ty		*cp;
	wchar_t		*ws;
{
	trace(("col_wputs(cp = %08lX, ws = %08lX)\n{\n"/*}*/, (long)cp, (long)ws));
	while (*ws)
		col_put_wchar(cp, *ws++);
	trace((/*{*/"}\n"));
}


void
col_puts(cid, s)
	int		cid;
	char		*s;
{
	col_ty		*cp;
	wstring_ty	*ws;

	trace(("col_puts(cid = %d, s = %08lX)\n{\n"/*}*/, cid, s));
	assert(cid >= 0 && cid < ncols);
	cp = col[cid];

	/*
	 * Convert the C string into a wide C string.
	 * The wstr_from_c function manages the locale,
	 * so there is no need to select one here.
	 */
	ws = wstr_from_c(s);

	col_wputs(cp, ws->wstr_text);
	wstr_free(ws);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	col_printf
 *
 * SYNOPSIS
 *	void col_printf(int cid, char *format, ...);
 *
 * DESCRIPTION
 *	The col_printf function is used to
 *	format strings for appending to columns for later output.
 *
 *	The format is that used by printf et al.
 *
 * ARGUMENTS
 *	cid	- column identifier
 *	format	- format string, a la printf
 *	...	- additional arguments as required by the format
 *
 * CAVEAT
 *	This is the external user interface for placing characters in a column.
 */

void
col_printf(cid, s sva_last)
	int		cid;
	char		*s;
	sva_last_decl
{
	va_list		ap;
	string_ty	*tmp;

	trace(("col_printf(cid = %d, s = %08lX, ...)\n{\n"/*}*/, cid, s));
	sva_init(ap, s);
	trace_string(s);
	tmp = str_vformat(s, ap);
	va_end(ap);
	col_puts(cid, tmp->str_text);
	str_free(tmp);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	top_of_page_mark
 *
 * SYNOPSIS
 *	void top_of_page_mark(void);
 *
 * DESCRIPTION
 *	The top_of_page_mark function is used to
 *	mark that the top of page has occurred,
 *	and that top-of-page processing should be performed
 *	when next there is output.
 */

static void top_of_page_mark _((void));

static void
top_of_page_mark()
{
	int		j;
	col_ty		*cp;

	top_of_page = 1;
	for (j = 0; j < ncols; ++j)
	{
		cp = col[j];
		if (cp->heading)
			cp->heading_required = 1;
	}
}


#define INDENT 8

static int	in_col;
static int	out_col;

static void top_of_page_processing _((void)); /* forward */


static void col_emit_wchar_inner2 _((wint_t));

static void
col_emit_wchar_inner2(c)
	wint_t		c;
{
	int		nbytes;
	int		j;
	char		dummy[MB_LEN_MAX];

	/*
	 * Keep printing characters.  Use a dummy character for
	 * unprintable sequences (which should not happen, because they
	 * were filtered out already).
	 */
	if (c && !iswspace(c) && !iswprint(c))
		c = '?';

	/*
	 * Turn the wide character into a multi-byte character
	 * string.
	 */
	nbytes = wctomb(dummy, c);
	if (c ? (nbytes <= 0) : (nbytes < 0))
	{
		/*
		 * This should not happen!  All unprintable characters
		 * should have been turned into C escapes inside the
		 * common/wstr.c file when converting from C string to
		 * wide strings.
		 */
		dummy[0] = '?';
		nbytes = 1;

		/*
		 * The wctomb state will be ``error'', so reset it and
		 * brave the worst.
		 */
		wctomb(NULL, 0);
	}

	/*
	 * Emit the multi-byte sequence.
	 */
	for (j = 0; j < nbytes; ++j)
	{
		int	nc;

		nc = dummy[j];
		if (!nc)
			continue;
		if (filename != pager)
			glue_fputc(nc, fp);
		else
			putc(nc, fp);
	}
}


static void col_emit_wchar_inner _((wint_t));

static void
col_emit_wchar_inner(c)
	wint_t		c;
{
	static wchar_t	*buffer;
	static size_t	buf_pos;
	static size_t	buf_max;
	wchar_t		*cp;
	wchar_t		*ep;

	/*
	 * For all characters but end-of-line, buffer the character.
	 * This is because we need to convert the line from wide
	 * characters to multi-byte characters in one pass, otherwise
	 * we screw up the conversion state for other conversions (or
	 * they screw us up).
	 */
	if (c != '\n')
	{
		if (buf_pos >= buf_max)
		{
			size_t	nbytes;

			buf_max = buf_max * 2 + 32;
			nbytes = sizeof(wchar_t) * buf_max;
			buffer = mem_change_size(buffer, nbytes);
		}
		buffer[buf_pos++] = c;
		return;
	}
	assert(fp);

	/*
	 * reset conversion status
	 */
	trace(("col.emit.newline\n{\n"));
	language_human();
	wctomb(NULL, 0);

	/*
	 * iterate over the string
	 */
	cp = buffer;
	ep = buffer + buf_pos;
	buf_pos = 0;
	while (cp < ep)
		col_emit_wchar_inner2(*cp++);

	/*
	 * End the line.
	 */
	col_emit_wchar_inner2((wchar_t)'\n');

	/*
	 * End any outstanding shift state.
	 */
	col_emit_wchar_inner2((wchar_t)0);
	language_C();

	/*
	 * Check for errors once per line.
	 */
	if (filename != pager)
	{
#ifdef DEBUG
		glue_fflush(fp);
#endif
		if (glue_ferror(fp))
			col_error();
	}
	else
	{
#ifdef DEBUG
		fflush(fp);
#endif
		if (ferror(fp))
			col_error();
	}
	trace(("}\n"));
}


/*
 * NAME
 *	col_emit_wchar
 *
 * SYNOPSIS
 *	void col_emit_wchar(wint_t c);
 *
 * DESCRIPTION
 *	The col_emit_char function is used to
 *	emit on character to the output file or pager.
 *
 *	White space optimization is performed, raplacing
 *	sequences of blanks with horizontal tabs whenever possible.
 *
 *	This function is also responsible for tracking when end-of-page
 *	is reached and notifying that top-of-page processing is required.
 *
 * ARGUMENTS
 *	c	- character to emit
 */

static void col_emit_wchar _((wint_t));

static void
col_emit_wchar(c)
	wint_t		c;
{
	trace(("col_emit_wchar(c = %d)\n{\n", (int)c));
	assert(fp);
	if (top_of_page)
		top_of_page_processing();
	switch (c)
	{
	case '\n':
		col_emit_wchar_inner('\n');
		in_col = 0;
		out_col = 0;
		page_line++;
		if (page_line >= page_length)
			top_of_page_mark();
		break;

	case ' ':
		++in_col;
		break;

	case '\t':
		/*
		 * Internally, treat tabs as 8 characters wide.
		 * (This case should never be exersized.)
		 */
		in_col = (in_col + 8) & ~7;
		break;

	default:
		if (!unf && tab_width)
		{
			while
			(
				out_col + 1 < in_col
			&&
				((out_col / tab_width + 1) * tab_width) <= in_col
			)
			{
				col_emit_wchar_inner((wchar_t)'\t');
				out_col = (out_col / tab_width + 1) * tab_width;
			}
		}
		while (out_col < in_col)
		{
			col_emit_wchar_inner((wchar_t)' ');
			++out_col;
		}
		col_emit_wchar_inner(c);
		in_col += wcwidth(c);
		out_col = in_col;
		break;
	}
	trace(("}\n"));
}


/*
 * NAME
 *	col_emit_str
 *
 * SYNOPSIS
 *	void col_emit_str(char *s);
 *
 * DESCRIPTION
 *	The col_emit_str function is used to
 *	send a string through the col_emit_char function,
 *	one character at a time.
 *
 * ARGUMENTS
 *	s	- pointer to NUL terminated string
 *		  to be sent.
 */

static void col_emit_wstr _((wstring_ty *));

static void
col_emit_wstr(ws)
	wstring_ty	*ws;
{
	wchar_t		*wcp;

	wcp = ws->wstr_text;
	while (*wcp)
		col_emit_wchar(*wcp++);
}


/*
 * NAME
 *	col_eoln_sub
 *
 * SYNOPSIS
 *	void col_eoln_sub(void);
 *
 * DESCRIPTION
 *	The col_eoln_sub function is used to
 *	send the buffered contents of the columns to the
 *	output file or pager.
 *
 *	Columns are wrapped as needed, and thus may emit
 *	more than one line of output.
 *
 *	This function is also used to emit the column headers.
 */

static void col_eoln_sub _((void));

static void
col_eoln_sub()
{
	int		more;
	int		j;
	col_ty		*cp;

	trace(("col_eoln_sub()\n{\n"/*}*/));
	for (;;)
	{
		more = 0;
		for (j = 0; j < ncols; ++j)
		{
			int	width;
			wchar_t	*s;
			int	s_wid;
			int	c_wid;

			trace(("mark j=%d\n", j));
			cp = col[j];
			if (!cp->current)
				continue;
			if (in_col > cp->min)
			{
				more = 1;
				break;
			}
			trace(("mark\n"));
			while (in_col < cp->min)
				col_emit_wchar((wchar_t)' ');
			width = cp->max - cp->min;
			s = cp->current;
			s_wid = 0;
			if (*s && *s != '\n')
			{
				/*
				 * Always use the first character.
				 * This avoids an infinite loop where
				 * you have a 1-position wide column,
				 * and a 2-position wide character.
				 * (Or other variations on the same theme.)
				 */
				s_wid += wcwidth(*s++);
			}
			while (*s && *s != '\n')
			{
				c_wid = s_wid + wcwidth(*s);
				if (c_wid > width)
					break;
				++s;
				s_wid = c_wid;
			}
			trace(("mark s_wid=%d width=%d\n", s_wid, width));
			if (!*s)
			{
				trace(("mark cp->current=%08lX s=%08lX\n", (long)cp->current, (long)s));
				while (cp->current < s)
					col_emit_wchar(*cp->current++);
				cp->current = 0;
				trace(("mark\n"));
				continue;
			}
			trace(("mark\n"));
			if (*s == '\n')
			{
				while (cp->current < s)
					col_emit_wchar(*cp->current++);
				cp->current++;
				for (s = cp->current; *s && iswspace(*s); ++s)
					;
				if (!*s)
				{
					trace(("mark\n"));
					cp->current = 0;
					continue;
				}
				for
				(
					s = cp->current;
					*s && *s != '\n' && iswspace(*s);
					++s
				)
					;
				more = 1;
				cp->current = s;
				trace(("mark\n"));
				continue;
			}

			/*
			 * the line must be wrapped
			 */
			trace(("mark\n"));
			if (*s && !iswspace(*s))
			{
				wchar_t		*s2;

				s2 = s;
				while
				(
					s2 > cp->current
				&&
					!iswspace(s2[-1])
				&&
					s2[-1] != (wchar_t)'-'
				&&
					s2[-1] != (wchar_t)'_'
				&&
					s2[-1] != (wchar_t)'/'
				)
					--s2;
				if (s2 <= cp->current)
				{
					/*
					 * no nice place to break it
					 */
					while (cp->current < s)
						col_emit_wchar(*cp->current++);
					more = 1;
					trace(("mark\n"));
					continue;
				}
				s = s2;
			}

			trace(("mark\n"));
			while (cp->current < s)
				col_emit_wchar(*cp->current++);
			while (iswspace(*cp->current))
				cp->current++;
			if (*cp->current)
				more = 1;
			else
				cp->current = 0;
			trace(("mark\n"));
		}
		trace(("mark\n"));
		col_emit_wchar((wchar_t)'\n');
		if (!more)
			break;
	}
	trace((/*{*/"}\n"));
}


static wstring_ty *wstr_subset_of_max_width _((wstring_ty *, int));

static wstring_ty *
wstr_subset_of_max_width(ws, max)
	wstring_ty	*ws;
	int		max;
{
	wchar_t		*s;
	int		s_wid;
	int		c_wid;

	s = ws->wstr_text;
	s_wid = 0;
	while (*s)
	{
		c_wid = s_wid + wcwidth(*s);
		if (c_wid > max)
			break;
		++s;
		s_wid = c_wid;
	}
	return wstr_n_from_wc(ws->wstr_text, s - ws->wstr_text);
}


static wstring_ty *wstr_spaces(int);

static wstring_ty *
wstr_spaces(n)
	int		n;
{
	wchar_t		*buffer;
	wstring_ty	*result;
	int		j;

	trace(("mark\n"));
	if (n <= 0)
		return wstr_from_c("");
	trace(("mark\n"));
	buffer = mem_alloc(n * sizeof(wchar_t));
	trace(("mark\n"));
	for (j = 0; j < n; ++j)
		buffer[j] = ' ';
	trace(("mark\n"));
	result = wstr_n_from_wc(buffer, n);
	trace(("mark\n"));
	mem_free(buffer);
	trace(("mark\n"));
	return result;
}


static void left_and_right _((wstring_ty *, char *));

static void
left_and_right(lhs, rhs)
	wstring_ty	*lhs;
	char		*rhs;
{
	wstring_ty	*tmp1;
	wstring_ty	*tmp2;
	wstring_ty	*tmp3;
	int		w3;
	int		frac;

	trace(("left_and_right()\n{\n"));
	tmp3 = wstr_from_c(rhs);
	trace(("mark\n"));
	w3 = wstr_column_width(tmp3);
	frac = page_width - 2 - w3;
	trace(("mark\n"));
	tmp1 = wstr_subset_of_max_width(lhs, frac);
	trace(("mark\n"));
	frac = page_width - wstr_column_width(tmp1) - w3;
	trace(("mark frac=%d\n", frac));
	tmp2 = wstr_spaces(frac);
	trace(("mark\n"));
	col_emit_wstr(tmp1);
	col_emit_wstr(tmp2);
	col_emit_wstr(tmp3);
	col_emit_wchar((wchar_t)'\n');
	wstr_free(tmp1);
	wstr_free(tmp2);
	wstr_free(tmp3);
	trace(("}\n"));
}


/*
 * NAME
 *	top_of_page_processing
 *
 * SYNOPSIS
 *	void top_of_page_processing(void);
 *
 * DESCRIPTION
 *	The top_of_page_processing function is used to
 *	emit the various titles and headings required at the top of a page.
 *
 * ARGUMENTS
 *
 * RETURNS
 */

static void
top_of_page_processing()
{
	int		heading_required;
	int		j;
	col_ty		*cp;
	char		tmp1[50];

	/*
	 * setup
	 */
	top_of_page = 0;
	page_number++;
	page_line = 0;
	if (unf)
		return;

	/*
	 * seek to next page
	 * and emit top margin
	 */
	trace(("top_of_page_processing()\n{\n"));
	if (is_a_printer)
	{
		if (page_number > 1)
			col_emit_wchar((wchar_t)'\f');
		col_emit_wchar((wchar_t)'\n');
		col_emit_wchar((wchar_t)'\n');
	}
	col_emit_wchar((wchar_t)'\n');

	/*
	 * first line of titles
	 */
	trace(("mark\n"));
	sprintf(tmp1, "Page %ld", page_number);
	if (!title1)
		title1 = wstr_from_c("");
	left_and_right(title1, tmp1);

	/*
	 * second line of titles
	 */
	trace(("mark\n"));
	sprintf(tmp1, "%.24s", ctime(&page_time));
	if (!title2)
		title2 = wstr_from_c("");
	left_and_right(title2, tmp1);

	/*
	 * blank line between titles and columns
	 */
	trace(("mark\n"));
	col_emit_wchar((wchar_t)'\n');

	/*
	 * in weird cases where the terminal is too narrow,
	 * the headings can try to span pages.
	 * If we are already top-of-page diverted, don't do it again.
	 */
	trace(("mark\n"));
	heading_required = 0;
	for (j = 0; j < ncols; ++j)
	{
		if (col[j]->top_of_page_diverted)
			heading_required--;
	}
	if (heading_required < 0)
	{
		for (j = 0; j < ncols; ++j)
			col[j]->heading_required = 0;
		trace(("}\n"));
		return;
	}

	/*
	 * divert for headings
	 */
	trace(("mark\n"));
	heading_required = 0;
	for (j = 0; j < ncols; ++j)
	{
		cp = col[j];
		assert(!cp->top_of_page_diverted);
		cp->top_of_page_diverted = cp->current;
		if (cp->heading && cp->heading_required)
		{
			heading_required++;
			cp->current = cp->heading->wstr_text;
		}
		else
			cp->current = 0;
		cp->heading_required = 0;
	}

	/*
	 * do the headings as necessary
	 */
	trace(("mark\n"));
	if (heading_required)
		col_eoln_sub();

	/*
	 * undivert from headings
	 */
	trace(("mark\n"));
	for (j = 0; j < ncols; ++j)
	{
		cp = col[j];
		cp->current = cp->top_of_page_diverted;
		cp->heading_required = 0;
		cp->top_of_page_diverted = 0;
	}
	trace(("}\n"));
}


/*
 * NAME
 *	col_eoln
 *
 * SYNOPSIS
 *	void col_eoln(void);
 *
 * DESCRIPTION
 *	The col_eoln function is used to
 *	signal that an entire line of columns has been assembled
 *	and that it should be emitted to the output file or pager.
 *
 *	This is the interface used by clients of this subsystem.
 */

void
col_eoln()
{
	int		j;
	col_ty		*cp;
	int		heading_required;

	/*
	 * terminate text fields
	 */
	trace(("col_eoln()\n{\n"/*}*/));
	for (j = 0; j < ncols; ++j)
	{
		cp = col[j];
		if (cp->text_length && cp->text)
			cp->text[cp->text_length] = 0;
	}

	/*
	 * the unformatted variant emits each column
	 * with a single space between.
	 * Width specifications will be ignored.
	 * This is for the convenience of shell scripts.
	 *
	 * If a column is wider than page_width characters, it will be trimmed.
	 * If a column spans several lines, only the first will be printed.
	 */
	if (unf)
	{
		for (j = 0; j < ncols; ++j)
		{
			wchar_t	*ep;
			wchar_t	*sp;
			int	s_wid;
			int	c_wid;

			trace(("mark j=%d\n", j));
			cp = col[j];
			if (!cp->text || !cp->text_length)
				continue;
			sp = cp->text;
			while (*sp && iswspace(*sp))
				++sp;
			ep = sp;
			s_wid = 0;
			while (*ep && *ep != '\n')
			{
				c_wid = s_wid + wcwidth(*ep);
				if (c_wid > page_width)
					break;
				++ep;
				s_wid = c_wid;
			}
			while (ep > sp && iswspace(ep[-1]))
				--ep;
			if (ep <= sp)
				continue;
			if (in_col)
				col_emit_wchar((wchar_t)' ');
			while (sp < ep)
				col_emit_wchar(*sp++);
		}
		trace(("mark\n"));
		col_emit_wchar((wchar_t)'\n');
		goto cleanup;
	}

	/*
	 * do headings if required
	 *
	 * Top of page is not enough,
	 * there could be mid-page changes of heading.
	 */
	trace(("mark\n"));
	heading_required = 0;
	for (j = 0; j < ncols; ++j)
	{
		cp = col[j];
		if (cp->heading && cp->heading_required)
		{
			heading_required++;
			cp->current = cp->heading->wstr_text;
		}
		else
			cp->current = 0;
		cp->heading_required = 0;
	}
	if (heading_required)
		col_eoln_sub();

	/*
	 * do the text body
	 */
	trace(("mark\n"));
	for (j = 0; j < ncols; ++j)
	{
		cp = col[j];
		if (cp->text_length)
			cp->current = cp->text;
		else
			cp->current = 0;
	}
	col_eoln_sub();

	/*
	 * clean up for next time
	 */
	cleanup:
	trace(("mark\n"));
	for (j = 0; j < ncols; ++j)
	{
		cp = col[j];
		cp->text_length = 0;
		cp->icol = 0;
		cp->ocol = 0;
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	col_bol
 *
 * SYNOPSIS
 *	void col_bol(int cid);
 *
 * DESCRIPTION
 *	The col_bol function is used to
 *	ensure that the buffer accumulating a specified column
 *	is at the beginning of a line.
 *
 * ARGUMENTS
 *	cid	- column identifier
 */

void
col_bol(cid)
	int		cid;
{
	col_ty		*cp;

	trace(("col_bol(cid = %d)\n{\n"/*}*/, cid));
	assert(cid >= 0 && cid < ncols);
	cp = col[cid];
	if (cp->icol)
		col_put_wchar(cp, (wchar_t)'\n');
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	col_heading
 *
 * SYNOPSIS
 *	void col_heading(int cid, char *s);
 *
 * DESCRIPTION
 *	The col_heading function is used to
 *	set the heading of a specified column
 *
 * ARGUMENTS
 *	cid	- column identifier
 *	s	- heading to be set
 */

void
col_heading(cid, s)
	int		cid;
	char		*s;
{
	col_ty		*cp;

	trace(("col_heading(cid = %d, s = %08lX)\n{\n"/*}*/, cid, s));
	assert(cid >= 0 && cid < ncols);
	cp = col[cid];
	if (cp->heading)
		wstr_free(cp->heading);
	if (s && *s)
	{
		/*
		 * The heading_required flag is used
		 * when a heading is created in the middle of a page,
		 * and should be output at that time.
		 *
		 * Convert the C string into a wide C string.  The
		 * wstr_from_c function manages the locale, so there is
		 * no need to select one here.
		 */
		cp->heading = wstr_from_c(s);
		cp->heading_required = 1;
	}
	else
	{
		cp->heading = 0;
		cp->heading_required = 0;
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	col_title
 *
 * SYNOPSIS
 *	void col_title(char *s1, char *s2);
 *
 * DESCRIPTION
 *	The col_title function is used to
 *	set the two lines of page headings.
 *
 * ARGUMENTS
 *	s1	- first line of heading, NULL means blank
 *	s2	- second line of heading, NULL means blank
 */

void
col_title(s1, s2)
	char		*s1;
	char		*s2;
{
	trace(("col_title(s1 = %08lX, s2 = %08lX)\n{\n"/*}*/, s1, s2));
	if (!s1)
		s1 = "";
	trace_string(s1);
	if (title1)
		wstr_free(title1);
	title1 = wstr_from_c(s1);

	if (!s2)
		s2 = "";
	trace_string(s2);
	if (title2)
		wstr_free(title2);
	title2 = wstr_from_c(s2);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	col_eject
 *
 * SYNOPSIS
 *	void col_eject(void);
 *
 * DESCRIPTION
 *	The col_eject function is used to move to the top of the next page,
 *	if we aren't there already.
 */

void
col_eject()
{
	trace(("col_eject()\n{\n"/*}*/));
	if (!top_of_page)
	{
		if (is_a_printer)
			top_of_page_mark();
		else
		{
			for (;;)
			{
				col_emit_wchar((wchar_t)'\n');
				if (top_of_page)
					break;
			}
		}
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	col_need
 *
 * SYNOPSIS
 *	void col_need(int n);
 *
 * DESCRIPTION
 *	The col_need function is used to numinate that
 *	a number of lines is needed before the end of the page.
 *	If the lines are available, a blank line is emitted,
 *	otherwise a new page is thrown.
 *
 * ARGUMENTS
 *	n	- the number of lines,
 *		  not counting the blank line.
 */

void
col_need(n)
	int		n;
{
	trace(("col_need(n = %d)\n{\n"/*}*/, n));
	assert(n < page_length);
	if (page_line + n >= page_length)
		col_eject();
	else
		if (!top_of_page)
			col_eoln();
	trace((/*{*/"}\n"));
}
