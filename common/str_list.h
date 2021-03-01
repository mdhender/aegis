//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1996, 1997, 2003-2006, 2008 Peter Miller
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

#ifndef COMMON_STR_LIST_H
#define COMMON_STR_LIST_H

#include <common/str.h>

/** \addtogroup String_List
  * \brief String Lists
  * \ingroup String
  * @{
  */

/**
  * The string_list_ty type is used to represent a list of string
  * (string_ty *) values.
  */
class string_list_ty
{
public:
    /**
      * The destructor.
      */
    ~string_list_ty();

    /**
      * The default constructor.  This list will be initialised as
      * being empty.
      */
    string_list_ty();

    /**
      * The copy constructor.
      */
    string_list_ty(const string_list_ty &);

    /**
      * The assignment operator.
      */
    string_list_ty &operator=(const string_list_ty &);

    /**
      * The member method is used to test whether a given string value
      * is a member of the string list.
      *
      * \param arg
      *     The string value to search for.
      * \returns
      *     bool: true if the string is present, false if it is not.
      */
    bool member(string_ty *arg) const;

    /**
      * The member_nocase method is used to test whether a given string
      * value is a member of the string list.  The conparison is
      * performed in a case-INsensitive way.
      *
      * \param arg
      *     The string value to search for.
      * \returns
      *     bool: true if the string is present, false if it is not.
      */
    bool member_nocase(string_ty *arg) const;

    /**
      * The size method is used to obtain the number of string in the
      * list.
      */
    size_t size() const { return nstrings; }

    /**
      * The empty method i used to determine if the list is empty
      * (contains no strings) or not.
      *
      * \returns
      *     bool: true if the list contains no strings, false if the
      *     list contains one or more strings.
      */
    bool empty() const { return (nstrings == 0); }

    /**
      * The [] operator may be used to extract a list member.  It may
      * only be used as an r-value.
      */
    string_ty *operator[](size_t n) const { return string[n]; }

    /**
      * The unsplit method is used to form a string from a word list.
      *
      * \param start
      *     The first string in the list to start from.
      * \param finish
      *     One past the last string in the list be be used.
      * \param sep
      *     The seapator string between words.  If yoy give NULL, it
      *     will use a single space.
      * \returns
      *     A pointer to the newly formed string in dynamic memory.
      *
      * \note
      *     It is the responsibility of the caller to ensure that the
      *     new string is freed when finished with, by a call to str_free().
      */
    string_ty *unsplit(size_t start, size_t finish, const char *sep) const;

    /**
      * The unsplit method is used to form a string from a word list.
      *
      * \param sep
      *     The seapator string between words.  If yoy give NULL, it
      *     will use a single space.
      * \returns
      *     A pointer to the newly formed string in dynamic memory.
      *
      * \note
      *     It is the responsibility of the caller to ensure that the
      *     new string is freed when finished with, by a call to str_free().
      */
    string_ty *unsplit(const char *sep = 0) const;

    /**
      * The push_front method is used to prepend a string to the list.
      *
      * \param arg
      *     The string to be appended.
      * \note
      *     This is not terribly efficient, try not to use it too often,
      *     because it has to shuffle all of the string contents up by
      *     one.
      */
    void push_front(string_ty *arg);

    /**
      * The push_front method is used to prepend a string to the list.
      *
      * \param arg
      *     The list of string to be appended.
      * \note
      *     This is not terribly efficient, try not to use it too often,
      *     because it has to shuffle all of the string contents up first.
      */
    void push_front(const string_list_ty &arg);

    /**
      * The push_back method is used to append a string to the list.
      * This has O(1) insert times.
      *
      * \param arg
      *     The string to be appended.
      */
    void push_back(string_ty *arg);

    /**
      * The push_back method is used to append a string to the list,
      * but only if it isn't already in the list.
      * This has O(n) insert times.
      *
      * \param arg
      *     The string to be appended.
      */
    void push_back_unique(string_ty *arg);

    /**
      * The push_back method is used to append all the strings string in
      * one list to the end of this list.  This has O(1) insert times.
      *
      * \param arg
      *     The strings to be appended.
      */
    void push_back(const string_list_ty &arg);

    /**
      * The push_back method is used to append all the strings string in
      * one list to the end of this list.  This has O(n*m) insert times.
      *
      * \param arg
      *     The strings to be appended.
      */
    void push_back_unique(const string_list_ty &arg);

    /**
      * The split method is used to convert a string to a word list.
      * The string list is cleared()ed before the string is split into it.
      *
      * \param arg
      *     The string to be split into pieces.
      * \param sep
      *      Separator characters; sequences of noe or more of these
      *      characters seaprate each part.  Defaults to " " if NULL is
      *      given.
      * \param white
      *      if true, supress extra white space around separators
      * \note
      *     Quoting is not understood.
      */
    void split(string_ty *arg, const char *sep = 0, bool white = false);

    /**
      * The remove method is used to remove a string.  It is not an
      * error if the string is not present.
      * This has O(n) behaviour.
      *
      * \param arg
      *     The string value to be removed.
      */
    void remove(string_ty *arg);

    /**
      * The remove method is used to remove a set of string.  It is not
      * an error if one or more of the strings are not present.
      * This has O(n*m) behaviour.
      *
      * \param arg
      *     The string values to be removed.
      */
    void remove(const string_list_ty &arg);

    /**
      * The clear method is used to remove all list elements.  This has
      * O(n) behaviour.  Afterwards, the list is once again empty.
      */
    void clear();

    /**
      * The front method is used to obtain the first string value in the
      * list.
      */
    string_ty *front() { return (nstrings ? string[0] : 0); }

    /**
      * The back method is used to obtain the last string value in the
      * list.
      */
    string_ty *back() { return (nstrings ? string[nstrings - 1] : 0); }

    /**
      * The pop_front method is used to discard the first value in the list.
      * This has O(n) behaviour.
      */
    void pop_front();

    /**
      * The pop_back method is used to discard the last value in the list.
      * This has O(1) behaviour.
      */
    void pop_back();

    /**
      * The equal method is used to determine if this string list is
      * equal to another string list.  Two lists are considered equal if
      * they both contains the same strings, regardless of order.
      *
      * \returns
      *     bool: true if equal, false if not
      */
    bool equal(const string_list_ty &arg) const;

    /**
      * The subset method is used to determine if this string list is
      * a subset of another string list, regardless of order.
      *
      * \param arg
      *     strings list to test against,
      *     i.e. that is (*this is-a-subset-of arg).
      * \returns
      *     bool: true if subset, false if not
      *
      * \note
      *     By subset, this also includes improper subsets (equality).
      */
    bool subset(const string_list_ty &arg) const;

    /**
      * The sort method is used to perform an <i>in situ</i> sort the
      * string list values in a string list.  The comparison function
      * used is strcmp.
      */
    void sort();

    /**
      * The sort method is used to perform an <i>in situ</i> sort the
      * string list values in a string list.  The comparison function
      * used is strcasecmp.
      */
    void sort_nocase();

    /**
      * The sort method is used to perform an <i>in situ</i> sort the
      * string list values in a string list.  The comparison function
      * used is strverscmp.
      */
    void sort_version();

    /**
      * The quote_shell method is used to produce a new string list by
      * quoting the strings of this string list.
      */
    string_list_ty quote_shell() const;

    /**
      * The validate method is used to validate a string list.
      * Usually used for debugging, usually with assert.
      *
      * \returns
      *     bool: true if the string list is valis, false if not.
      */
    bool validate() const;

    /**
      * The intersection method is used to calculate the set
      * intersection between this set of strings and the rhs set of
      * strings.
      */
    string_list_ty intersection(const string_list_ty &rhs) const;

// private:
    size_t nstrings;
    size_t nstrings_max;
    string_ty **string;
};

/**
  * The string_list_member function is used to test for the existence
  * of a string in a string list.  (This is an O(n) search.)
  */
inline DEPRECATED int
string_list_member(const string_list_ty *slp, string_ty *s)
{
    return slp->member(s);
}

/**
  * The wl2str function is used to convert a list of strings into a
  * single string, separated by the given separator.
  *
  * @param slp
  *	The string list to be joined.
  * @param start
  *	The index of the first string in the list to be joined (counting
  *	from zero).
  * @param finish
  *	The index of the last string in the list to be joined (counting
  *	from one, or one past the end, counting from zero).
  * @param sep
  *     The separator characters.  Defaults to spaces if NULL.
  */
inline DEPRECATED string_ty *
wl2str(const string_list_ty *slp, int start, int finish, const char *sep)
{
    return slp->unsplit(start, finish, sep);
}

/**
  * The str2wl function is used to break a string up into several strings
  * at a given separatopr character.
  *
  * @param slp
  *	The place to put the resulting list of strings.  They are in
  *	the same order as they appeared in the input string (arg).
  * @param arg
  *	The input string to be split into component strings.
  * @param sep
  *	This is the set of separator characters.  Supostrings will be
  *	extracted if *any* of the characters in this strings are seen
  *	(like $IFS in the shell).  If NULL, white space will be used.
  * @param white
  *	If true (non-zero) leading ans trailing white space will be
  *	removed from the result strings.  If false (zero) white space
  *	will be retained.
  */
inline DEPRECATED void
str2wl(string_list_ty *slp, string_ty *arg, const char *sep, int white)
{
    slp->split(arg, sep, white);
}

/**
  * The string_list_prepend function is used to add another string to
  * the start of a string list.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string to be added to the front of lhs.
  */
inline DEPRECATED void
string_list_prepend(string_list_ty *lhs, string_ty *rhs)
{
    lhs->push_front(rhs);
}

/**
  * The string_list_prepend_list function is used to add all the strings
  * of another string list to the start of a string list.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string list to be added to the front of lhs.
  */
inline DEPRECATED void
string_list_prepend_list(string_list_ty *lhs, const string_list_ty *rhs)
{
    lhs->push_front(*rhs);
}

/**
  * The string_list_append function is used to add another string to
  * the end of a string list.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string to be added to the end of lhs.
  */
inline DEPRECATED void
string_list_append(string_list_ty *lhs, string_ty *rhs)
{
    lhs->push_back(rhs);
}

/**
  * The string_list_append_list function is used to add all the strings
  * of another string list to the end of a string list.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string list to be added to the end of lhs.
  */
inline DEPRECATED void
string_list_append_list(string_list_ty *lhs, const string_list_ty *rhs)
{
    lhs->push_back(*rhs);
}

/**
  * The string_list_append_unique function is used to add another string
  * to the end of a string list, provided it is not already present.
  * This is an O(n) operation.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string to be added to the end of lhs.
  */
inline DEPRECATED void
string_list_append_unique(string_list_ty *lhs, string_ty *rhs)
{
    lhs->push_back_unique(rhs);
}

/**
  * The string_list_append_list_unique function is used to add all the
  * strings of another string list to the end of a string list, provided
  * they are not already present.  This is an O(n**2) operation.
  *
  * @param lhs
  *	The string list to be added to.
  * @param rhs
  *	The string list to be added to the end of lhs.
  */
inline DEPRECATED void
string_list_append_list_unique(string_list_ty *lhs, const string_list_ty *rhs)
{
    lhs->push_back_unique(*rhs);
}

/**
  * The string_list_copy function is sued to replace the contents of one
  * string list with another.  This is more like a copy constructor than
  * an assignment operator.  The previous contents will be discarded
  * NOT free()ed.
  */
inline DEPRECATED void
string_list_copy(string_list_ty *lhs, const string_list_ty *rhs)
{
    *lhs = *rhs;
}

/**
  * The string_list_remove function is used to remove a speific valued
  * string from a string list.  If there are no occurrences, it is not
  * an error.  If there is more than one occurrece, only the first will
  * be removed.  This is an O(n) operation.
  */
inline DEPRECATED void
string_list_remove(string_list_ty *slp, string_ty *s)
{
    slp->remove(s);
}

/**
  * The string_list_remove_list function is used to remove all instances
  * of one string list (rhs) from another string list (lhs).
  *
  * @param lhs
  *	The string list to have element removed.
  * @param rhs
  *	The list of values to be removed from lhs.
  */
inline DEPRECATED void
string_list_remove_list(string_list_ty *lhs, const string_list_ty *rhs)
{
    lhs->remove(*rhs);
}

/**
  * The string_list_destructor function is used to release all resources
  * held by a string list.  This function shall be called when you are
  * finished with a string list.
  */
inline DEPRECATED void
string_list_destructor(string_list_ty *slp)
{
    // from context, this is the right thing to do,
    // now that the real destructor does something useful.
    slp->clear();
}

/**
  * The string_list_constructor function is used to prepare a string
  * list for use.  (Note: strings lists which are global variables do
  * NOT need this function called before use).
  */
inline DEPRECATED void
string_list_constructor(string_list_ty *slp)
{
    // from context, this is the right thing to do,
    // now that the real constructor does something useful.
    slp->clear();
}

/**
  * The quality (==) operator is used to determine if two string lists
  * contain the same strings.  The _ordering_ of the strings is not
  * considered.  This is an O(n**2) operation.
  *
  * @return
  *	Returns true if the two sets of strings are the same,
  *	false if they are not.
  */
inline bool
operator==(const string_list_ty &lhs, const string_list_ty &rhs)
{
    return lhs.equal(rhs);
}


/**
  * The inquality (!=) operator is used to determine if two string lists
  * do not contain the same strings.  The _ordering_ of the strings is
  * not considered.  This is an O(n**2) operation.
  *
  * @return
  *     Returns true if the two sets of strings are different, false if
  *     they are the same.
  */
inline bool
operator!=(const string_list_ty &lhs, const string_list_ty &rhs)
{
    return !lhs.equal(rhs);
}

/**
  * The string_list_subset function is used to determine if lhs is an
  * (improper) subset of rhs.
  *
  * @return
  *	Returns true (non-zero) if the two sets of strings are the same,
  *	false (zero) if they are not.
  */
inline DEPRECATED int
string_list_subset(const string_list_ty *lhs, const string_list_ty *rhs)
{
    return lhs->subset(*rhs);
}

/**
  * The string_list_sort function is used to sort a list of strings.
  * The comparison function is strcmp(3).  This is an O(n log n)
  * operation.
  */
inline DEPRECATED void
string_list_sort(string_list_ty *slp)
{
    slp->sort();
}

/**
  * The string_list_quote_shell function is used to shell quote each
  * string in a list of strings.  The quoting is done by using the
  * str_quote_shell() function.
  *
  * @param lhs
  *	The place to put this list of quoted strings.
  * @param rhs
  *	The unquoted input strings.
  */
inline DEPRECATED void
string_list_quote_shell(string_list_ty *lhs, const string_list_ty *rhs)
{
    *lhs = rhs->quote_shell();
}

/**
  * The in situ addition operator is used to union values into a set of
  * strings.  Duplicates on the right will be omitted.
  */
inline void
operator+=(string_list_ty &lhs, const string_list_ty &rhs)
{
    lhs.push_back_unique(rhs);
}

/**
  * The addition operator is used to union two sets of strings.
  * Duplicates will be omitted.
  */
inline string_list_ty
operator+(const string_list_ty &lhs, const string_list_ty &rhs)
{
    string_list_ty result;
    result.push_back_unique(lhs);
    result.push_back_unique(rhs);
    return result;
}

/**
  * The in situ subtract operator is used to difference two sets of
  * strings by removing strings from the left which appear in the right.
  */
inline void
operator-=(string_list_ty &lhs, const string_list_ty &rhs)
{
    lhs.remove(rhs);
}

/**
  * The subtract operator is used to create a new set of strings
  * by removing strings from the left which appear in the right.
  */
inline string_list_ty
operator-(const string_list_ty &lhs, const string_list_ty &rhs)
{
    string_list_ty result;
    result.push_back_unique(lhs);
    result.remove(rhs);
    return result;
}

/**
  * The in situ multiply operator is used to keep only those strings on
  * the left which also appear on the right.  Duplicates are omitted.
  */
inline void
operator*=(string_list_ty &lhs, const string_list_ty &rhs)
{
    lhs = lhs.intersection(rhs);
}

/**
  * The multiply operator is used to calculate the set intersection of
  * the left and right.  Duplicates are omitted.
  */
inline string_list_ty
operator*(const string_list_ty &lhs, const string_list_ty &rhs)
{
    return lhs.intersection(rhs);
}

/** @} */
#endif // COMMON_STR_LIST_H
