/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995 Peter Miller;
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to manipulate ssps
 */

#include <stdio.h>
#include <ctype.h>

#include <error.h>
#include <mem.h>
#include <ssp.h>


#define remember(c)						\
	do							\
	{							\
		if (len + 1 > buf_max)				\
		{						\
			buf_max = 2 * buf_max + 16;		\
			buf = mem_change_size(buf, buf_max);	\
		}						\
		buf[len++] = (c);				\
	}							\
		while(0)


static char *readline _((FILE *, char *));

static char *
readline(ifp, ifn)
	FILE		*ifp;
	char		*ifn;
{
	static char	*buf;
	static size_t	buf_max;
	int		icol;
	int		ocol;
	size_t		len;
	int		c;

	icol = 0;
	ocol = 0;
	len = 0;
	for (;;)
	{
		c = getc(ifp);
		switch (c)
		{
		case EOF:
			if (ferror(ifp))
				nfatal("read %s", ifn);
			if (len > 0)
			{
				/*
				 * silently cope with an imcomplete last line
				 */
				buf[len] = 0;
				return buf;
			}
			return 0;

		case '\n':
			/*
			 * discard any trailing white space
			 */
			buf[len] = 0;
			return buf;

		case ' ':
			/*
			 * advance the input column,
			 * but do not remember any characters here
			 */
			++icol;
			break;
		
		case '\t':
			/*
			 * advance the input column,
			 * but do not remember any characters here
			 */
			icol = (icol / 8 + 1) * 8;
			break;

		default:
			/*
			 * discard unprintable characters
			 */
			if (!isprint(c) || isspace(c))
				break;

			/*
			 * only insert tabs at the beginning of the line
			 */
			if (!ocol)
			{
				while (ocol + 8 <= icol)
				{
					remember('\t');
					ocol = (ocol / 8 + 1) * 8;
				}
			}

			/*
			 * insert blanks to reach the correct column
			 */
			while (ocol < icol)
			{
				remember(' ');
				++ocol;
			}

			/*
			 * remember each printable non-blank character
			 */
			remember(c);
			++ocol;
			++icol;
			break;
		}
	}
}


void
ssp(ifn, ofn)
	char		*ifn;
	char		*ofn;
{
	FILE		*ifp;
	FILE		*ofp;
	int		blank;
	char		*s;

	if (ifn)
	{
		ifp = fopen(ifn, "r");
		if (!ifp)
			nfatal("open \"%s\"", ifn);
	}
	else
	{
		ifn = "standard input";
		ifp = stdin;
	}

	if (ofn)
	{
		ofp = fopen(ofn, "w");
		if (!ofp)
			nfatal("create \"%s\"", ofn);
	}
	else
	{
		ofn = "standatd output";
		ofp = stdout;
	}

	blank = -1;
	for (;;)
	{
		s = readline(ifp, ifn);
		if (!s)
		{
			/*
			 * discard trailing blank lines
			 */
			if (blank < 0)
				fatal("%s: empty input", ifn);
			break;
		}

		/*
		 * count blank lines, but do not emit any here
		 */
		if (!*s)
		{
			if (blank >= 0)
				blank = 1;
			continue;
		}

		/*
		 * only emit a single blank line,
		 * no matter how many were in the input
		 */
		if (blank > 0)
			putc('\n', ofp);
		blank = 0;

		/*
		 * output the non-blank line
		 */
		fputs(s, ofp);
		putc('\n', ofp);
		if (ferror(ofp))
			nfatal("write %s", ofn);
	}
	if (fflush(ofp))
		nfatal("write %s", ofn);
	if (ofp != stdout && fclose(ofp))
		nfatal("close %s", ofn);
	if (ifp != stdin && fclose(ifp))
		nfatal("close %s", ifn);
}
