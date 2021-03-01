//
//      aegis - project change supervisor
//      Copyright (C) 2004-2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef COMMON_WSTRING_H
#define COMMON_WSTRING_H

/** @addtogroup WString
  * @brief Wide char strings
  * @ingroup String
  * @{
  */

#include <common/wstr.h>

class nstring; // forward

/**
  * The wstring class is used to represent a reference counted wide string
  * with fast equality comparison.
  *
  * This is a wide string class, using wchar_t as the internal character
  * type.  For ordinary narrow strings, which use char as the internal
  * character type, see the nstring class.
  */
class wstring
{
public:
    /**
      * The destructor.
      *
      * This destructor is <b>not</b> virtual, do not derive from this class.
      */
    ~wstring();

    /**
      * The default constructor.
      */
    wstring() :
        ref(get_empty_ref())
    {
    }

    /**
      * The constructor.
      */
    explicit
    wstring(const char *arg) :
        ref(arg ? wstr_from_c(arg) : get_empty_ref())
    {
    }

    /**
      * The constructor.
      */
    explicit
    wstring(const wchar_t *arg) :
        ref(arg ? wstr_from_wc(arg) : get_empty_ref())
    {
    }

    /**
      * The constructor.
      */
    wstring(const char *data, size_t len) :
        ref(wstr_n_from_c(data, len))
    {
    }

    /**
      * The constructor.
      */
    wstring(const wchar_t *data, size_t len) :
        ref(wstr_n_from_wc(data, len))
    {
    }

    /**
      * The constructor.
      */
    wstring(wstring_ty *arg) :
        ref(arg ? wstr_copy(arg) : get_empty_ref())
    {
    }

    /**
      * The constructor.
      *
      * This will be DEPRECATED as soon as possible.
      */
    wstring(string_ty *arg);

    /**
      * The constructor.
      */
    wstring(const nstring &arg);

    /**
      * The copy constructor.
      */
    wstring(const wstring &arg) :
        ref(wstr_copy(arg.ref))
    {
    }

    /**
      * The assignment operator.
      */
    wstring &
    operator=(const wstring &arg)
    {
        if (this != &arg)
        {
            wstr_free(ref);
            ref = wstr_copy(arg.ref);
        }
        return *this;
    }

    /**
      * The clear method may be used to discard the contents of the
      * string, and replace it with the empty string.
      */
    void clear();

    /**
      * The c_str method is used to obtain a pointer to the underlying C
      * string (guaranteed to the NUL terminated).
      */
    const wchar_t *
    c_str()
        const
    {
        return ref->wstr_text;
    }

    /**
      * The to_nstring method is used to convert a wide string to a
      * narrow string.
      */
    nstring to_nstring() const;

    bool
    empty()
        const
    {
        return (ref->wstr_length == 0);
    }

    size_t
    size()
        const
    {
        return ref->wstr_length;
    }

    size_t
    length()
        const
    {
        return ref->wstr_length;
    }

    /**
      * @brief
      * join two strings together
      *
      * The wstr_catenate function is used to join two strings togther to
      * form a new string.  The are joined in the order given.
      *
      * @param arg
      *     A string to be joined.  Will not be modified.
      *
      * @return
      *     a pointer to a string in dynamic memory.  Use wstr_free()
      *     when finished with.  The contents of the structure pointed
      *     to <b>shall not</b> be altered.
      */
    wstring
    catenate(const wstring &arg)
        const
    {
        return wstring(wstr_catenate(ref, arg.ref));
    }

    wstring
    operator+(const wstring &arg)
        const
    {
        return wstring(wstr_catenate(ref, arg.ref));
    }

    wstring &
    operator+=(const wstring &arg)
    {
        if (!arg.empty())
        {
            wstring_ty *s = wstr_catenate(ref, arg.ref);
            wstr_free(ref);
            ref = s;
        }
        return *this;
    }

    /**
      * @brief
      * joing strings together
      *
      * The wstr_cat_three function is used to join three strings together
      * to form a new string.  The are joined in the order given.
      *
      * @param str2
      *     A string to be joined.  Will not be modified.
      * @param str3
      *     A string to be joined.  Will not be modified.
      *
      * @return
      *     a pointer to a string in dynamic memory.  Use wstr_free() when
      *     finished with.  The contents of the structure pointed to
      *     <b>shall not</b> be altered.
      */
    wstring
    cat_three(const wstring &str2, const wstring &str3)
        const
    {
        return wstring(wstr_cat_three(ref, str2.ref, str3.ref));
    }

    /**
      * @brief
      * convert to upper case
      *
      * The wstr_upcase function is used to create a new string where the
      * lower case characters in the input string are converted to upper
      * case.
      *
      * @return
      *     a pointer to a string in dynamic memory.  Use wstr_free()
      *     when finished with.  The contents of the structure pointed
      *     to <b>shall not</b> be altered.
      */
    wstring
    upcase()
        const
    {
        return wstring(wstr_to_upper(ref));
    }

    /**
      * @brief
      * convert to lower case
      *
      * The wstr_downcase function is used to create a new string where the
      * upper case characters in the input string are converted to lower
      * case.
      *
      * @return
      *     a pointer to a string in dynamic memory.  Use wstr_free()
      *     when finished with.  The contents of the structure pointed
      *     to <b>shall not</b> be altered.
      */
    wstring
    downcase()
        const
    {
        return wstring(wstr_to_lower(ref));
    }

    /**
      * @brief
      * convert to title case
      *
      * The wstr_capitalize function is used to create a new string where the
      * first letter or each word of the inopuyt string are upper case, and
      * the remaining letters in each word are lower case.  (Sometimes called
      * Title Case.)
      *
      * @returns
      *     a pointer to a string in dynamic memory.
      */
    wstring
    capitalize()
        const
    {
        return wstring(wstr_capitalize(ref));
    }

    /**
      * @brief
      * test string equality
      *
      * The wstr_equal function is used to test to see if two strings are
      * exactly the same.
      *
      * @param arg
      *     A string to be compared.  Will not be modified.
      *
      * @note
      *     Users shall always write code as if they did not know that a
      *     string equality test is a pointer equality test.
      *
      * @return
      *     Non-zero if the strings are equal,
      *     zero if the strings are unequal.
      */
    bool
    equal(const wstring &arg)
        const
    {
        return (ref == arg.ref);
    }

    bool
    operator==(const wstring &arg)
        const
    {
        return (ref == arg.ref);
    }

    bool
    operator!=(const wstring &arg)
        const
    {
        return (ref != arg.ref);
    }

    /**
      * The get_ref method is used to extract the reference to the
      * underlying reference counted string object.  If the ref pointer
      * is NULL a pointer to a common empty string object is returned.
      *
      * Once the sources cease using wstring_ty directly, this method
      * will become private.
      */
    wstring_ty *
    get_ref()
        const
    {
        return ref;
    }

    /**
      * The identifier method is used to convert all non-C-identifier
      * characters in the string to underscores.  The intention is to
      * create a valid C identifier from the string.
      */
    wstring
    identifier()
        const
    {
        return wstr_to_ident(get_ref());
    }

    /**
      * The indexing operator is used to extract the nth character of a
      * string.  Indexes out of range will result in the NUL character
      * being returned.
      *
      * @param n
      *     The character to extract.  Zero based.
      * @returns
      *     The character requested, or NUL if the index is out
      *     of range.
      */
    char
    operator[](size_t n)
        const
    {
        return (n < size() ? ref->wstr_text[n] : L'\0');
    }

    /**
      * The column_width method is used to determine how many printing
      * columns wde this character is.  Most characters are one printing
      * column wide, byt Kanji characters (for example0 can be two
      * columns wide.
      */
    int
    column_width()
        const
    {
        return wstr_column_width(ref);
    }

private:
    /**
      * The ref instance variable is used to remember the location of
      * the object common to all of the references.  The is <i>never</i>
      * the NULL pointer.
      */
    wstring_ty *ref;

    /**
      * The get_empty_ref() class method is used to get a pointer to an
      * underlying string object of length zero.
      */
    static wstring_ty *
    get_empty_ref()
    {
        return wstr_from_c("");
    }
};

inline wstring
operator+(const char *lhs, const wstring &rhs)
{
    return wstring(lhs).catenate(rhs);
}

inline wstring
operator+(const wstring &lhs, const char *rhs)
{
    return lhs.catenate(wstring(rhs));
}

/** @} */

#endif // COMMON_WSTRING_H
// vim: set ts=8 sw=4 et :
