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

#ifndef AEMAKEFILE_PROCESS_BODY_H
#define AEMAKEFILE_PROCESS_BODY_H

#include <aemakefile/process.h>

/**
  * The process_body class is used to represent the processing in the
  * middle of the generated Makefile.in file, the rules needed for each
  * source file.
  */
class process_body:
    public process
{
public:
    /**
      * The destructor.
      */
    virtual ~process_body();

    /**
      * The constructor.
      */
    process_body(printer &arg);

    // See base class for documentation.
    void per_file(const nstring &filename);

    /**
      * The directory class method is used to specify a search path.
      * May be called multiple times.  The directories will be searched
      * in the order given.
      */
    static void directory(const char *arg);

private:
    /**
      * The search_path class variable is used to remember where to look
      * for dependency files.
      */
    static nstring_list search_path;

    /**
      * The read_dependency_file class method is used to read a
      * dependency file that that it can be inserted into the generated
      * Makefile.in file.
      */
    static void read_dependency_file(const nstring&, nstring_list&);

    /**
      * The default constructor.  Do not use.
      */
    process_body();

    /**
      * The copy constructor.  Do not use.
      */
    process_body(const process_body &);

    /**
      * The assignment operator.  Do not use.
      */
    process_body &operator=(const process_body &);
};

#endif // AEMAKEFILE_PROCESS_BODY_H
