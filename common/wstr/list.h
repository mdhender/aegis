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

#ifndef COMMON_WSTR_LIST_H
#define COMMON_WSTR_LIST_H

#include <common/wstr.h>

/** \addtogroup WStringList
  * \brief Lists of wide char strings
  * \ingroup WString
  * @{
  */

/**
  * The wstring_list_ty class is used to represent a list of wstring_ty
  * elements.
  *
  * \note
  *     This is the inner declaration of wstring_list; if at all
  *     possible, use the wstring_list class instead.
  */
class wstring_list_ty
{
public:
    /**
      * The destructor.
      *
      * \note
      *     It is not virtual.  Do not derive from this class.
      */
    ~wstring_list_ty();

    /**
      * The default constructor.
      */
    wstring_list_ty();

    /**
      * The copy constructor.
      */
    wstring_list_ty(const wstring_list_ty &);

    /**
      * The assignment operator.
      */
    wstring_list_ty &operator=(const wstring_list_ty &);

    /**
      * The size method may be used to determine how many items are in
      * the wide strinbg list.
      */
    size_t size() const { return nitems; }

    /**
      * The empty method is used to determine whether the wide string
      * list is empty (contains no wide strings).
      */
    bool empty() const { return (nitems == 0); }

    /**
      * The get method is used to obtain the Nth wide string pointer.
      *
      * \param n
      *     The zero-based string index.
      * \note
      *     No range checking is performed.
      */
    wstring_ty *get(size_t n) const { return item[n]; }

    /**
      * The [] operator is used to obtain the Nth wide string pointer.
      *
      * \param n
      *     The zero-based string index.
      * \note
      *     No range checking is performed.
      */
    wstring_ty *operator[](size_t n) const { return get(n); }

    /**
      * The clear method is used to discard all elements of the list.
      */
    void clear();

    /**
      * The push_back mthod is used to append to a wide string list.
      * This is an O(1) operation.
      *
      * \param arg
      *     The string to be appended to the list.
      */
    void push_back(wstring_ty *arg);

    /**
      * The push_back method is used to append to a wide string list to
      * the end of this list.  This is an O(n) operation.
      *
      * \param arg
      *     The string list to be appended to the list.
      */
    void push_back(const wstring_list_ty &arg);

    /**
      * The pop_back method is used to discard the last element of the list.
      */
    void pop_back();

    /**
      * The push_back_unique method is similar to the push_back method,
      * however it does not append the wide string if it is already in
      * the list.
      *
      * \param arg
      *     The wide string to conditionally append to a wide string list.
      */
    void push_back_unique(wstring_ty *arg);

    /**
      * The push_front method is used to prepend to a wide string list.
      * Note that this is an O(n) operation.
      *
      * \param arg
      *     The string to be appended to the list.
      */
    void push_front(wstring_ty *arg);

    /**
      * The member method is used to determine whether the given wide
      * string is a member of this list.  This is an O(n) operation.
      *
      * \param arg
      *     The wide string to look for.
      *
      * \returns
      *     bool; true if the string is present, false if it is not.
      */
    bool member(wstring_ty *arg) const;

    /**
      * The remove method is used to remove a wide string from a wide
      * string list.  It is <i>not</i> an error if the string is not a
      * list member.
      *
      * \param arg
      *     The wide string to remove.
      */
    void remove(wstring_ty *arg);

    /**
      * The unsplit method is used to form a wide string from a wide
      * string list.
      *
      * \param start
      *     The first element to use.
      * \param stop
      *     The last element to be used (NOT one past it, as is common
      *     for C++ functions).
      * \param sep
      *     The separator string to use between elements.
      *     If NULL a single space will be used.
      */
    wstring_ty *unsplit(size_t start, size_t stop, const char *sep = 0) const;

    /**
      * The unsplit method is used to form a wide string from a wide
      * string list.  All emelents will be used.
      *
      * \param sep
      *     The separator string to use between elements.
      *     If NULL a single space will be used.
      */
    wstring_ty *unsplit(const char *sep = 0) const;

    /**
      * The split method is used to split a wide string into compendent pieces.
      *
      * \param wis
      *     The wide string to be split.
      * \param sep
      *     The separator charactres.  Any one of these will split.
      *     Defauls to all white space characters.
      * \param ewhite
      *     True if should discard extra while space at the start and
      *     end of each piece.  Defaults to false.
      */
    void split(wstring_ty *wis, const char *sep = 0, bool ewhite = false);

    /**
      * The equal_to method is used to test whether this wide string
      * list is equal to another wide string list.  The order of
      * elements is not relevant.
      *
      * \param rhs
      *     The other string list to compare against.
      */
    bool equal_to(const wstring_list_ty &rhs) const;

    /**
      * The is_a_subset_of method is used to test whether this wide
      * string list is an improper subset of another wide string list.
      * The order of elements is not relevant.
      *
      * \param rhs
      *     The other string list to compare against.
      */
    bool is_a_subset_of(const wstring_list_ty &rhs) const;

private:
    /**
      * The nitems instance variable is used to remember how many
      * elements of the "item" array have been used.
      */
    size_t nitems;

    /**
      * The nitems_max instance variable is used to remember how many
      * elements have been allocated in the "item" array.
      */
    size_t nitems_max;

    /**
      * The items instance variable is used to remember the base of an
      * arrat of wide string pointers.
      */
    wstring_ty **item;
};

inline bool
operator==(const wstring_list_ty &lhs, const wstring_list_ty &rhs)
{
    return lhs.equal_to(rhs);
}

inline bool
operator!=(const wstring_list_ty &lhs, const wstring_list_ty &rhs)
{
    return !lhs.equal_to(rhs);
}

/** @} */
#endif // COMMON_WSTR_LIST_H
