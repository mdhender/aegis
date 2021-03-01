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

#ifndef LIBAEGIS_SUB_DIVERSION_H
#define LIBAEGIS_SUB_DIVERSION_H

#include <common/ac/stddef.h>

#include <common/wstring.h>

/**
  * The sub_diversion class is used to represent the state of a
  * substitution diversion.
  */
class sub_diversion
{
public:
    /**
      * The destructor.
      */
    virtual ~sub_diversion();

    /**
      * The default constructor.
      */
    sub_diversion();

    /**
      * The constructor.
      *
      * \param arg1
      *     The text of this diversion.
      * \param arg2
      *     Wether or not to rescan the text of this diversion for more
      *     substitutions.
      */
    sub_diversion(const wstring &arg1, bool arg2 = false);

    /**
      * The copy constructor.
      */
    sub_diversion(const sub_diversion &);

    /**
      * The assignment operator.
      */
    sub_diversion &operator=(const sub_diversion &);

    bool resub_both() const { return resubstitute; }

    /**
      * The getch method is used to get the next character from the diversion.
      *
      * \returns
      *     wchar_t; the next characters of the diversion text, or a
      *     wide NUL character if there is not more text to read from
      *     this diversion.
      */
    wchar_t getch();

    /**
      * The ungetch method is used to return a chaaracter to the
      * diversion, fo re-reading at a later time.
      *
      * \note
      *     Only call ungetch with characters returned by getch, or the
      *     behaviour is undefinbed.
      */
    void ungetch(wchar_t wc);

private:
    /**
      * The pos instance variable is used to remember where in the
      * "text" the read point is up to.
      */
    size_t pos;

    /**
      * The text instance variable is used to remember text of this
      * diversion.
      */
    wstring text;

    /**
      * The resubstitute instance variable is used to remember whether
      * or not this diversion is to be reinterpreted by the scanner.
      */
    bool resubstitute;
};

#endif // LIBAEGIS_SUB_DIVERSION_H
// vim: set ts=8 sw=4 et :
