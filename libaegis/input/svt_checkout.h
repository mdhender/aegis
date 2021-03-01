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
// MANIFEST: interface of the input_svt_checkout class
//

#ifndef LIBAEGIS_INPUT_SVT_CHECKOUT_H
#define LIBAEGIS_INPUT_SVT_CHECKOUT_H

#pragma interface "input_svt_checkout"

#include <input.h>

/**
  * The input_svt_checkout class is used to represent and input which is
  * a version extracted from a Simple Version Tool archive.
  */
class input_svt_checkout:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_svt_checkout();

    /**
      * The constructor.
      *
      * @param filename
      *     The name of the history file being manipulated
      * @param version
      *     The version to checkout.
      */
    input_svt_checkout(const nstring &filename, const nstring &version);

    // See base class for socumentation.
    nstring name();

    // See base class for socumentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for socumentation.
    long length();

    // See base class for socumentation.
    long ftell_inner();

private:
    input_ty *src1_p;
    input_ty *src2_p;
    input_ty *src3_p;
    input_ty *src4_p;
    long pos;

    /**
      * The default constructor.  Do not use.
      */
    input_svt_checkout();

    /**
      * The copy constructor.  Do not use.
      */
    input_svt_checkout(const input_svt_checkout &);

    /**
      * The assignment operator.  Do not use.
      */
    input_svt_checkout &operator=(const input_svt_checkout &);
};

#endif // LIBAEGIS_INPUT_SVT_CHECKOUT_H
