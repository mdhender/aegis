//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_OUTPUT_TEE_H
#define LIBAEGIS_OUTPUT_TEE_H

#include <libaegis/output.h>

/**
  * the output_tee class is used to represent an output filter which
  * writes its output to two places at once.
  */
class output_tee:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_tee();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * \param d1
      *     the deeper output stream on which this filter writes to.
      * \param d2
      *     the deeper output stream on which this filter writes to.
      */
    output_tee(const output::pointer &d1, const output::pointer &d2);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * \param d1
      *     the deeper output stream on which this filter writes to.
      * \param d2
      *     the deeper output stream on which this filter writes to.
      */
    static pointer create(const output::pointer &d1, const output::pointer &d2);

protected:
    // See base class for documentation.
    nstring filename() const;

    // See base class for documentation.
    long ftell_inner() const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    int page_width() const;

    // See base class for documentation.
    int page_length() const;

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    void end_of_line_inner();

    // See base class for documentation.
    void flush_inner();

private:
    /**
      * The d1 instance variable is used to remember the deeper output
      * stream on which this filter writes to.
      */
    output::pointer d1;

    /**
      * The d1_close instance variable is used to remember whether or
      * not the d1 output stream is to be deleted in our destructor.
      */
    bool d1_close;

    /**
      * The d2 instance variable is used to remember the deeper output
      * stream on which this filter writes to.
      */
    output::pointer d2;

    /**
      * The d2_close instance variable is used to remember whether or
      * not the d2 output stream is to be deleted in our destructor.
      */
    bool d2_close;

    /**
      * The default constructor.  Do not use.
      */
    output_tee();

    /**
      * The copy constructor.  Do not use.
      */
    output_tee(const output_tee &);

    /**
      * The assignment operator.  Do not use.
      */
    output_tee &operator=(const output_tee &);
};

#endif // LIBAEGIS_OUTPUT_TEE_H
