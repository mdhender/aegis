//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: interface of the printer class
//

#ifndef AEMAKEFILE_PRINTER_H
#define AEMAKEFILE_PRINTER_H

#include <common/nstring.h>

/**
  * The printer class is used to represent the state of a simple output
  * class.
  */
class printer
{
public:
    /**
      * The destructor.
      */
    virtual ~printer();

    /**
      * The putch method is used to write a character to the output.
      */
    virtual void putch(char c) = 0;

    /**
      * The write method is used to write data to the output.
      */
    void write(const char *data, size_t len);

    /**
      * The puts method is used to write a string to the output.
      */
    void puts(const char *s);

    /**
      * The put method is used to write a string to the output.
      */
    void put(const nstring &arg) { puts(arg.c_str()); }

protected:
    /**
      * The default constructor.
      * You must derive classes from this one; only derived classes may
      * construct onject of this type.
      */
    printer();

private:
    /**
      * The copy constructor.  Do not use.
      */
    printer(const printer &);

    /**
      * The assignment operator.  Do not use.
      */
    printer &operator=(const printer &);
};

inline printer &
operator<<(printer &p, char arg)
{
    p.putch(arg);
    return p;
}

inline printer &
operator<<(printer &p, const char *arg)
{
    p.puts(arg);
    return p;
}

inline printer &
operator<<(printer &p, const nstring &arg)
{
    p.put(arg);
    return p;
}

class nstring_list; // forward
printer &operator<<(printer &p, const nstring_list &nsl);

#endif // AEMAKEFILE_PRINTER_H
