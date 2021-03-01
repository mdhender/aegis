//
//	aegis - project change supervisor
//	Copyright (C) 1995, 2002-2006, 2008 Peter Miller
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

#include <common/ac/string.h>

#include <common/error.h>
#include <common/mem.h>
#include <common/wstr/list.h>


wstring_list_ty::~wstring_list_ty()
{
    clear();
    delete [] item;
    nitems_max = 0;
    item = 0;
}


wstring_list_ty::wstring_list_ty() :
    nitems(0),
    nitems_max(0),
    item(0)
{
}


wstring_list_ty::wstring_list_ty(const wstring_list_ty &arg) :
    nitems(0),
    nitems_max(0),
    item(0)
{
    for (size_t j = 0; j < arg.nitems; j++)
       	push_back(arg);
}


wstring_list_ty &
wstring_list_ty::operator=(const wstring_list_ty &arg)
{
    if (this != &arg)
    {
	clear();
	push_back(arg);
    }
    return *this;
}


void
wstring_list_ty::clear()
{
    while (!empty())
	pop_back();
}


void
wstring_list_ty::push_back(wstring_ty *w)
{
    if (nitems >= nitems_max)
    {
	//
	// always 8 less than a power of 2, which is
	// most efficient for many memory allocators
	//
	size_t new_nitems_max = nitems_max * 2 + 8;
	wstring_ty **new_item = new wstring_ty * [new_nitems_max];
	if (nitems)
	    memcpy(new_item, item, nitems * sizeof(item[0]));
	delete [] item;
	item = new_item;
	nitems_max = new_nitems_max;
    }
    item[nitems++] = wstr_copy(w);
}


void
wstring_list_ty::push_back(const wstring_list_ty &arg)
{
    if (nitems + arg.nitems > nitems_max)
    {
	//
	// always 8 less than a power of 2, which is
	// most efficient for many memory allocators
	//
	size_t new_nitems_max = nitems_max * 2 + 8;
	while (nitems + arg.nitems > new_nitems_max)
	    new_nitems_max = new_nitems_max * 2 + 8;
	wstring_ty **new_item = new wstring_ty * [new_nitems_max];
	if (nitems)
	    memcpy(new_item, item, nitems * sizeof(item[0]));
	delete [] item;
	item = new_item;
	nitems_max = new_nitems_max;
    }

    //
    // Copy the elements across.
    //
    for (size_t j = 0; j < arg.nitems; ++j)
	item[nitems++] = wstr_copy(arg.item[j]);
}


void
wstring_list_ty::push_front(wstring_ty *w)
{
    if (nitems >= nitems_max)
    {
	//
	// always 8 less than a power of 2, which is
	// most efficient for many memory allocators
	//
	size_t new_nitems_max = nitems_max * 2 + 8;
	wstring_ty **new_item = new wstring_ty * [new_nitems_max];
	if (nitems)
	    memcpy(new_item + 1, item, nitems * sizeof(item[0]));
	item = new_item;
	nitems_max = new_nitems_max;
    }
    else
    {
	for (size_t j = nitems; j > 0; --j)
	    item[j] = item[j - 1];
    }
    ++nitems;
    item[0] = wstr_copy(w);
}


bool
wstring_list_ty::member(wstring_ty *w)
    const
{
    for (size_t j = 0; j < nitems; j++)
	if (wstr_equal(item[j], w))
    	    return true;
    return false;
}


wstring_ty *
wstring_list_ty::unsplit(const char *sep)
    const
{
    return unsplit(0, size(), sep);
}


wstring_ty *
wstring_list_ty::unsplit(size_t start, size_t stop, const char *sep)
    const
{
    static wchar_t  *tmp;
    static size_t   tmplen;

    if (!sep)
	sep = " ";
    size_t seplen = strlen(sep);
    size_t length = 0;
    for (size_t k = start; k <= stop && k < nitems; ++k)
    {
	wstring_ty *s = item[k];
	if (s->wstr_length)
	{
    	    if (length)
		length += seplen;
    	    length += s->wstr_length;
	}
    }

    if (tmplen < length)
    {
	delete tmp;
	for (;;)
	{
	    tmplen = tmplen * 2 + 8;
	    if (length <= tmplen)
		break;
	}
	tmp = new wchar_t[tmplen];
    }

    wchar_t *pos = tmp;
    for (size_t j = start; j <= stop && j < nitems; j++)
    {
	wstring_ty *s = item[j];
	if (s->wstr_length)
	{
	    if (pos != tmp)
	    {
		// can't memcpy, because this is narrow to wide
		for (size_t k = 0; k < seplen; ++k)
	    	    *pos++ = sep[k];
	    }
	    memcpy(pos, s->wstr_text, s->wstr_length * sizeof(wchar_t));
	    pos += s->wstr_length;
	}
    }

    return wstr_n_from_wc(tmp, length);
}


static int
wc_find(const char *s, wchar_t c)
{
    while (*s)
    {
	if (*s == c)
    	    return 1;
	++s;
    }
    return 0;
}


//
// NAME
//	wstring_to_wstring_list - string to wide string list
//
// SYNOPSIS
//	void wstring_to_wstring_list(wstring_list_ty *wlp, wstring_ty *s,
//		char *sep, int ewhite);
//
// DESCRIPTION
//	The wstring_to_wstring_list function is used to form a wide string
//	list from a string.
//
// ARGUMENTS
//	wlp	- where to put the wide string list
//	s	- string to break
//	sep	- separators, default to " " if 0 given
//	ewhite	- supress extra white space around separators
//
// RETURNS
//	The string is broken on spaces into words,
//	using strndup() and wstring_list_append().
//
// CAVEAT
//	Quoting is not understood.
//

void
wstring_list_ty::split(wstring_ty *s, const char *sep, bool ewhite)
{
    static char     white[] = " \t\n\f\r";
    if (!sep)
    {
	sep = white;
	ewhite = true;
    }

    clear();
    const wchar_t *cp = s->wstr_text;
    bool more = false;
    while (*cp || more)
    {
	if (ewhite)
	{
	    while (wc_find(white, *cp))
	       	cp++;
	}
	if (!*cp && !more)
	    break;
	more = false;
	const wchar_t *cp1 = cp;
	while (*cp && !wc_find(sep, *cp))
	    cp++;
	const wchar_t *cp2 = cp;
	if (*cp)
	{
	    cp2 = cp + 1;
	    more = true;
	}
	if (ewhite)
	{
	    while (cp > cp1 && wc_find(white, cp[-1]))
	       	cp--;
	}
	wstring_ty *w = wstr_n_from_wc(cp1, cp - cp1);
	push_back(w);
	wstr_free(w);
	cp = cp2;
    }
}


//
// NAME
//	wstring_list_insert - a insert a wide string into a list
//
// SYNOPSIS
//	void wstring_list_insert(wstring_list_ty *wlp, wstring_ty *wp);
//
// DESCRIPTION
//	The wstring_list_insert function is similar to the
//	wstring_list_append function, however it does not append the
//	wide string unless it is not already in the list.
//
// CAVEAT
//	If the wide string is inserted it is copied.
//

void
wstring_list_ty::push_back_unique(wstring_ty *wp)
{
    if (!member(wp))
	push_back(wp);
}


void
wstring_list_ty::remove(wstring_ty *wp)
{
    for (size_t j = 0; j < nitems; ++j)
    {
	if (wstr_equal(item[j], wp))
	{
	    --nitems;
	    wstr_free(item[j]);
	    for (size_t k = j; k < nitems; ++k)
		item[k] = item[k + 1];
	    break;
	}
    }
}


bool
wstring_list_ty::equal_to(const wstring_list_ty &rhs)
    const
{
    return is_a_subset_of(rhs) && rhs.is_a_subset_of(*this);
}


bool
wstring_list_ty::is_a_subset_of(const wstring_list_ty &rhs)
    const
{
    //
    // test if "*this is a subset of rhs"
    //
    if (nitems > rhs.size())
	return false;
    for (size_t j = 0; j < nitems; ++j)
    {
	if (!rhs.member(item[j]))
    	    return false;
    }
    return true;
}


void
wstring_list_ty::pop_back()
{
    if (nitems > 0)
    {
	--nitems;
	wstr_free(item[nitems]);
    }
}
