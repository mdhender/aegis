//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005, 2006 Peter Miller
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
// MANIFEST: interface definition for libaegis/output/tee.c
//

#ifndef LIBAEGIS_OUTPUT_TEE_H
#define LIBAEGIS_OUTPUT_TEE_H

#include <libaegis/output.h>

/**
  * the output_tee_ty class is used to represent an output filter which
  * writes its output to two places at once.
  */
class output_tee_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_tee_ty();

    /**
      * The constructor.
      *
      * \param d1
      *     the deeper output stream on which this filter writes to.
      * \param d1_close
      *     whether or not the d1 output stream is to be deleted in our
      *     destructor.
      * \param d2
      *     the deeper output stream on which this filter writes to.
      * \param d2_close
      *     whether or not the d2 output stream is to be deleted in our
      *     destructor.
      */
    output_tee_ty(output_ty *d1, bool d1_close, output_ty *d2, bool d2_close);

    // See base class for documentation.
    string_ty *filename() const;

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
    output_ty *d1;

    /**
      * The d1_close instance variable is used to remember whether or
      * not the d1 output stream is to be deleted in our destructor.
      */
    bool d1_close;

    /**
      * The d2 instance variable is used to remember the deeper output
      * stream on which this filter writes to.
      */
    output_ty *d2;

    /**
      * The d2_close instance variable is used to remember whether or
      * not the d2 output stream is to be deleted in our destructor.
      */
    bool d2_close;

    /**
      * The default constructor.  Do not use.
      */
    output_tee_ty();

    /**
      * The copy constructor.  Do not use.
      */
    output_tee_ty(const output_tee_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_tee_ty &operator=(const output_tee_ty &);
};

inline DEPRECATED output_ty *
output_tee(output_ty *arg1, bool arg2, output_ty *arg3, bool arg4)
{
    return new output_tee_ty(arg1, arg2, arg3, arg4);
}

#endif // LIBAEGIS_OUTPUT_TEE_H
