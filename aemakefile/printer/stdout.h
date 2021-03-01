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

#ifndef AEMAKEFILE_PRINTER_STDOUT_H
#define AEMAKEFILE_PRINTER_STDOUT_H

#include <aemakefile/printer.h>

/**
  * The printer_stdout class is used to represent
  */
class printer_stdout:
    public printer
{
public:
    /**
      * The destructor.
      */
    virtual ~printer_stdout();

    /**
      * The default constructor.
      */
    printer_stdout();

    // See base class for documentation.
    void putch(char arg);

private:
    /**
      * The copy constructor.  Do not use.
      */
    printer_stdout(const printer_stdout &);

    /**
      * The assignment operator.  Do not use.
      */
    printer_stdout &operator=(const printer_stdout &);
};

#endif // AEMAKEFILE_PRINTER_STDOUT_H
