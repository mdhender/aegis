/*
 *	aegis - project change supervisor
 *	Copyright (C) 1993, 1994 Peter Miller.
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
 * MANIFEST: functions to turn text into C strings
 */

#include <ctype.h>
#include <stdio.h>
#include <ac/string.h>

#include <error.h>
#include <txt2c.h>


static int readline _((FILE *, char *, char *, long));

static int
readline(fp, fn, buf, buflen)
	FILE	*fp;
	char	*fn;
	char	*buf;
	long	buflen;
{
	char	*bp;
	char	*ep;

	bp = buf;
	ep = buf + buflen - 1;
	while (bp < ep)
	{
		int c = getc(fp);
		if (c == EOF)
		{
			if (ferror(fp))
				nfatal("read %s", fn);
			if (bp == buf)
				return 0;
			break;
		}
		if (c == '\n')
			break;
		*bp++ = c;
	}
	*bp = 0;
	return 1;
}


static int comment _((char *));

static int
comment(line)
	char	*line;
{
	if (*line != '.' && *line != '\'')
		return 0;
	++line;
	while (*line == ' ' || *line == '\t')
		++line;
	return (line[0] == '\\' && line[1] == '"');
}


static int printable _((char *));

static int
printable(line)
	char	*line;
{
	while (*line)
	{
		if (!isprint(*line) && *line != '\t')
			return 0;
		++line;
	}
	return 1;
}


static void print _((FILE *, char *, char *));

static void
print(fp, fn, line)
	FILE	*fp;
	char	*fn;
	char	*line;
{
	putc('"', fp);
	while (*line)
	{
		int c = *line++;
		switch (c)
		{
		case '\t':
			fprintf(fp, "\\t");
			break;

		case '"':
		case '\\':
			putc('\\', fp);
			putc(c, fp);
			break;

		default:
			if (!isprint(c))
				break;
			putc(c, fp);
			break;
		}
	}
	fprintf(fp, "\",\n");
	if (ferror(fp))
		nfatal("write %s", fn);
}


void
txt2c(ifn, ofn)
	char	*ifn;
	char	*ofn;
{
	FILE	*ifp;
	FILE	*ofp;
	long	linum;
	char	line[1000];
	long	len;
	long	nerrs;
	long	olinum;

	if (ifn)
	{
		ifp = fopen(ifn, "r");
		if (!ifp)
			nfatal("open %s", ifn);
	}
	else
	{
		ifp = stdin;
		ifn = "standard input";
	}

	if (ofn)
	{
		ofp = fopen(ofn, "w");
		if (!ofp)
			nfatal("create %s", ofn);
	}
	else
	{
		ofp = stdout;
		ofn = "standard output";
	}

	nerrs = 0;
	linum = 0;
	olinum = -1;
	while (readline(ifp, ifn, line, sizeof(line)))
	{
		for (;;)
		{
			++linum;
			len = strlen(line);
			if (!len || line[len - 1] != '\\')
				break;
			--len;
			if (!readline(ifp, ifn, line + len, sizeof(line) - len))
				break;
		}
		if (comment(line))
			continue;
		if (!printable(line))
		{
			error
			(
				"%s: %ld: contains unprintable character",
				ifn,
				linum
			);
			++nerrs;
			continue;
		}
		if (len > 72)
		{
			error
			(
				"%s: %ld: line too long (by %ld)",
				ifn,
				linum,
				len - 72
			);
			++nerrs;
		}
		if (olinum != linum)
		{
			fprintf(ofp, "\".lf %ld \\\"%s\\\"\",\n", linum, ifn);
			fprintf(ofp, "#line %ld \"%s\"\n", linum, ifn);
			olinum = linum;
		}
		print(ofp, ofn, line);
		++olinum;
	}
	if (nerrs)
		fatal("found %ld error%s", nerrs, (nerrs == 1 ? "" : "s"));
	if (fclose(ofp))
		nfatal("write %s", ofn);
	fclose(ifp);
}
