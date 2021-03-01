//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#ifndef AEMAKEFILE_PRINTER_WRAP_H
#define AEMAKEFILE_PRINTER_WRAP_H

#include <aemakefile/printer.h>

/**
  * The printer_wrap class is used to represent a line wrapping output
  * filter.  Lines ar ewrapped in such a way as to remain a valid
  * Makefile.
  */
class printer_wrap:
    public printer
{
public:
    /**
      * The destructor.
      */
    virtual ~printer_wrap();

    /**
      * The constructor.
      */
    printer_wrap(printer *arg);

    // See base class for documentation.
    void putch(char c);

private:
    enum {maximum_line_width = 72 };
    printer *deeper;
    bool start_of_line;
    bool continuation_line;
    int column;
    int buflen;
    int bufwidth;
    int bufmax;
    char *buffer;

    void stash_inner(char c);
    void emit_buffer(void);

    /**
      * The default constructor.  Do not use.
      */
    printer_wrap();

    /**
      * The copy constructor.  Do not use.
      */
    printer_wrap(const printer_wrap &);

    /**
      * The assignment operator.  Do not use.
      */
    printer_wrap &operator=(const printer_wrap &);
};

#endif // AEMAKEFILE_PRINTER_WRAP_H
