//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2010-2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef COMMON_NSTRING_LIST_H
#define COMMON_NSTRING_LIST_H

#include <common/nstring.h>
#include <common/str_list.h>

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
    }

    /**
      * The default constructor.
      */
    nstring_list()
    {
    }

    /**
      * The copy constructor.
      */
    nstring_list(const nstring_list &arg) :
        content(arg.content)
    {
    }

    /**
      * The backwards compatible copy constructor.
      */
    nstring_list(const string_list_ty &arg) :
        content(arg)
    {
    }

    /**
      * The assignment operator.
      */
    nstring_list &operator=(const nstring_list &);

    /**
      * The push_front method is used to add a string to the front of a
      * string list.
      *
      * @param arg
      *     The list of strings to be prepended.
      */
    void
    push_front(const nstring &arg)
    {
        content.push_front(arg.get_ref());
    }

    /**
      * The push_front method is used to add a string list to the front
      * of a string list.
      *
      * @param arg
      *     The list of strings to be prepended.
      */
    void
    push_front(const nstring_list &arg)
    {
        content.push_front(arg.content);
    }

    /**
      * The push_back method is used to add a string to the end of a
      * string list.
      */
    void
    push_back(const nstring &arg)
    {
        content.push_back(arg.get_ref());
    }

    /**
      * The push_back method is used to append an nstring list onto the
      * end of this list.
      *
      * @param arg
      *     The strings to push
      */
    void push_back(const nstring_list &arg);

    /**
      * The push_back_unique method is used to add a string to the end
      * of a string list, provided it isn't already in the list.
      */
    void
    push_back_unique(const nstring &arg)
    {
        content.push_back_unique(arg.get_ref());
    }

    /**
      * The push_back_unique method is used to add a list of strings to
      * the end of a string list, provided it they aren't already in the
      * list.
      */
    void push_back_unique(const nstring_list &arg);

    /**
      * The pop_back method is used to discard the last value in the list.
      * This has O(1) behaviour.
      */
    void
    pop_back(void)
    {
        content.pop_back();
    }

    /**
      * The back method is used to obtain the value of the last element
      * of a string list.
      */
    const nstring
    back(void)
        const
    {
        if (!content.nstrings)
            return nstring();
        return nstring(content.string[content.nstrings - 1]);
    }

    /**
      * The pop_front method is used to discard the first value in the list.
      * This has O(n) behaviour.
      */
    void pop_front(void);

    /**
      * The front method is used to obtain the value of the first element
      * of a string list.
      */
    nstring
    front(void)
        const
    {
        if (!content.nstrings)
            return nstring();
        return nstring(content.string[0]);
    }

    /**
      * The size method is used to obtain the size of the list (the
      * number of string in the list).
      */
    size_t
    size(void)
        const
    {
        return content.nstrings;
    }

    /**
      * The empty method is used to determine whether the string list is
      * empty (no elements) or not.
      */
    bool
    empty(void)
        const
    {
        return !content.nstrings;
    }

    /**
      * The clear method is used to discard all elemets of a string list.
      */
    void clear(void);

    /**
      * The get method is used to obtain the value of the nth element
      * of a string list.
      */
    nstring get(int n) const;

    /**
      * The [] operator is used to obtain the value of the nth element
      * of a string list.
      */
    nstring
    operator[](int n)
        const
    {
        return get(n);
    }

    /**
      * The split method is used to replace the contents of the list
      * with a new list, formed by splitting "str" into several pieces,
      * separated by any pf the characters in "sep".
      *
      * @param str
      *     The string to be split.
      * @param sep
      *     The separators between each field.
      * @param ewhite
      *     If true, get rid of extra white space at the beginning and
      *     end of each field.  Default to false.
      */
    void split(const nstring &str, const char *sep = 0, bool ewhite = false);

    /**
      * The unsplit method is used to form a single string by glueing all
      * of the string list members together.
      *
      * @param separator
      *     The string to place bewteen each list emlement
      */
    nstring unsplit(const char *separator = 0) const;

    /**
      * The unsplit method is used to form a single string by glueing all
      * of the string list members together.
      *
      * @param begin
      *     The index of the first element to be joined
      * @param length
      *     The number of elements to join together.
      * @param separator
      *     The string to place bewteen each list element.
      */
    nstring unsplit(size_t begin, size_t length, const char *separator = 0)
        const;

    /**
      * The member method is used to test whether the given narrow
      * string is present in the narrow string list,
      *
      * @param arg
      *     The narrow string to look for.
      * @returns
      *     bool; false if not present, true if present at least once.
      */
    bool member(const nstring &arg) const;

    /**
      * The sort method is used to perform an <i>in situ</i> sort the
      * string list values in a string list.  The comparison function
      * used is strcmp.
      */
    void sort(void);

    /**
      * The sort method is used to perform an <i>in situ</i> sort the
      * string list values in a string list.  The comparison function
      * used is strverscmp.
      */
    void sort_version(void);

    /**
      * The sort method is used to perform an <i>in situ</i> sort the
      * string list values in a string list.  The comparison function
      * used is strcasecmp.
      */
    void sort_nocase(void);

    /**
      * The gmatch_pattern method is used to determine if there is at
      * least one member of the string list which matches the given
      * pattern.
      *
      * @param pattern
      *     This is a file globbing pattern, such as used by the shell
      *     for expanding file names.  See glob(3) for a definition of
      *     the patterns.
      * @returns
      *     int; 1 for a match, 0 for no match, -1 for invalid pattern.
      */
    int gmatch_pattern(const nstring &pattern) const;

    /**
      * The gmatch_candidate method is used to determine if there is at
      * least one member of the string list which matches the given
      * pattern.
      *
      * @param candidate
      *     This is the candidate string to be matched against each
      *     pattern in the string list.
      * @returns
      *     int; 1 for a match, 0 for no match, -1 for invalid pattern.
      */
    int gmatch_candidate(const nstring &candidate) const;

    /**
      * The remove method is used to remove a string.  It is not an
      * error if the string is not present.  This has O(n) behaviour.
      *
      * @param arg
      *     The string value to be removed.
      */
    void remove(const nstring &arg);

    /**
      * The add-in-situ operator is used to perform an in-situ set union
      * of this string list with another string list.
      *
      * @param rhs
      *     The right has side of the string set union.
      */
    void operator+=(const nstring_list &rhs);

    /**
      * The add operator is used to perform a set union of this string
      * list with another string list.
      *
      * @param rhs
      *     The right has side of the string set union.
      * @returns
      *     string set union
      */
    nstring_list operator+(const nstring_list &rhs) const;

    /**
      * The subtract-in-situ operator is used to perform an in-situ set
      * difference of this string list with another string list.
      *
      * @param rhs
      *     The right has side of the string set difference.
      */
    void operator-=(const nstring_list &rhs);

    /**
      * The subtract operator is used to perform a set difference of
      * this string list with another string list.
      *
      * @param rhs
      *     The right has side of the string set difference.
      * @returns
      *     string set difference
      */
    nstring_list operator-(const nstring_list &rhs) const;

    /**
      * The multiply-in-situ operator is used to perform an in-situ set
      * intersection of this string list with another string list.
      *
      * @param rhs
      *     The right has side of the string set intersection.
      */
    void operator*=(const nstring_list &rhs);

    /**
      * The multiply operator is used to perform a set intersection of
      * this string list with another string list.
      *
      * @param rhs
      *     The right has side of the string set intersection.
      * @returns
      *     string set intersection
      */
    nstring_list operator*(const nstring_list &rhs) const;

    /**
      * The inequality operator is used to determine whether or not this
      * string set is the same as another string set.
      *
      * @param rhs
      *     The right hand side of the comparison
      * @returns
      *     bool; true if not equal (at least one string not in common),
      *     false if equal (all strings in common)
      */
    bool operator!=(const nstring_list &rhs) const;

    /**
      * The equality operator is used to determine whether or not this
      * string set is the same as another string set.
      *
      * @param rhs
      *     The right hand side of the comparison
      * @returns
      *     bool; true if equal (all strings in common), false if not
      *     equal (at least one string not in common).
      */
    bool operator==(const nstring_list &rhs) const;

private:
    /**
      * The content instance variable is used to remember the contents
      * of the string list.
      */
    string_list_ty content;
};

#endif // COMMON_NSTRING_LIST_H
// vim: set ts=8 sw=4 et :
