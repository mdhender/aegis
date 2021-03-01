/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate https
 */

#include <ac/ctype.h>
#include <ac/errno.h>
#include <ac/magic.h>
#include <ac/stdarg.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>

#include <change.h>
#include <emit/project.h>
#include <http.h>
#include <now.h>
#include <project.h>
#include <progname.h>
#include <str.h>
#include <version_stmp.h>


void
http_fatal(const char *fmt, ...)
{
    va_list         ap;

    va_start(ap, fmt);
    printf("Content-Type: text/html\n\n");
    printf("<html><head><title>Error</title></head><body><h1>Error</h1>\n");
    printf("The request failed because:\n<em>");
    vprintf(fmt, ap);
    va_end(ap);
    printf("</em>\n</body></html>\n");
    exit(0);
}


const char *
http_getenv(const char *name)
{
    char            *result;

    result = getenv(name);
    if (!result)
	http_fatal("Environment variable $%s not set.", name);
    return result;
}


void
html_escape_charstar(const char *s)
{
    for (;;)
    {
	unsigned char c = *s++;
	switch (c)
	{
	case 0:
	    return;

	case '%':
	case '?':
	case '#':
	case ':':
	case '"':
	case ' ':
	case '~':
	    printf("%%%02X", c);
	    break;

	default:
	    /* C locale */
	    if (isprint(c))
		putchar(c);
	    else
		printf("%%%02X", c);
	    break;
	}
    }
}


void
html_escape_string(string_ty *s)
{
    html_escape_charstar(s->str_text);
}


static int
ends_soon(const char *s)
{
    for (;;)
    {
	if (!*s)
	    return 1;
	if (!isspace(*s))
	    return 0;
	++s;
    }
}


void
html_encode_charstar(const char *s)
{
    int             column;

    column = 0;
    for (;;)
    {
	unsigned char   c;

	c = *s++;
	switch (c)
	{
	case 0:
	    return;

	case '&':
	    printf("&amp;");
	    column += 5;
	    break;

	case '<':
	    printf("&lt;");
	    column += 4;
	    break;

	case '>':
	    printf("&gt;");
	    column += 4;
	    break;

	case '\n':
	    if (ends_soon(s))
		return;
	    if (*s == '\n')
	    {
		++s;
		printf("\n<p>\n");
	    }
	    else
		printf("<br>\n");
	    column = 0;
	    while (*s && isspace((unsigned char)*s))
		++s;
	    break;

	case ' ':
	case '\t':
	    /*
	     * HTTP only allows lines of up to 510 characters,
	     * so break the line once it gets too wide.
	     */
	    if (column > 70)
	    {
		while (*s && isspace((unsigned char)*s))
		    ++s;
		putchar('\n');
		column = 0;
	    }
	    else
	    {
		putchar(c);
		++column;
	    }
	    break;

	default:
	    /* C locale */
	    if (isprint(c))
	    {
		putchar(c);
		++column;
	    }
	    else
	    {
		printf("&#%d;", c);
		column += 5;
	    }
	    break;
	}
    }
}


void
html_encode_string(string_ty *s)
{
    html_encode_charstar(s->str_text);
}


void
http_content_type_header(string_ty *filename)
{
    magic_t         cookie;
    const char      *content_type;

    cookie = magic_open(MAGIC_MIME);
    if (!cookie)
	http_fatal("magic_open(): %s", strerror(errno));
    content_type = magic_file(cookie, filename->str_text);
    if (!content_type)
    {
	http_fatal
	(
	    "magic_file(\"%s\"): %s",
	    filename->str_text,
	    magic_error(cookie)
	);
    }
    printf("Content-Type: %s\n\n", content_type);
    magic_close(cookie);
    fflush(stdout);
}


void
html_footer(void)
{
    time_t          tmp;

    tmp = now();
    printf("<hr>\n");
    printf("This page was generated by <em>%s</em>\n", progname_get());
    printf("version %s\n", version_stamp());
    printf("on %.24s.\n", ctime(&tmp));
    printf("</body></html>\n");
}


static void
emit_project_stylesheet(project_ty *pp)
{
    /*
     * Netscape 4.x has numerous CSS bugs, two of which need mentioning.
     * 1. If a style sheet is not present Netscape says 404 Not found, when
     * it should silently ignore it.  2. Style sheets who's media is not
     * "screen" will be ignored.  Fortunately we can use (2) to get around (1).
     */
    if (pp && pp->parent)
	emit_project_stylesheet(pp->parent);
    else
    {
	printf("<style type=\"text/css\">\n"
	    "tr.even-group { background-color: #CCCCCC; }\n"
	    "body { background-color: white; }\n"
	    "</style>\n");
	printf("<link rel=\"stylesheet\" type=\"text/css\" "
	    "href=\"/aedefault.css\" media=\"all\">\n");
    }
    if (pp)
    {
	printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"/");
	html_escape_string(project_name_get(pp));
	printf(".css\" media=\"all\">\n");
    }
}


void
html_header(project_ty *pp)
{
    printf("Content-Type: text/html\n\n");
    printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\""
	"\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n");
    printf("<html><head>\n");
    printf("<meta name=\"ROBOTS\" content=\"NOINDEX, NOFOLLOW\">\n");
    printf("<meta name=\"GENERATOR\" content=\"%s\">\n", http_script_name());
    printf("<meta http-equiv=\"Content-Type\" "
	"content=\"text/html; charset=ISO-8859-1\">\n");
    emit_project_stylesheet(pp);
}


void
emit_change(change_ty *cp)
{
    long            n;

    emit_project(cp->pp);
    if (cp->bogus)
	return;
    printf(",<br>\nChange <a href=\"%s/", http_script_name());
    html_escape_string(project_name_get(cp->pp));
    n = magic_zero_decode(cp->number);
    printf(".C%ld/@@menu\">%ld</a>", n, n);
}


void
emit_change_but1(change_ty *cp)
{
    if (cp->bogus)
	emit_project_but1(cp->pp);
    else
    {
	emit_project(cp->pp);
	printf(",<br>\nChange %ld", magic_zero_decode(cp->number));
    }
}


const char *
http_script_name(void)
{
    static const char *result;

    if (!result)
	result = http_getenv("SCRIPT_NAME");
    return result;
}


void
emit_project_href(project_ty *pp, const char *modifier)
{
    printf("<a href=\"%s/", http_script_name());
    html_escape_string(project_name_get(pp));
    printf("/");
    if (modifier && *modifier)
	printf("@@%s", modifier);
    printf("\">");
}


void
emit_change_href_n(project_ty *pp, long n, const char *modifier)
{
    printf("<a href=\"%s/", http_script_name());
    html_escape_string(project_name_get(pp));
    printf(".C%ld/", n);
    if (modifier && *modifier)
	printf("@@%s", modifier);
    printf("\">");
}


void
emit_change_href(change_ty *cp, const char *modifier)
{
    if (cp->bogus)
	emit_project_href(cp->pp, modifier);
    else
	emit_change_href_n(cp->pp, magic_zero_decode(cp->number), modifier);
}


void
emit_file_href(change_ty *cp, string_ty *filename, const char *modifier)
{
    printf("<a href=\"%s/", http_script_name());
    html_escape_string(project_name_get(cp->pp));
    if (!cp->bogus)
	printf(".C%ld", magic_zero_decode(cp->number));
    printf("/");
    html_escape_string(filename);
    if (modifier && *modifier)
	printf("@@%s", modifier);
    printf("\">");
}


void
emit_rect_image(int width, int height, const char *label)
{
    if (width < 3)
	width = 3;
    if (height < 3)
	height = 3;
    printf("<img src=\"%s/@@rect@%d@%d", http_script_name(), width, height);
    if (label && *label)
    {
	printf("@");
	html_escape_charstar(label);
    }
    printf("\" width=%d height=%d>\n", width, height);
}