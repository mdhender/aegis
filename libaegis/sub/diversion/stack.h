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

#ifndef LIBAEGIS_SUB_DIVERSION_STACK_H
#define LIBAEGIS_SUB_DIVERSION_STACK_H

#include <common/ac/stddef.h>
#include <libaegis/sub/diversion.h>

/**
  * The sub_diversion_stack class is used to represent a push-down stack
  * of substitution diversions.
  */
class sub_diversion_stack
{
public:
    /**
      * The destructor.
      */
    virtual ~sub_diversion_stack();

    /**
      * The default constructor.
      */
    sub_diversion_stack();

    /**
      * The push_back method is sued to push another string onto the
      * context stack.
      *
      * \param s
      *     The wide string to be pushed onto the diversion stack.
      * \param resub
      *     Whether or nmot to resubstitute tinto the text of this diversion.
      */
    void push_back(const wstring &s, bool resub);

    /**
      * The pop_back method is used to discard to top element of the
      * stack.
      */
    void pop_back();

    /**
      * The resub moth method is used to determine whether ot not to
      * resubstitute into the text of the diversion on the top of the
      * stack.
      */
    bool resub_both() const;

    /**
      * The getch method is used to get the next character from the
      * top-most diversion on the stack.
      */
    wchar_t getch();

    /**
      * The ungetch method is used to return a character to the top-most
      * diversion on the stack.
      */
    void ungetch(wchar_t c);

    /**
      * The empty method is used to determine whether or not the
      * diversion stack is empty.
      */
    bool empty() const { return (top == 0); }

private:
    /**
      * The top instance variable is used to remember the height of the
      * diversion stack.
      */
    size_t top;

    /**
      * The max instance variable is used to remember the allocated size
      * of the array holding the diversion stack.
      */
    size_t max;

    /**
      * The stack instance variable is used to remember a pointer to
      * the base of the dynamically allocated array representing the
      * diversioon stack.
      */
    sub_diversion *stack;

    /**
      * The copy constructor.  Do not use.
      */
    sub_diversion_stack(const sub_diversion_stack &);

    /**
      * The assignment operator.  Do not use.
      */
    sub_diversion_stack &operator=(const sub_diversion_stack &);
};

#endif // LIBAEGIS_SUB_DIVERSION_STACK_H
