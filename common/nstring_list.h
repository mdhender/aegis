//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface of the nstring_list class
//

#ifndef COMMON_NSTRING_LIST_H
#define COMMON_NSTRING_LIST_H

#pragma interface "nstring_list"

#include <nstring.h>
#include <str_list.h>

/**
  * The nstring_list class is used to represent a dynamically sized list
  * of nstrings.
  */
class nstring_list
{
public:
    /**
      * The destructor.
      *
      * This class is not to be derived from because the destructor is
      * not virtual.
      */
    ~nstring_list()
    {
	string_list_destructor(&content);
    }

    /**
      * The default constructor.
      */
    nstring_list()
    {
	string_list_constructor(&content);
    }

    /**
      * The copy constructor.
      */
    nstring_list(const nstring_list &arg)
    {
	string_list_copy(&content, &arg.content);
    }

    /**
      * The assignment operator.
      */
    nstring_list &operator=(const nstring_list &);

    /**
      * The push_back method is used to add a string to the end of a
      * string list.
      */
    void
    push_back(const nstring &arg)
    {
	string_list_append(&content, arg.get_ref());
    }

    /**
      * The back method is used to obtain the value of the last element
      * of a string list.
      */
    const nstring
    back()
	const
    {
	if (!content.nstrings)
	    return nstring();
	return nstring(content.string[content.nstrings - 1]);
    }

    /**
      * The size method is used to obtain the size of the list (the
      * number of string in the list).
      */
    size_t
    size()
	const
    {
	return content.nstrings;
    }

    /**
      * The empty method is used to determine whether the string list is
      * empty (no elements) or not.
      */
    bool
    empty()
	const
    {
	return !content.nstrings;
    }

    /**
      * The clear method is used to discard all elemets of a string list.
      */
    void clear();

    /**
      * The [] operator is used to obtain the value of the nth element
      * of a string list.
      */
    nstring operator[](int n) const;

private:
    /**
      * The content instance variable is used to remember the contents
      * of the string list.
      */
    string_list_ty content;
};

#endif // COMMON_NSTRING_LIST_H
