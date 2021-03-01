//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/stdio.h> // for snprintf
#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/nstring.h>
#include <common/nstring/accumulator.h>


static bool
ends_soon(const char *s)
{
    for (;;)
    {
	if (!*s)
	    return true;
	if (!isspace((unsigned char)*s))
	    return false;
	++s;
    }
}

#define UPPER       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define LOWER       "abcdefghijklmnopqrstuvwxyz"
#define ALPHA       UPPER LOWER
#define NUMERIC     "0123456789"
#define HOSTCHARS   "-" ALPHA NUMERIC


static const char *
at_host_dot(const char *cp)
{
    size_t n = strspn(cp, HOSTCHARS ".");
    if (n < 1)
	return 0;
    return (cp + n);
}


static const char *
at_host_star(const char *cp)
{
    size_t n = strspn(cp, HOSTCHARS);
    return (cp + n);
}


static const char *
at_user(const char *cp)
{
    size_t n = strcspn(cp, "-" ALPHA NUMERIC);
    if (n < 1)
	return 0;
    cp += n;

    if (*cp == ':')
    {
	n = strspn(cp + 1, ALPHA NUMERIC "-,?;.:/!%$^*&~\"#'");
	if (n > 0)
	    cp += 1 + n;
    }
    return cp;
}


static const char *
at_urlpath(const char *cp)
{
    if (*cp++ != '/')
	return 0;
    cp += strspn(cp, "-" ALPHA NUMERIC "_$.+!*(),;:@&=?/~#%");
    cp += strcspn(cp, "]'.}>) \t\r\n,\\\"");
    return cp;
}


static const char *
at_number(const char *cp)
{
    size_t n = strspn(cp, NUMERIC);
    if (n < 1)
	return 0;
    return (cp + n);
}


static const char *
at_url(const char *cp)
{
    if (memcmp(cp, "news", 4) == 0)
	cp += 4;
    else if (memcmp(cp, "telnet", 6) == 0)
	cp += 6;
    else if (memcmp(cp, "nntp", 4) == 0)
	cp += 4;
    else if (memcmp(cp, "http", 4) == 0)
    {
	cp += 4;
	if (*cp == 's')
	    ++cp;
    }
    else if (memcmp(cp, "ftp", 3) == 0)
    {
	cp += 3;
	if (*cp == 's')
	    ++cp;
    }
    else if (memcmp(cp, "webcal", 6) == 0)
	cp += 6;
    else
	return 0;

    if (*cp++ != ':')
	return 0;
    if (*cp++ != '/')
	return 0;
    if (*cp++ != '/')
	return 0;

    const char *end_of_user = at_user(cp);
    if (end_of_user && *end_of_user == '@')
	cp = end_of_user + 1;
    const char *end_of_host = at_host_dot(cp);
    if (!end_of_host)
	return 0;
    cp = end_of_host;
    if (*cp == ':')
    {
	const char *end_of_num = at_number(cp + 1);
	if (end_of_num)
	    cp = end_of_num;
    }
    const char *end_of_path = at_urlpath(cp);
    if (end_of_path)
	cp = end_of_path;
    return cp;
}

static const char *
at_url2(const char *cp)
{
    if (memcmp(cp, "www", 3) == 0)
	cp += 3;
    else if (memcmp(cp, "ftp", 3) == 0)
	cp += 3;
    else
	return 0;
    cp = at_host_star(cp);
    if (*cp++ != '.')
	return 0;
    const char *end_of_host = at_host_dot(cp);
    if (!end_of_host)
	return 0;
    cp = end_of_host;
    if (*cp == ':')
    {
	const char *end_of_num = at_number(cp + 1);
	if (end_of_num)
	    cp = end_of_num;
    }
    const char *end_of_path = at_urlpath(cp);
    if (end_of_path)
	cp = end_of_path;
    return cp;
}


static const char *
at_email_lhs(const char *cp)
{
    if (strchr(ALPHA NUMERIC, *cp) == 0)
	return 0;
    ++cp;
    size_t n = strspn(cp, ALPHA NUMERIC ".-");
    return (cp + n);
}


static const char *
at_email_rhs(const char *cp)
{
    if (strchr(ALPHA NUMERIC, *cp) == 0)
	return 0;
    ++cp;
    size_t n = strspn(cp, ALPHA NUMERIC "-");
    return (cp + n);
}


static const char *
at_mailto(const char *cp)
{
    cp = at_email_lhs(cp);
    if (!cp)
	return 0;
    if (*cp++ != '@')
	return 0;
    cp = at_email_rhs(cp);
    if (!cp)
	return 0;
    int n = 0;
    for (;;)
    {
	if (*cp != '.')
	    return (n > 0 ? cp : 0);
	const char *end = at_email_rhs(cp + 1);
	if (!end)
	    return (n > 0 ? cp : 0);
	cp = end;
	++n;
    }
}


static const char *
at_mailto2(const char *cp)
{
    if (memcmp(cp, "mailto:", 7) != 0)
	return 0;
    return at_mailto(cp + 7);
}

static const char *
at_news(const char *cp)
{
    if (memcmp(cp, "news:", 5) != 0)
	return 0;
    cp += 5;
    size_t n = strspn(cp, "-" ALPHA NUMERIC "\\^_{|}~!\"#$%&'()*+,./;:=?`");
    if (n < 1)
	return 0;
    cp += n;
    if (*cp++ != '@')
	return 0;
    cp = at_host_dot(cp);
    if (!cp)
	return 0;
    if (*cp == ':')
    {
	const char *end = at_number(cp + 1);
	if (end)
	    cp = end;
    }
    return cp;
}


static bool
at_anchor(const char *&s, nstring_accumulator &ac)
{
    const char *cp = at_url(s);
    if (cp)
    {
	as_is:
	ac.push_back("<A HREF=\"");
	ac.push_back(s, cp - s);
	ac.push_back("\">");
	ac.push_back(s, cp - s);
	ac.push_back("</A>");
	s = cp;
	return true;
    }
    cp = at_url2(s);
    if (cp)
    {
	nstring url(s, cp - s);
	ac.push_back("<A HREF=\"http://");
	ac.push_back(url);
	ac.push_back("\">");
	ac.push_back(url);
	ac.push_back("</A>");
	s = cp;
	return true;
    }
    cp = at_mailto(s);
    if (cp)
    {
	nstring url(s, cp - s);
	ac.push_back("<A HREF=\"mailto:");
	ac.push_back(url);
	ac.push_back("\">");
	ac.push_back(url);
	ac.push_back("</A>");
	s = cp;
	return true;
    }
    cp = at_mailto2(s);
    if (cp)
	goto as_is;
    cp = at_news(s);
    if (cp)
	goto as_is;
    return false;
}


nstring
nstring::html_quote(bool paragraphs)
    const
{
    const char *s = c_str();
    int column = 0;
    static nstring_accumulator sa;
    sa.clear();
    for (;;)
    {
	unsigned char c = *s++;
	switch (c)
	{
	case '\n':
	    if (ends_soon(s))
	    {
	case 0:
		return sa.mkstr();
	    }
	    if (paragraphs)
	    {
		if (*s == '\n')
		{
		    ++s;
		    sa.push_back("\n<p>\n");
		}
		else
		{
		    sa.push_back("<br>\n");
		}
		while (*s && isspace((unsigned char)*s))
		    ++s;
	    }
	    else
	    {
		sa.push_back('\n');
	    }
	    column = 0;
	    break;

	case '&':
	    sa.push_back("&amp;");
	    column += 5;
	    break;

	case '<':
	    sa.push_back("&lt;");
	    column += 4;
	    break;

	case '>':
	    sa.push_back("&gt;");
	    column += 4;
	    break;

	case '"':
	    sa.push_back("&quot;");
	    column += 6;
	    break;

	case ' ':
	case '\t':
	    //
	    // HTTP only allows lines of up to 510 characters,
	    // so break the line once it gets too wide.
	    //
	    if (column > 70)
	    {
		while (*s && isspace((unsigned char)*s))
		    ++s;
		sa.push_back('\n');
		column = 0;
	    }
	    else
	    {
		sa.push_back(c);
		++column;
	    }
	    break;

	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
	case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
	case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
	case 'V': case 'W': case 'X': case 'Y': case 'Z':
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
	case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
	case 'v': case 'w': case 'x': case 'y': case 'z':
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    --s;
	    if (!at_anchor(s, sa))
	    {
		size_t n = strspn(s, ALPHA NUMERIC);
		assert(n);
		sa.push_back(s, n);
		s += n;
	    }
	    break;

	default:
	    // C locale
	    if (isprint(c))
	    {
		sa.push_back(c);
		++column;
	    }
	    else
	    {
		char temp[10];
		snprintf(temp, sizeof(temp), "&#%d;", c);
		size_t len = strlen(temp);
		sa.push_back(temp, len);
		column += len;
	    }
	    break;
	}
    }
}
