/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate to_wides
 */

#include <ac/stdlib.h>
#include <ac/string.h>

#include <error.h> /* for assert */
#include <language.h>
#include <mem.h>
#include <output.h>
#include <output/private.h>
#include <output/to_wide.h>
#include <str.h>
#include <trace.h>
#include <wide_output.h>


typedef struct output_to_wide_ty output_to_wide_ty;
struct output_to_wide_ty
{
	output_ty	inherited;
	wide_output_ty	*deeper;
	int		delete_on_close;
	
	char		*input_buf;
	size_t		input_len;
	size_t		input_max;
	mbstate_t	input_state;				/*lint !e43*/
	int		input_bol;

	wchar_t		*output_buf;
	size_t		output_len;
	size_t		output_max;
};

static mbstate_t initial_state;					/*lint !e86*/


static void output_to_wide_flush _((output_ty *));

static void
output_to_wide_flush(fp)
	output_ty	*fp;
{
	output_to_wide_ty *this;

	trace(("output_to_wide::flush(fp = %08lX)\n{\n", (long)fp));
	this = (output_to_wide_ty *)fp;
	if (this->output_len > 0)
	{
		wide_output_write
		(
			this->deeper,
			this->output_buf,
			this->output_len
		);
		this->output_len = 0;
	}
	wide_output_flush(this->deeper);
	trace(("}\n"));
}


static void output_to_wide_destructor _((output_ty *));

static void
output_to_wide_destructor(fp)
	output_ty	*fp;
{
	output_to_wide_ty	*this;

	trace(("output_to_wide::destructor(fp = %08lX)\n{\n", (long)fp));
	this = (output_to_wide_ty *)fp;

	/*
	 * If there are any input characters left, they are probably a
	 * partial which is never going to be completed.
	 */
	language_human();
	while (this->input_len)
	{
		int		n;
		wchar_t		wc;
		mbstate_t	sequester;			/*lint !e86*/

		/*
		 * The state represents the state before the mutli-byte
		 * character is scanned.  If there is an error, we want
		 * to be able to restore it.
		 */
		sequester = this->input_state;
		n =
			mbrtowc
			(
				&wc,
				this->input_buf,
				this->input_len,
				&this->input_state
			);
		if (n == 0)
		{
			/*
			 * This only happens if there is a NUL in
			 * the input stream.  It shouldn't happen,
			 * so ignore it.
		 	 */
			n = 1;
			goto skip_one;
		}
		if (n < 0)
		{
			/*
			 * Invalid multi byte sequence.  (Even though
			 * the -1 return value is ambiguous, it could be
			 * illegal or it could simply be incomplete, we
			 * know it's illegal because we are never getting
			 * any more characters.)  Restore the state.
			 */
			this->input_state = sequester;

			/*
			 * It's an illegal sequence.  Use the first
			 * character in the buffer, and shuffle the
			 * rest down.
			 */
			wc = (unsigned char)this->input_buf[0];
			n = 1;
		}

		/*
		 * stash the output
		 */
		if (this->output_len >= this->output_max)
		{
			size_t		nbytes;

			this->output_max = 32 + 2 * this->output_max;
			nbytes = sizeof(this->output_buf[0]) * this->output_max;
			this->output_buf =
				mem_change_size(this->output_buf, nbytes);
		}
		this->output_buf[this->output_len++] = wc;

		/*
		 * the one wchar_t used n chars
		 */
		skip_one:
		assert(n <= this->input_len);
		if (n >= this->input_len)
			this->input_len = 0;
		else
		{
			memmove(this->input_buf + n, this->input_buf, n);
			this->input_len -= n;
		}
	}
	language_C();

	/*
	 * Flush the output buffer
	 */
	output_to_wide_flush(fp);

	/*
	 * Delete the deeper output if we were asked to.
	 */
	if (this->delete_on_close)
		wide_output_delete(this->deeper);

	/*
	 * Let the buffers go.
	 */
	if (this->input_buf)
		mem_free(this->input_buf);
	if (this->output_buf)
		mem_free(this->output_buf);

	/*
	 * paranoia
	 */
	this->deeper = 0;
	this->delete_on_close = 0;
	this->input_buf = 0;
	this->input_len = 0;
	this->input_max = 0;
	this->input_state = initial_state;
	this->output_buf = 0;
	this->output_len = 0;
	this->output_max = 0;
	trace(("}\n"));
}


static string_ty *output_to_wide_filename _((output_ty *));

static string_ty *
output_to_wide_filename(fp)
	output_ty	*fp;
{
	output_to_wide_ty *this;
	string_ty	*result;

	trace(("output_to_wide::filename(fp = %08lX)\n{\n", (long)fp));
	this = (output_to_wide_ty *)fp;
	result = wide_output_filename(this->deeper);
	trace(("return \"%s\";\n", result->str_text));
	trace(("}\n"));
	return result;
}


static long output_to_wide_ftell _((output_ty *));

static long
output_to_wide_ftell(fp)
	output_ty	*fp;
{
	trace(("output_to_wide::ftell(fp = %08lX)\n", (long)fp));
	return -1;
}


static void output_to_wide_write _((output_ty *, const void *, size_t));

static void
output_to_wide_write(fp, input_p, len)
	output_ty	*fp;
	const void	*input_p;
	size_t		len;
{
	const char	*input;
	output_to_wide_ty *this;

	trace(("output_to_wide::write(fp = %08lX, data = %08lX, len = %ld)\n{\n", (long)fp, (long)input_p, (long)len));
        this = (output_to_wide_ty *)fp;
	language_human();
	input = input_p;
	while (len > 0)
	{
		int		c;
		int		n;
		wchar_t		wc;
		mbstate_t	sequester;			/*lint !e86*/

		c = (unsigned char)*input++;
		--len;

		/*
		 * Track whether we are at the start of a line.
		 * (This makes the assumption that \n will no be part
		 * of any multi-byte sequence.)
		 */
		this->input_bol = (c == '\n');

		/*
		 * The NUL character is not legal in the output stream.
		 * Throw it away if we see it.
		 */
		assert(c);
		if (c == 0)
			continue;
		
		/*
		 * drop the character into the buffer
		 */
		if (this->input_len >= this->input_max)
		{
			this->input_max = 4 + 2 * this->input_max;
			this->input_buf =
				mem_change_size
				(
					this->input_buf,
					this->input_max
				);
		}
		this->input_buf[this->input_len++] = c;

		/*
		 * The state represents the state before the mutli-byte
		 * character is scanned.  If there is an error, we want
		 * to be able to restore it.
		 */
		sequester = this->input_state;
		n =
			mbrtowc
			(
				&wc,
				this->input_buf,
				this->input_len,
				&this->input_state
			);
		if (n == 0)
		{
			/*
			 * This only happens if there is a NUL in
			 * the input stream.  It shouldn't happen,
			 * so ignore it.
		 	 */
			n = 1;
			goto skip_one;
		}
		if (n < 0)
		{
			/*
			 * Invalid multi byte sequence.
			 * Restore the state.
			 */
			this->input_state = sequester;

			/*
			 * The error return from mbrtowc fails to
			 * distinguish between (a) a sequence the will
			 * never be a legal character, and (b) a partial
			 * sequence that could potentially be valid if
			 * there were more characters in the buffer.
			 */
			if (this->input_len < MB_CUR_MAX)
			{
				/*
				 * We do, however, know the maximum
				 * multi-byte sequence length.	Anthing
				 * less than this is probably a partial,
				 * so collect more.
				 */
				continue;
			}

			/*
			 * It's an illegal sequence.  Use the first
			 * character in the buffer, and shuffle the
			 * rest down.
			 */
			wc = (unsigned char)this->input_buf[0];
			n = 1;
		}

		/*
		 * stash the output
		 */
		if (this->output_len >= this->output_max)
		{
			size_t		nbytes;

			this->output_max = 32 + 2 * this->output_max;
			nbytes = sizeof(this->output_buf[0]) * this->output_max;
			this->output_buf =
				mem_change_size(this->output_buf, nbytes);
		}
		this->output_buf[this->output_len++] = wc;

		/*
		 * If the output buffer is starting to fill up, empty it.
		 */
		if (this->output_len >= 1024)
		{
			language_C();
			output_to_wide_flush(fp);
			language_human();
		}

		/*
		 * the one wchar_t used n chars
		 */
		skip_one:
		assert(n <= this->input_len);
		if (n >= this->input_len)
			this->input_len = 0;
		else
		{
			memmove(this->input_buf + n, this->input_buf, n);
			this->input_len -= n;
		}
	}
	language_C();
	trace(("}\n"));
}


static int output_to_wide_page_width _((output_ty *));

static int
output_to_wide_page_width(fp)
	output_ty	*fp;
{
	output_to_wide_ty *this;

	trace(("output_to_wide::width(fp = %08lX)\n", (long)fp));
	this = (output_to_wide_ty *)fp;
	return wide_output_page_width(this->deeper);
}


static int output_to_wide_page_length _((output_ty *));

static int
output_to_wide_page_length(fp)
	output_ty	*fp;
{
	output_to_wide_ty *this;

	trace(("output_to_wide::length(fp = %08lX)\n", (long)fp));
	this = (output_to_wide_ty *)fp;
	return wide_output_page_length(this->deeper);
}


static void output_to_wide_eoln _((output_ty *));

static void
output_to_wide_eoln(fp)
	output_ty	*fp;
{
	output_to_wide_ty *this;

	trace(("output_to_wide::eoln(fp = %08lX)\n{\n", (long)fp));
	this = (output_to_wide_ty *)fp;
	if (!this->input_bol)
		output_fputc(fp, '\n');
	trace(("}\n"));
}


static output_vtbl_ty vtbl =
{
	sizeof(output_to_wide_ty),
	output_to_wide_destructor,
	output_to_wide_filename,
	output_to_wide_ftell,
	output_to_wide_write,
	output_to_wide_flush,
	output_to_wide_page_width,
	output_to_wide_page_length,
	output_to_wide_eoln,
	"to_wide",
};


output_ty *
output_to_wide_open(deeper, delete_on_close)
	wide_output_ty	*deeper;
	int		delete_on_close;
{
	output_ty	*result;
	output_to_wide_ty *this;

	trace(("output_to_wide::new(deeper = %08lX)\n{\n", (long)deeper));
	result = output_new(&vtbl);
	this = (output_to_wide_ty *)result;
	this->deeper = deeper;
	this->delete_on_close = delete_on_close;
	this->input_buf = 0;
	this->input_len = 0;
	this->input_max = 0;
	this->input_state = initial_state;
	this->input_bol = 1;
	this->output_buf = 0;
	this->output_len = 0;
	this->output_max = 0;
	trace(("return %08lX;\n", (long)result));
	trace(("}\n"));
	return result;
}
