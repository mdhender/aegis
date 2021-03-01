//
// aegis - project change supervisor
// Copyright (C) 2005, 2006, 2008, 2011, 2012 Peter Miller
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

#ifndef AEREVML_OUTPUT_REVML_ENCODE_H
#define AEREVML_OUTPUT_REVML_ENCODE_H

#include <libaegis/output.h>

/**
  * The output_revml_encode class is used to represent an output filter
  * which XML (SGML) encodes a character stream, but with some RevML
  * extentions.
  */
class output_revml_encode:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_revml_encode();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * \param deeper
      *     The deeper output on which this filter writes its output.
      */
    output_revml_encode(const output::pointer &deeper);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * \param deeper
      *     The deeper output on which this filter writes its output.
      */
    static pointer create(const output::pointer &deeper);

protected:
    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void flush_inner(void);

    // See base class for documentation.
    nstring filename(void) const;

    // See base class for documentation.
    long ftell_inner(void) const;

    // See base class for documentation.
    int page_width(void) const;

    // See base class for documentation.
    int page_length(void) const;

    // See base class for documentation.
    void end_of_line_inner(void);

    // See base class for documentation.
    nstring type_name(void) const;

private:
    /**
      * The deeper instance variable is used to remember the deeper
      * output on which this filter writes its output.
      */
    output::pointer deeper;

    long pos;

    bool bol;

    /**
      * The default constructor.  Do not use.
      */
    output_revml_encode();

    /**
      * The copy constructor.  Do not use.
      */
    output_revml_encode(const output_revml_encode &);

    /**
      * The assignment operator.  Do not use.
      */
    output_revml_encode &operator=(const output_revml_encode &);
};

#endif // AEREVML_OUTPUT_REVML_ENCODE_H
// vim: set ts=8 sw=4 et :
