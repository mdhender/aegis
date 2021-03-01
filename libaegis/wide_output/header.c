/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999-2001 Peter Miller;
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
 * MANIFEST: functions to manipulate headers
 *
 * This class is used to add page headers to the output.
 */

#include <ac/stdio.h> /* for sprintf */
#include <ac/time.h>
#include <ac/wchar.h>
#include <ac/wctype.h>

#include <language.h>
#include <mem.h>
#include <option.h>
#include <str.h>
#include <trace.h>
#include <wide_output.h>
#include <wide_output/header.h>
#include <wide_output/private.h>
#include <wstr.h>

/*
 * This is how you figure out whether you are talking to a printer,
 * or to a terminal.
 */
#define PRINTER_THRESHOLD (DEFAULT_PRINTER_LENGTH/2)
/* assert(PRINTER_THRESHOLD > DEFAULT_PAGE_LENGTH); */


typedef struct wide_output_header_ty wide_output_header_ty;
struct wide_output_header_ty
{
	wide_output_ty	inherited;
	wide_output_ty	*deeper;
	int		delete_on_close;
	wstring_ty	*title1;
	wstring_ty	*title2;
	int		is_a_printer;
	int		line_number;
	int		length;
	int		width;
	int		column;
	int		page_number;
	time_t		page_time;
	int		already_top_diverted;
};


static void wide_output_header_destructor _((wide_output_ty *));

static void
wide_output_header_destructor(fp)
	wide_output_ty	*fp;
{
	wide_output_header_ty *this;

	trace(("wide_output_header::destructor(fp = %08lX)\n{\n", (long)fp));
	/* eoln? */
	/* bottom of page processing? */
	this = (wide_output_header_ty *)fp;
	if (this->delete_on_close)
		wide_output_delete(this->deeper);
	this->deeper = 0;
	if (this->title1)
	{
		wstr_free(this->title1);
		this->title1 = 0;
	}
	if (this->title2)
	{
		wstr_free(this->title2);
		this->title2 = 0;
	}
	trace(("}\n"));
}


static string_ty *wide_output_header_filename _((wide_output_ty *));

static string_ty *
wide_output_header_filename(fp)
	wide_output_ty	*fp;
{
	wide_output_header_ty *this;

	this = (wide_output_header_ty *)fp;
	return wide_output_filename(this->deeper);
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


static wstring_ty *wstr_spaces _((int));

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


static void left_and_right _((wide_output_header_ty *, wstring_ty *, const char *));

static void
left_and_right(this, lhs, rhs)
	wide_output_header_ty *this;
	wstring_ty	*lhs;
	const char	*rhs;
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
	frac = this->width - 2 - w3;
	trace(("mark\n"));
	tmp1 = wstr_subset_of_max_width(lhs, frac);
	trace(("mark\n"));
	frac = this->width - wstr_column_width(tmp1) - w3;
	trace(("mark frac=%d\n", frac));
	tmp2 = wstr_spaces(frac);
	trace(("mark\n"));
	wide_output_write(this->deeper, tmp1->wstr_text, tmp1->wstr_length);
	wide_output_write(this->deeper, tmp2->wstr_text, tmp2->wstr_length);
	wide_output_write(this->deeper, tmp3->wstr_text, tmp3->wstr_length);
	wide_output_putwc(this->deeper, (wchar_t)'\n');
	wstr_free(tmp1);
	wstr_free(tmp2);
	wstr_free(tmp3);
	trace(("}\n"));
}


static void top_of_page_processing _((wide_output_header_ty *));

static void
top_of_page_processing(this)
	wide_output_header_ty *this;
{
	char		tmp1[30];

	/*
	 * advance the page number
	 */
	if (this->already_top_diverted)
		return;
	this->already_top_diverted = 1;
	trace(("wide_output_header::top_of_page(this = %08lX)\n{\n", (long)this));
	this->page_number++;

	/*
	 * Blank line(s) before the title
	 */
	if (this->is_a_printer)
	{
		if (this->page_number > 1)
			wide_output_putwc(this->deeper, (wchar_t)'\f');
		wide_output_putwc(this->deeper, (wchar_t)'\n');
		wide_output_putwc(this->deeper, (wchar_t)'\n');
	}
	wide_output_putwc(this->deeper, (wchar_t)'\n');

	/*
	 * first line of titles
	 */
	trace(("mark\n"));
	sprintf(tmp1, "Page %d", this->page_number);
	if (!this->title1)
		this->title1 = wstr_from_c("");
	left_and_right(this, this->title1, tmp1);

	/*
	 * second line of titles
	 */
	trace(("mark\n"));
	sprintf(tmp1, "%.24s", ctime(&this->page_time));
	if (!this->title2)
		this->title2 = wstr_from_c("");
	left_and_right(this, this->title2, tmp1);

	/*
	 * blank line between titles and columns
	 */
	trace(("mark\n"));
	wide_output_putwc(this->deeper, (wchar_t)'\n');

	this->already_top_diverted = 0;
	trace(("}\n"));
}


static void bottom_of_page_processing _((wide_output_header_ty *));

static void
bottom_of_page_processing(this)
	wide_output_header_ty *this;
{
	trace(("bottom of page\n"));

	/*
	 * Flush the output here.  This means that the last page will be
	 * available on the output immediately.  This can be important
	 * for reports and lists which take a long to to generate.
	 */
	wide_output_flush(this->deeper);
}


static void wide_output_header_write _((wide_output_ty *, const wchar_t *,
	size_t));

static void
wide_output_header_write(fp, data, len)
	wide_output_ty	*fp;
	const wchar_t	*data;
	size_t		len;
{
	wide_output_header_ty *this;

	/*
	 * If we see a form-feed, advance to the next page.
	 * Do this by inserting the right number of newlines.
	 * By doing it first, by recursion, the rest of the unexceptional
	 * processing simply falls out cleanly.
	 */
	trace(("wide_output_header::write(fp = %08lX, data = %08lX, \
len = %ld)\n{\n", (long)fp, (long)data, (long)len));
	this = (wide_output_header_ty *)fp;
	while (len > 0)
	{
		wchar_t wc = *data++;
		--len;
	
		wc = (wchar_t)wc;
		if (wc == '\f')
		{
			/*
			 * If we are at the top of the page,
			 * emit the page header.
			 */
			if
			(
				this->line_number == 0
			&&
				this->column == 0
			&&
				!this->already_top_diverted
			)
				top_of_page_processing(this);
			for (;;)
			{
				/*
				 * Forward the character to the output.
				 * Keep track of the column for eoln()
				 * purposes; it doesn't need to be exact,
				 * so we aren't using wcwidth.
				 */
				wide_output_putwc(this->deeper, (wchar_t)'\n');
			
				/*
				 * Keep track of the line number so
				 * that we can work out when we get to
				 * the end of the page (and implicitly,
				 * the top of the next page).
				 */
				this->column = 0;
				this->line_number++;
				trace(("linum = %d;\n", this->line_number));
				if (this->line_number >= this->length)
				{
					bottom_of_page_processing(this);
					this->line_number = 0;
					break;
				}
			}
			continue;
		}
	
		/*
		 * If we are at the top of the page,
		 * emit the page header.
		 */
		if
		(
			this->line_number == 0
		&&
			this->column == 0
		&&
			!this->already_top_diverted
		)
			top_of_page_processing(this);
	
		/*
		 * Forward the character to the output.
		 * Keep track of the column for eoln() purposes;
		 * it doesn't need to be exact, so we aren't using wcwidth.
		 */
		wide_output_putwc(this->deeper, wc);
		this->column++;
	
		/*
		 * If the character was a newline, keep track of the line number
		 * so that we can work out when we get to the end of the page
		 * (and implicitly, the top of the next page).
		 */
		if (wc == '\n')
		{
			this->column = 0;
			this->line_number++;
			trace(("line_number = %d;\n", this->line_number));
			if (this->line_number >= this->length)
			{
				bottom_of_page_processing(this);
				this->line_number = 0;
			}
		}
	}
	trace(("}\n"));
}


static int wide_output_header_page_width _((wide_output_ty *));

static int
wide_output_header_page_width(fp)
	wide_output_ty	*fp;
{
	wide_output_header_ty *this;

	trace(("wide_output_header::width(fp = %08lX)\n{\n", (long)fp));
	this = (wide_output_header_ty *)fp;
	trace(("return %d;\n", this->width));
	trace(("}\n"));
	return this->width;
}


static void wide_output_header_flush _((wide_output_ty *));

static void
wide_output_header_flush(fp)
	wide_output_ty	*fp;
{
	wide_output_header_ty *this;

	trace(("wide_output_header::width(fp = %08lX)\n{\n", (long)fp));
	this = (wide_output_header_ty *)fp;
	wide_output_flush(this->deeper);
	trace(("}\n"));
}


static int wide_output_header_page_length _((wide_output_ty *));

static int
wide_output_header_page_length(fp)
	wide_output_ty	*fp;
{
	wide_output_header_ty *this;

	trace(("wide_output_header::length(fp = %08lX)\n{\n", (long)fp));
	this = (wide_output_header_ty *)fp;
	trace(("return %d;\n", this->length));
	trace(("}\n"));
	return this->length;
}


static void wide_output_header_eoln _((wide_output_ty *));

static void
wide_output_header_eoln(fp)
	wide_output_ty	*fp;
{
	wide_output_header_ty *this;

	trace(("wide_output_header::eoln(fp = %08lX)\n{\n", (long)fp));
	this = (wide_output_header_ty *)fp;
	if (this->column > 0)
		wide_output_putwc(fp, (wchar_t)'\n');
	trace(("}\n"));
}


static wide_output_vtbl_ty vtbl =
{
	sizeof(wide_output_header_ty),
	wide_output_header_destructor,
	wide_output_header_filename,
	wide_output_header_write,
	wide_output_header_flush,
	wide_output_header_page_width,
	wide_output_header_page_length,
	wide_output_header_eoln,
	"header",
};


wide_output_ty *
wide_output_header_open(deeper, delete_on_close)
	wide_output_ty	*deeper;
	int		delete_on_close;
{
	wide_output_ty	*result;
	wide_output_header_ty *this;

	trace(("wide_output_header::new(deeper = %08lX, doc = %d)\n{\n",
		(long)deeper, delete_on_close));
	result = wide_output_new(&vtbl);
	this = (wide_output_header_ty *)result;
	this->deeper = deeper;
	this->delete_on_close = delete_on_close;
	this->title1 = 0;
	this->title2 = 0;
	this->line_number = 0;
	this->width = wide_output_page_width(deeper);
	this->column = 0;
	this->page_number = 0;
	time(&this->page_time);
	this->already_top_diverted = 0;

	this->length = wide_output_page_length(deeper);
	this->is_a_printer = (this->length > PRINTER_THRESHOLD);
	if (this->is_a_printer)
	{
		/*
		 * Three blank lines at top (avoid perforation)
		 * two title lines,
		 * one blank line before columns.
		 * Plus, 3 blank lines at bottom (avoid perforation).
		 */
		this->length -= 9;
	}
	else
	{
		/*
		 * One blank line at top,
		 * two title lines,
		 * one blank line before columns.
		 * Leave the last line for the pager
		 */
		this->length -= 5;
	}
	trace(("return %08lX;\n", (long)result));
	trace(("}\n"));
	return result;
}


static wstring_ty *censor _((const char *));

static wstring_ty *
censor(s)
	const char	*s;
{
	wstring_ty	*ws;
	size_t		j;
	wstring_ty	*ws2;

	/*
	 * convert to a wide string
	 */
	ws = wstr_from_c(s);

	/*
	 * make sure it doesn't have any unreasonable characters
	 */
	language_human();
	for (j = 0; j < ws->wstr_length; ++j)
		if (!iswprint(ws->wstr_text[j]))
			break;
	language_C();

	/*
	 * Truncate at the first unprintable character,
	 * if any are present
	 */
	if (j < ws->wstr_length)
	{
		ws2 = wstr_n_from_wc(ws->wstr_text, j);
		wstr_free(ws);
		ws = ws2;
	}

	/*
	 * return result
	 */
	return ws;
}


void
wide_output_header_title(fp, title1, title2)
	wide_output_ty	*fp;
	const char	*title1;
	const char	*title2;
{
	wide_output_header_ty *this;

	if (fp->vptr != &vtbl)
		return;
	trace(("wide_output_header::title(fp = %08lX, \"%s\", \"%s\")\n{\n",
		(long)fp, (title1 ? title1 : ""), (title2 ? title2 : "")));
	this = (wide_output_header_ty *)fp;
	if (this->title1)
	{
		wstr_free(this->title1);
		this->title1 = 0;
	}
	if (title1)
		this->title1 = censor(title1);

	if (this->title2)
	{
		wstr_free(this->title2);
		this->title2 = 0;
	}
	if (title2)
		this->title2 = censor(title2);
	trace(("}\n"));
}


void
wide_output_header_need(fp, nlines)
	wide_output_ty	*fp;
	int		nlines;
{
	wide_output_header_ty *this;

	if (nlines <= 0)
		return;
	if (fp->vptr != &vtbl)
		return;
	trace(("wide_output_header::need(fp = %08lX, nlines = %d)\n{\n",
		(long)fp, nlines));
	wide_output_flush(fp);
	this = (wide_output_header_ty *)fp;
	if (this->line_number > 0 && this->line_number + nlines > this->length)
		wide_output_putwc(fp, (wchar_t)'\f');
	trace(("}\n"));
}


void
wide_output_header_need1(fp, nlines)
	wide_output_ty	*fp;
	int		nlines;
{
	wide_output_header_ty *this;

	if (nlines <= 0)
		return;
	if (fp->vptr != &vtbl)
		return;
	trace(("wide_output_header::need1(fp = %08lX, nlines = %d)\n{\n",
		(long)fp, nlines));
	wide_output_flush(fp);
	this = (wide_output_header_ty *)fp;
	if (this->line_number > 0)
	{
		if (this->line_number + nlines > this->length)
			wide_output_putwc(fp, (wchar_t)'\f');
		else
			wide_output_putwc(fp, (wchar_t)'\n');
	}
	trace(("}\n"));
}


void
wide_output_header_eject(fp)
	wide_output_ty	*fp;
{
	wide_output_header_ty *this;

	if (fp->vptr != &vtbl)
		return;
	trace(("wide_output_header::eject(fp = %08lX)\n{\n", (long)fp));
	this = (wide_output_header_ty *)fp;
	if (this->column > 0)
		wide_output_putwc(fp, (wchar_t)'\n');
	if (this->line_number > 0)
		wide_output_putwc(fp, '\f');
	trace(("}\n"));
}


int
wide_output_header_is_at_top_of_page(fp)
	wide_output_ty	*fp;
{
	wide_output_header_ty *this;

	if (fp->vptr != &vtbl)
		return 0;
	wide_output_flush(fp);
	this = (wide_output_header_ty *)fp;
	return (this->line_number == 0 && this->column == 0);
}
