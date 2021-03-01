//
//      aegis - project change supervisor
//      Copyright (C) 2004-2006, 2008, 2009, 2012 Peter Miller
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

#ifndef COMMON_REGULA_EXPRE_H
#define COMMON_REGULA_EXPRE_H

#include <common/ac/regex.h>
#include <common/nstring.h>

/**
  * The regular_expression class is used to represent the state of a
  * regular expression match.  All of the interfacing with the regex
  * code is taken care of by this class.
  *
  * You can match the one pattern against numerous strings by making
  * repeated calls to the match() method.
  *
  * You can match against a string, and replace it with something else,
  * by using the match_and_substitute method.  It, too, may be called
  * more than once.
  */
class regular_expression
{
public:
    /**
      * The destructor.
      */
    virtual ~regular_expression();

    /**
      * The constructor.
      *
      * @param pattern
      *     The pattern to be matched against.
      * @param icase
      *     true for case-insensitive match, the default is case sensitive
      * @note
      *     The compilation of the pattern will be deferred until it is
      *     actually used by the "match" method.
      */
    regular_expression(const nstring &pattern, bool icase = false);

    /**
      * The matches method is used to match a string against the pattern
      * given to the constructor.
      *
      * @param text
      *     The string to be matched against the pattern.
      * @param so
      *     The start offset of the match (on success)
      * @param eo
      *     The end offset of the match (on success)
      * @returns
      *     true if there was a match, or false if there was no match or
      *     an error occurred.
      */
    bool matches(const char *text, size_t &so, size_t &eo);

    /**
      * The match method is used to match a string against the pattern
      * given to the constructor.
      *
      * \param arg
      *     The string to be matched against the pattern.
      * \param offset
      *     The point at which to start searching in the string; defaults
      *     to zero, meaning the start of the string.
      * \returns
      *     true if there was a match, or false if there was no match or
      *     an error occurred.
      */
    bool match(const nstring &arg, size_t offset = 0);

    /**
      * Obtain an human readable representation of the most recent error,
      * or NULL if there has been no error.
      */
    const char *strerror() const;

    /**
      * The match_and_substitute method is used to match the left hand
      * side of a regular expression and substitute the right hand side
      * if there was a match
      *
      * \param rhs
      *     The right-hand-side pattern for substituting.
      * \param actual
      *     The text to be matched, possibly more than once.
      * \param how_many_times
      *     The maximum number of times to perform the match;
      *     zero means unlimited.
      * \param output
      *     The result of the substitution is placed here.
      * \returns
      *     true of there were no errors, or false if there was an error.
      */
    bool match_and_substitute(const nstring &rhs, const nstring &actual,
        long how_many_times, nstring &output);

private:
    /**
      * The errstr instance variable is used to remember the human
      * readable text of the last error which ocurred.
      */
    nstring errstr;

    /**
      * The set_error method is used to obtain the text of the error
      * for the last regex function call, and set the errstr instance
      * variable accordingly.
      *
      * \param err
      *     The error code returned by the last regex call.
      */
    void set_error(int err);

    /**
      * The lhs instance variable is used to remember the pattern on the
      * left hand side to be matched against.
      */
    nstring lhs;

    /**
      * The icase instance variable is sud to remeber whether we are doing a
      * case-sensitive match (false) or a case-IN-sensitive match (true).
      */
    bool icase;

    /**
      * The compile method is used to compile the left and side of the
      * regular expression match.  Compilation is deferred until the
      * regular expression is actually used.
      */
    bool compile();

    /**
      * The compiled instance variable is used to remember whether or
      * not the left hand side has been compiled yet.
      */
    bool compiled;

    /**
      * The preg instance variable is an opaque storage area used by
      * the regex functions.  It is initialised by the compile() method.
      */
    regex_t preg;

    /**
      * The regmatch instance variable is used to remember the matching
      * postions from the last match() method call.
      */
    regmatch_t regmatch[10];

    /**
      * The default constructor.  Do not use.
      */
    regular_expression();

    /**
      * The copy constructor.  Do not use.
      */
    regular_expression(const regular_expression &);

    /**
      * The assignment operator.  Do not use.
      */
    regular_expression &operator=(const regular_expression &);
};

#endif // COMMON_REGULA_EXPRE_H
// vim: set ts=8 sw=4 et :
