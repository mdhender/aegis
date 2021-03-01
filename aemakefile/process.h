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

#ifndef AEMAKEFILE_PROCESS_H
#define AEMAKEFILE_PROCESS_H

#include <aemakefile/printer.h>
#include <common/nstring/list.h>

/**
  * The process class is used to represent
  */
class process
{
public:
    /**
      * The destructor.
      */
    virtual ~process();

    /**
      * The run method is used to call the prelude, per_file and
      * postlude methods in order.
      */
    void run(const nstring_list &filenames);

protected:
    /**
      * The constructor.
      * May only be called by a derived class.
      *
      * @param arg
      *     When to print the output.
      */
    process(printer &arg);

    /**
      * The prelude method is used to emit constant text before all of
      * the files.
      */
    virtual void prelude();

    /**
      * The per_file method is used to do some file-specific processing.
      * It may produce output at this point, or it may not.
      */
    virtual void per_file(const nstring &filename) = 0;

    /**
      * The per_file method is used to do some file-specific processing
      * given a list of file names.
      */
    void per_file(const nstring_list &filenames);

    /**
      * The postlude method is used to emit text after all of the files.
      * Often it will be targets derived from the file names specified.
      */
    virtual void postlude();

protected:
    /**
      * The print instance variable is used to remember where to send
      * the output.
      */
    printer &print;

private:
    /**
      * The default constructor.  Do not use.
      */
    process();

    /**
      * The copy constructor.  Do not use.
      */
    process(const process &);

    /**
      * The assignment operator.  Do not use.
      */
    process &operator=(const process &);
};

#endif // AEMAKEFILE_PROCESS_H
