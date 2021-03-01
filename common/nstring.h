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

#ifndef COMMON_NSTRING_H
#define COMMON_NSTRING_H

#include <common/str.h>

class nstring_list; // forward

/**
  * The nstring class is used to represent a reference counted narrow string
  * with fast equality comparison.
  *
  * This is a narrow string class, using ordinary char as the internal
  * character type.  For wide strings, which use wchar_t as the internal
  * character type, see the wstring class.
  */
class nstring
{
public:
    /**
      * The destructor.
      *
      * This destructor is <b>not</b> virtual, do not derive from this class.
      */
    ~nstring()
    {
	str_free(ref);
	ref = 0;
    }

    /**
      * The default constructor.
      */
    nstring() :
	ref(get_empty_ref())
    {
    }

    /**
      * The constructor.
      */
    nstring(const char *arg) :
	ref(arg ? str_from_c(arg) : get_empty_ref())
    {
    }

    /**
      * The constructor.
      */
    nstring(const char *data, size_t len) :
	ref(str_n_from_c(data, len))
    {
    }

    /**
      * The constructor.
      */
    explicit
    nstring(string_ty *arg) :
	ref(arg ? str_copy(arg) : get_empty_ref())
    {
    }

    /**
      * The copy constructor.
      */
    nstring(const nstring &arg) :
	ref(str_copy(arg.ref))
    {
    }

    /**
      * The assignment operator.
      */
    nstring &
    operator=(const nstring &arg)
    {
	if (this != &arg)
	{
	    str_free(ref);
	    ref = (arg.ref ? str_copy(arg.ref) : get_empty_ref());
	}
	return *this;
    }

    /**
      * The c_str method is used to obtain a pointer to the underlying C
      * string (guaranteed to the NUL terminated).
      */
    const char *
    c_str()
	const
    {
	return ref->str_text;
    }

    bool
    empty()
	const
    {
	return (ref->str_length == 0);
    }

    size_t
    size()
	const
    {
	return ref->str_length;
    }

    size_t
    length()
	const
    {
	return ref->str_length;
    }

    /**
      * \brief
      *	join two strings together
      *
      * The str_catenate function is used to join two strings togther to
      * form a new string.  The are joined in the order given.
      *
      * \param arg
      *	    A string to be joined.  Will not be modified.
      *
      * \return
      *     a pointer to a string in dynamic memory.  Use str_free()
      *     when finished with.  The contents of the structure pointed
      *     to <b>shall not</b> be altered.
      */
    nstring
    catenate(const nstring &arg)
	const
    {
        string_ty *tmp = str_catenate(ref, arg.ref);
        nstring result(tmp);
        str_free(tmp);
        return result;
    }

    nstring
    operator+(const nstring &arg)
	const
    {
        string_ty *tmp = str_catenate(ref, arg.ref);
        nstring result(tmp);
        str_free(tmp);
        return result;
    }

    nstring &
    operator+=(const nstring &arg)
    {
	if (!arg.empty())
	{
	    string_ty *s = str_catenate(ref, arg.ref);
	    str_free(ref);
	    ref = s;
	}
	return *this;
    }

    /**
      * \brief
      *	joing strings together
      *
      * The str_cat_three function is used to join three strings together
      * to form a new string.  The are joined in the order given.
      *
      * \param str2
      *	    A string to be joined.  Will not be modified.
      * \param str3
      *	    A string to be joined.  Will not be modified.
      *
      * \return
      *	    a pointer to a string in dynamic memory.  Use str_free() when
      *     finished with.  The contents of the structure pointed to
      *     <b>shall not</b> be altered.
      */
    nstring cat_three(const nstring &str2, const nstring &str3) const;

    /**
      * \brief
      *	test a boolean
      *
      * The str_bool function is used to test the value of a string, as if
      * it contained a number.  If it doesn't contain a number, it is as if
      * the strings was "1".
      *
      * \return
      *     False if the numeric value in the strings was zero, or the
      *     empty string.  True if the numeric value in the string was
      *     non-zero, or the string was non-numeric.
      */
    operator bool() const;

    /**
      * The logical netation operator.
      * Returns the negation of the bool() operator.
      */
    bool operator!() const;

    /**
      * \brief
      *	convert to upper case
      *
      * The str_upcase function is used to create a new string where the
      * lower case characters in the input string are converted to upper
      * case.
      *
      * \return
      *     a pointer to a string in dynamic memory.  Use str_free()
      *     when finished with.  The contents of the structure pointed
      *     to <b>shall not</b> be altered.
      */
    nstring upcase() const;

    /**
      * \brief
      *	convert to lower case
      *
      * The str_downcase function is used to create a new string where the
      * upper case characters in the input string are converted to lower
      * case.
      *
      * \return
      *     a pointer to a string in dynamic memory.  Use str_free()
      *     when finished with.  The contents of the structure pointed
      *     to <b>shall not</b> be altered.
      */
    nstring downcase() const;

    /**
      * \brief
      *	convert to title case
      *
      * The str_capitalize function is used to create a new string where the
      * first letter or each word of the inopuyt string are upper case, and
      * the remaining letters in each word are lower case.  (Sometimes called
      * Title Case.)
      *
      * \returns
      *	    a pointer to a string in dynamic memory.
      */
    nstring capitalize() const;

    /**
      * \brief
      *	extract a field
      *
      * The str_field function is used to extract the \a nth field, where
      * each field is separated by the \a sep string.
      *
      * \param sep
      *	    The string which separates each field.
      * \param nth
      *	    The number of the field to be extracted.  Zero based.
      *	    If too high, the emtry string is returned.
      *
      * \return
      *	    a pointer to a string in dynamic memory.  Use str_free() when
      *     finished with.  The contents of the structure pointed to
      *     <b>shall not</b> be altered.
      */
    nstring field(char sep, int nth) const;

    /**
      * \brief
      *	format text
      *
      * The str_format function is used to create a new string by interpreting
      * the \a fmt string.  All formats understood by the ANSI C printf(3)
      * are understood by this function (but probably not your favorite
      * proprietary extension).  In addition the '%S' specifier expects a <i>
      * string_ty * </i> argument.
      *
      * \param fmt
      *	    The format string to be interpreted when constructing the
      *	    return value.
      *
      * \return
      *     a pointer to a string in dynamic memory.  Use str_free()
      *     when finished with.  The contents of the structure pointed
      *     to <b>shall not</b> be altered.
      */
    static nstring format(const char *fmt, ...)               ATTR_PRINTF(1, 2);

    /**
      * \brief
      *	format text
      *
      * The str_vformat function is used to create a new string by
      * interpreting the \a fmt string.  All formats understood by the
      * ANSI C printf(3) are understood by this function (but probably
      * not your favorite proprietary extension).  In addition the '%S'
      * specifier expects a <i>string_ty *</i> argument.
      *
      * \param fmt
      *	    The format string to be interpreted when constructing the
      *	    return value.
      * \param ap
      *	    Where to obtain additional arguments required by the \a fmt string.
      *
      * \return
      *     a pointer to a string in dynamic memory.  Use str_free()
      *     when finished with.  The contents of the structure pointed
      *     to <b>shall not</b> be altered.
      */
    static nstring vformat(const char *fmt, va_list ap);

    /**
      * \brief
      *	test string equality
      *
      * The str_equal function is used to test to see if two strings are
      * exactly the same.
      *
      * \param arg
      *	    A string to be compared.  Will not be modified.
      *
      * \note
      *     Users shall always write code as if they did not know that a
      *     string equality test is a pointer equality test.
      *
      * \return
      *	    Non-zero if the strings are equal,
      *	    zero if the strings are unequal.
      */
    bool
    equal(const nstring &arg)
	const
    {
	return (ref == arg.ref);
    }

    /**
      * The equal-to operator.
      *
      * @param rhs
      *     The right hans side of the comparison.
      */
    bool
    operator==(const nstring &arg)
	const
    {
	return (ref == arg.ref);
    }

    /**
      * The not-equal-to operator.
      *
      * @param rhs
      *     The right hans side of the comparison.
      */
    bool
    operator!=(const nstring &arg)
	const
    {
	return (ref != arg.ref);
    }

    /**
      * The less-than operator.
      *
      * @param rhs
      *     The right hans side of the comparison.
      */
    bool operator<(const nstring &arg) const;

    /**
      * The less-than-or-equal operator.
      *
      * @param rhs
      *     The right hans side of the comparison.
      */
    bool operator<=(const nstring &arg) const;

    /**
      * The greater-than operator.
      *
      * @param rhs
      *     The right hans side of the comparison.
      */
    bool operator>(const nstring &arg) const;

    /**
      * The greater-than-or-equal operator.
      *
      * @param rhs
      *     The right hans side of the comparison.
      */
    bool operator>=(const nstring &arg) const;

    /**
      * \brief
      *	quote C meta-characters
      *
      * The quote_c method is used to create a new string which
      * quotes the C meta-characters in the input string.
      */
    nstring quote_c() const;

    /**
      * \brief
      *	quote shell meta-characters
      *
      * The str_quote_shell function is used to create a new string which
      * quotes the shell meta-characters in the input string.
      *
      * \return
      *     a pointer to a string in dynamic memory.  Use str_free()
      *     when finished with.  The contents of the structure pointed
      *     to <b>shall not</b> be altered.
      */
    nstring quote_shell() const;

    /**
      * \brief
      *    remove excess white space
      *
      * The trim method is used to remove white space from the beginning
      * and end of the string, and replace all other runs of one or more
      * white space characters with a single space.
      *
      * \return
      *     another string
      */
    nstring trim() const;

    /**
      * \brief
      *    remove excess white space
      *
      * The trim_lines method is used to remove white space from the
      * beginning and end of <i>lines</i> within the string, and replace
      * all other runs of one or more white space characters with a
      * single space.
      *
      * \return
      *     another string
      */
    nstring trim_lines() const;

    /**
      * The trim_extension method is used to build a new string without
      * the file extension.  For example, the string "a/b.c" will return
      * "a/b".
      */
    nstring trim_extension() const;

    /**
      * The get_extension method is used to build a new string
      * containing the file extension, if any, without the dot.  For
      * example, the string "a/b.c" will return "c".
      */
    nstring get_extension() const;

    /**
      * \brief
      *    remove excess white space
      *
      * The snip method is used to remove white space from the beginning
      * and end of the string.  Interior white space is left unchanged.
      *
      * \return
      *     another string
      */
    nstring snip() const;

    /**
      * \brief
      *	check is valid
      *
      * The str_validate function is used to confirm that the given
      * string pointer, \a str, points to a valid string.  Usually used
      * for debugging, often in assert()s.
      *
      * \return
      *	    Non-zero if valid, zero if invalid.
      */
    bool
    valid()
	const
    {
	return str_validate(ref);
    }

    /**
      * The get_ref method is used to extract the reference to the
      * underlying reference counted string object.  If the ref pointer
      * is NULL a pointer to a common empty string object is returned.
      *
      * Once the sources cease using string_ty directly, this method
      * will become private.
      */
    string_ty *
    get_ref()
	const
    {
	return ref;
    }

    /**
      * The starts_with method is ised to test whether this string
      * starts with the given prefix.
      *
      * @param prefix
      *     The string to test for.
      */
    bool starts_with(const nstring &prefix) const;

    /**
      * The ends_with method is ised to test whether this string
      * ends with the given suffix.
      *
      * @param suffix
      *     The string to test for.
      */
    bool ends_with(const nstring &suffix) const;

    /**
      * The ends_with_nocase method is ised to test whether this string
      * ends with the given suffix.  The comparison will be case
      * insensitive.
      *
      * @param suffix
      *     The string to test for.
      */
    bool ends_with_nocase(const nstring &suffix) const;

    /**
      * The gmatch function is used to match the string against a file
      * globbing pattern.
      *
      * @pattern
      *     The pattern to try against the string.
      * @returns
      *     bool; true if matches pattern, false if does not.
      */
    bool gmatch(const char *pattern) const;

    /**
     * The gmatch function is used to match the string against a file
     * globbing pattern.
     *
     * @pattern
     *     The pattern to try against the string.
     * @returns
     *     bool; true if matches pattern, false if does not.
     */
    bool gmatch(const nstring &pattern) const;

    /**
     * The gmatch function is used to match the string against a set of
     * file globbing patterns.
     *
     * @patterns
     *     The patterns to try against the string.
     * @returns
     *     bool; true if matches at least one pattern, false if does
     *     not match any pattern.
     */
    bool gmatch(const nstring_list &pattern) const;

     /**
      * The identifier method is used to convert all non-C-identifier
      * characters in the string to underscores.  The intention is to
      * create a valid C identifier from the string.
      */
    nstring identifier() const;

    /**
      * The replace method may be used to alter a string by replacing
      * one constant substring with another.
      *
      * @note
      *     The replacement is <b>not</b> done <i>in situ</i>.  The original
      *     string is unaltered.
      *
      * @param lhs
      *     The substring to look for.
      * @param rhs
      *     The substring to replace \a lhs if found.
      * @param maximum
      *     The maximum number of times to perform the replacement.
      *     Defaults to "infinity".
      * @returns
      *     A new string with the replacements made.
      */
    nstring replace(const nstring &lhs, const nstring &rhs, int maximum = -1)
	const;

    /**
      * The indexing operator is used to extract the nth character of a
      * string.  Indexes out of range will result in the NUL character
      * ('\0') being returned.
      *
      * @param n
      *     The character to extract.  Zero based.
      * @returns
      *     The character requested, or NUL ('\0') if the index is out
      *     of range.
      */
    char
    operator[](size_t n)
	const
    {
	return (n < size() ? ref->str_text[n] : '\0');
    }

    /**
      * The clear method is used to delete to contents of the string,
      * and replace it with the empty string/
      */
    void clear();

    /**
      * The url_quote mwthod is used to form a string suitable for use
      * within an HTML href="" string, or similar.  This means that
      * special characters and unprintable characters are replaced with
      * %NN escape sequences.
      */
    nstring url_quote() const;

    /**
      * The url_unquote method is used to form a normal string given an
      * HTML href="" string, or similar.  This means that %NN escape
      * sequences are replaced with single bytes.
      */
    nstring url_unquote() const;

    /**
      * The html_quote method is used to form a string suitable for use
      * withing an HTML paragraph.  This means that special characters
      * and unprintable characters are replaced with &#DDD; escape
      * sequences.  Some special characters are also replaced, e.g. &lt;
      *
      * \param para
      *     Whether or not to translate \n\n and \n as <p> and <br>.
      *     Defaults to false (no translation).
      */
    nstring html_quote(bool para = false) const;

    /**
      * The html_unquote method is used to form a normal string given
      * text from an HTML paragraph.  This means that &#DDD; escape
      * sequences are replaced by single bytes.  Some special characters
      * are also replaced, e.g. &lt;
      *
      * \note
      *     unicode values (DDD > 255) are not understood.
     */
    nstring html_unquote() const;

    /**
      * The to_long method attempts to turn a string into a long value.
      * It returns zero on failure.
      */
    long to_long() const;

    /**
      * The substring method may be used to extract a substring from this
      * string.
      *
      * @param start
      *     The offset into the string where the substring starts.  If
      *     negative, is measured from the end.
      * @param nbytes
      *     The number of bytes to extract, if that many available.  If
      *     negative, measured to the left (text not reversed).
      * @returns
      *     a string, note that it could be less than nbytes long.
      */
    nstring substring(long start, long nbytes) const;

    /**
     * The dirname method is used to extract the directory part of a
     * filename.  If there is no directory part, "." is returned.
     */
    nstring dirname() const;

    /**
      * The first_dirname method is used to extract the first directory
      * part of a filename.  If there is no directory part, "." is
      * returned.
      */
    nstring first_dirname() const;

    /**
     * The basename method is used to generate a new string from
     * this one with any leading directory components removed.  If
     * specified, also remove a trailing suffix.
     */
    nstring basename(const nstring &suffix = "") const;

    /**
      * The get_hash method is used to access the has value of this
      * key.  While this is an internal implementation feature, it is
      * often extremely handy to have a pre-calculated hash value for a
      * string, and so it is made public.
      */
    str_hash_ty get_hash() const { return ref->str_hash; }

private:
    /**
      * The ref instance variable is used to remember the location of
      * the object common to all of the references.
      * The is <i>never</i> the NULL pointer.
      */
    string_ty *ref;

    /**
      * The get_empty_ref() class method is used to get a
      * pointer to an underlying string object of length zero.
      */
    static string_ty *get_empty_ref();
};

inline nstring
operator+(const char *lhs, const nstring &rhs)
{
    return nstring(lhs).catenate(rhs);
}

inline nstring
operator+(const nstring &lhs, const char *rhs)
{
    return lhs.catenate(nstring(rhs));
}

#endif // COMMON_NSTRING_H
