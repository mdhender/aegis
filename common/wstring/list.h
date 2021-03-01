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

#ifndef COMMON_WSTRING_LIST_H
#define COMMON_WSTRING_LIST_H

#include <common/wstring.h>
#include <common/wstr/list.h>

/**
  * The wstring_list class is used to represent a dynamically sized list
  * of wstrings.
  */
class wstring_list
{
public:
    /**
      * The destructor.
      *
      * This class is not to be derived from because the destructor is
      * not virtual.
      */
    ~wstring_list() {}

    /**
      * The default constructor.
      */
    wstring_list() {}

    /**
      * The copy constructor.
      */
    wstring_list(const wstring_list &arg) :
	content(arg.content)
    {
    }

    /**
      * The assignment operator.
      */
    wstring_list &operator=(const wstring_list &);

    /**
      * The push_back method is used to add a string to the end of a
      * string list.
      */
    void
    push_back(const wstring &arg)
    {
	content.push_back(arg.get_ref());
    }

    /**
      * The push_back method is used to add a string to the end of a
      * string list.
      */
    void
    push_back(const wstring_list &arg);

    /**
      * The back method is used to obtain the value of the last element
      * of a string list.
      */
    const wstring
    back()
	const
    {
	if (content.empty())
	    return wstring();
	return wstring(content[content.size() - 1]);
    }

    /**
      * The size method is used to obtain the size of the list (the
      * number of string in the list).
      */
    size_t
    size()
	const
    {
	return content.size();
    }

    /**
      * The empty method is used to determine whether the string list is
      * empty (no elements) or not.
      */
    bool
    empty()
	const
    {
	return content.empty();
    }

    /**
      * The clear method is used to discard all elemets of a string list.
      */
    void clear();

    /**
      * The get is used to obtain the value of the nth element
      * of a string list.
      */
    wstring get(int n) const;

    /**
      * The [] operator is used to obtain the value of the nth element
      * of a string list.
      */
    wstring operator[](int n) const { return get(n); }

    /**
      * The unsplit method is used to form a single string by gluing all
      * of the string list members together.
      *
      * @param separator
      *     what to put between the strings, defaults to a single cpace.
      */
    wstring unsplit(const char *separator = 0) const;

    /**
      * The unsplit method is used to form a single string by gluing all
      * of the string list members together.
      *
      * @param first
      *     The place to start joining
      * @param last
      *     how many to join
      * @param separator
      *     what to put between the strings, defaults to a single cpace.
      */
    wstring unsplit(size_t first, size_t last, const char *separator = 0) const;

private:
    /**
      * The content instance variable is used to remember the contents
      * of the string list.
      */
    wstring_list_ty content;
};

#endif // COMMON_WSTRING_LIST_H
