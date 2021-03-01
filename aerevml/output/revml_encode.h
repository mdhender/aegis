//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: interface of the output_revml_encode class
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
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_revml_encode();

    /**
      * The constructor.
      *
      * \param deeper
      *     The deeper output on which this filter writes its output.
      * \param close_on_close
      *     Whether or not to delete deeper in our destructor.
      */
    output_revml_encode(output_ty *deeper, bool close_on_close);

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void flush_inner();

    // See base class for documentation.
    string_ty *filename() const;

    // See base class for documentation.
    long ftell_inner() const;

    // See base class for documentation.
    int page_width() const;

    // See base class for documentation.
    int page_length() const;

    // See base class for documentation.
    void end_of_line_inner();

    // See base class for documentation.
    const char *type_name() const;

private:
    /**
      * The deeper instance variable is used to remember the deeper
      * output on which this filter writes its output.
      */
    output_ty *deeper;

    /**
      * The close_on_close instance variable is used to remember whether
      * or not to delete deeper in our destructor.
      */
    bool close_on_close;

    long pos;

    bool bol;

    /**
      * The default constructor.
      */
    output_revml_encode();

    /**
      * The copy constructor.
      */
    output_revml_encode(const output_revml_encode &);

    /**
      * The assignment operator.
      */
    output_revml_encode &operator=(const output_revml_encode &);
};

#endif // AEREVML_OUTPUT_REVML_ENCODE_H
