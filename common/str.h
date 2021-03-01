//
// aegis - project change supervisor
// Copyright (C) 1991-1995, 1998, 1999, 2001-2006, 2008, 2012 Peter Miller
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

#ifndef STR_H
#define STR_H

/** @addtogroup String
  * @brief String manipulation funtions
  * @ingroup Common
  * @{
  */
#include <common/ac/stddef.h>
#include <common/ac/stdarg.h>
#include <common/gcc-attribute.h>

typedef unsigned long str_hash_ty;

/**
  * @struct string_ty
  * The string_ty struct is used to remember information about strings.
  * Users should always refer to strings as @em pointers.  The only
  * members within the struct that should be accessed are the str_text
  * member, which is a NUL terminated array of characters, and the
  * str_length member, which contains the length of the string's text
  * (not including the NUL terminator).
  *
  * It is guaranteed that all equal strings will not only have the same
  * hash, but will, if fact, be exactly the same sting.  The reference
  * count is how many strings currently exist with this value.  Thus,
  * the commonest string test, a string equality test becomes a pointer
  * equality test.
  *
  * <b>Thou shalt not</b> modify any member of a string struct, or the
  * forces of evil shall wreak misery in thy life.
  */
struct string_ty
{
    /**
      * The hash of the string.  Used internally by the string table,
      * users shall never access this member.
      */
    str_hash_ty str_hash;

    /**
      * The next string in a hash bucket.  Used internally by the string
      * table, users shall never access this member.
      */
    string_ty *str_next;

    /**
      * The reference count for this string.  It is guaranteed that
      * all equal strings will not only have the same hash, but will,
      * if fact, be exactly the same sting.  The reference count is how
      * many strings currently exist with this value.  Used internally
      * by the string table, users shall never access this member.
      */
    long str_references;

    /**
      * The length of the string (not including the terminating NUL).
      * Read-only access permitted to users.
      */
    size_t str_length;

    /**
      * The text value of the string.  It's actually longer than its
      * declaration when the string length is more than zero.
      * Read-only access permitted to users.
      */
    char str_text[1];
};

void str_release(void);

/**
  * @brief
  *     make string from C string
  *
  * The str_from_c function is used to make a string from a null
  * terminated C string.
  *
  * @param str
  *     The C string to be copied.  Will not be modified.
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_from_c(const char *str);

/**
  * @brief
  *     make string from C string
  *
  * The str_n_from_c function is used to make a string from an array of
  * characters.  No null terminator is assumed.
  *
  * @param str
  *     The C string to be copied.  Will not be modified.
  * @param len
  *     The maximum number of characters to be used (fewer will be used
  *     if there is an included NUL).
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_n_from_c(const char *str, size_t len);

/**
  * @brief
  *     make a copy of a string
  *
  * The str_copy function is used to make a copy of a string.
  *
  * @param str
  *     The string to be copied.  Will not be modified.
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_copy(string_ty *str);

/**
  * @brief
  *     release a string
  *
  * The str_free function is used to indicate that a string hash
  * been finished with.  This is the only way to release strings.
  * <b>Do not</b> use the free() function.
  *
  * @param str
  *     The string to be freed.
  *
  * @return
  *      void
  */
void str_free(string_ty *str);

/**
  * @brief
  *     join two strings together
  *
  * The str_catenate function is used to join two strings togther to
  * form a new string.  The are joined in the order given.
  *
  * @param str1
  *     A string to be joined.  Will not be modified.
  * @param str2
  *     A string to be joined.  Will not be modified.
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_catenate(string_ty *str1, string_ty *str2);

/**
  * @brief
  *     joing strings together
  *
  * The str_cat_three function is used to join three strings together
  * to form a new string.  The are joined in the order given.
  *
  * @param str1
  *     A string to be joined.  Will not be modified.
  * @param str2
  *     A string to be joined.  Will not be modified.
  * @param str3
  *     A string to be joined.  Will not be modified.
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_cat_three(string_ty *str1, string_ty *str2, string_ty *str3);

/**
  * @brief
  *     test a boolean
  *
  * The str_bool function is used to test the value of a string, as if
  * it contained a number.  If it doesn't contain a number, it is as if
  * the strings was "1".
  *
  * @param str
  *     The string to be tested.  Will not be modified.
  *
  * @return
  *     Zero if the numeric value in the strings was zero, or the empty
  *     string.  One if the numeric value in the string was non-zero,
  *     or the string was non-numeric.
  */
int str_bool(string_ty *str);

/**
  * @brief
  *     convert to upper case
  *
  * The str_upcase function is used to create a new string where the
  * lower case characters in the input string are converted to upper
  * case.
  *
  * @param str
  *     The string to be converted.  Will not be modified (the operation
  *     is <b>not</b> performed <i>in situ</i>).
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_upcase(string_ty *str);

/**
  * @brief
  *     convert to lower case
  *
  * The str_downcase function is used to create a new string where the
  * upper case characters in the input string are converted to lower
  * case.
  *
  * @param str
  *     The string to be converted.  Will not be modified (the operation
  *     is <b>not</b> performed <i>in situ</i>).
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_downcase(string_ty *str);

/**
  * @brief
  *     convert to title case
  *
  * The str_capitalize function is used to create a new string where the
  * first letter or each word of the inopuyt string are upper case, and
  * the remaining letters in each word are lower case.  (Sometimes called
  * Title Case.)
  *
  * @param str
  *     The string to be converted.  Will not be modified (the operation
  *     is <b>not</b> performed <i>in situ</i>).
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_capitalize(string_ty *str);

/**
  * @brief
  *     dump the string table
  *
  * The str_dum function is used to dump the contents of the string
  * table to the standard error.  Only useful for debugging.
  *
  * @return
  *     void
  */
void str_dump(void);

/**
  * @brief
  *     extract a field
  *
  * The str_field function is used to extract the @a nth field, where
  * each field is separated by the @a sep string.
  *
  * @param str
  *     The string from which the field is to be extracted.  Will not
  *     be modified (the operation <b>not</b> performed <i>in situ</i>).
  * @param sep
  *     The string which separates each field.
  * @param nth
  *     The number of the field to be extracted.  Zero based.
  *     If too high, the emtry string is returned.
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_field(string_ty *str, int sep, int nth);

/**
  * @brief
  *     convert tables of strings
  *
  * The slow_to_fast function is used to convert tables for normal
  * C strings into tables of reference countest strings.  Use amlost
  * exclusively by the fmtgen-generated sources.
  *
  * @return
  *     void
  */
void slow_to_fast(const char *const *, string_ty **, size_t);

/**
  * @brief
  *     format text
  *
  * The str_format function is used to create a new string by interpreting
  * the @a fmt string.  All formats understood by the ANSI C printf(3)
  * are understodd by this function (but probably not your favorite
  * proprietary extension).  In addition the '%S' specifier expects a <i>
  * string_ty * </i> argument.
  *
  * @param fmt
  *     The format string to be interpreted when constructing the
  *     return value.
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_format(const char *fmt, ...)               ATTR_PRINTF(1, 2);

/**
  * @brief
  *     format text
  *
  * The str_vformat function is used to create a new string by interpreting
  * the @a fmt string.  All formats understood by the ANSI C printf(3)
  * are understodd by this function (but probably not your favorite
  * proprietary extension).  In addition the '%S' specifier expects a <i>
  * string_ty * </i> argument.
  *
  * @param fmt
  *     The format string to be interpreted when constructing the
  *     return value.
  * @param ap
  *     Where to obtain additional arguments required by the @a fmt string.
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_vformat(const char *fmt, va_list ap);

/**
  * @brief
  *     test string equality
  *
  * The str_equal function is used to test to see if two strings are
  * exactly the same.
  *
  * @param str1
  *     A string to be compared.  Will not be modified.
  * @param str2
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
int str_equal(string_ty *str1, string_ty *str2);

/**
  * @brief
  *     test string equality
  * @see
  *     str_equal()
  *
  * The str_equal macro is used to accellerate string equality tests.
  * Users shall always write code as if they did not know that a string
  * equality test is a pointer  equality test.
  */
#define str_equal(s1, s2) ((s1) == (s2))

/**
  * @brief
  *     quote shell meta-characters
  *
  * The str_quote_shell function is used to create a new string which
  * quotes the shell meta-characters in the input string.
  *
  * @param str
  *     The string to be converted.  Will not be modified (the operation
  *     is <b>not</b> performed <i>in situ</i>).
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_quote_shell(string_ty *str);

/**
  * @brief
  *     remove excess white space
  *
  * The str_trim function is used to remove white space from the beginning
  * and end of the string, and replace all other runs of one or more
  * white space characters with a single space.
  *
  * @param str
  *     The string to be converted.  Will not be modified (the operation
  *     is <b>not</b> performed <i>in situ</i>).
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.  The contents of the structure pointed to <b>shall
  *     not</b> be altered.
  */
string_ty *str_trim(string_ty *str);

/**
  * @brief
  *     remove leading and trailing white space
  *
  * The str_snip function is used to remove white space from the beginning
  * and end of the string.  Interior white space is unchanged.
  *
  * @param str
  *     The string to be converted.
  *
  * @return
  *     a pointer to a string in dynamic memory.  Use str_free() when
  *     finished with.
 */
string_ty *str_snip(string_ty *str);

/**
  * @brief
  *     check is valid
  *
  * The str_validate function is used to confirm that the given string
  * pointer, @a str, points to a valid string.
  * Usually used for debugging, often in assert()s.
  *
  * @param str
  *     The string to be validated.  Willnot be modified.
  *
  * @return
  *     Non-zero if valid, zero if invalid.
  */
int str_validate(const string_ty *str);

/**
  * @brief
  *     look for a leading prefix
  *
  * The str_leading_prefix function is used to test whether the @a needle
  * argument is a leading prefix of the @a haystack argument.
  *
  * @param haystack
  *     The large string which allegedly contains the @a needle.
  * @param needle
  *     The substring to be tested for.
  *
  * @return
  *     Non-zero if is a leading prefix, zero if not.
  */
int str_leading_prefix(string_ty *haystack, string_ty *needle);

/**
  * @brief
  *     look for a trailing suffix
  *
  * The str_trailing_suffix function is used to test whether the @a needle
  * argument is a trailing suffix of the @a haystack argument.
  *
  * @param haystack
  *     The large string which allegedly contains the @a needle.
  * @param needle
  *     The substring to be tested for.
  *
  * @return
  *     Non-zero if is a trailing suffix, zero if not.
  */
int str_trailing_suffix(string_ty *haystack, string_ty *needle);

/**
  * The str_identifier function is used to generate another string,
  * replaceing all non-C-identifier characters with underscore.  The
  * intention is to generate a valid C identifier from the string.
  *
  * @param str
  *     The string to be converted.
  */
string_ty *str_identifier(string_ty *str);

/**
  * The str_replace function may be used to alter a string by replacing
  * one constant substring with another.
  *
  * @note
  *     The replacement is <b>not</b> done <i>in situ</i>.  The original
  *     @a str is unaltered.
  *
  * @param str
  *     The string to be altered.
  * @param lhs
  *     The substring to look for within @a str
  * @param rhs
  *     The substring to replace @a lhs if found within @a str
  * @param maximum
  *     The maximum number of times to perform the replacement.
  *     Defaults to "infinity".
  * @returns
  *     A new string with the replacements made.
  */
string_ty *str_replace(string_ty *str, string_ty *lhs, string_ty *rhs,
    int maximum = -1);

/** @} */

#endif // STR_H
// vim: set ts=8 sw=4 et :
