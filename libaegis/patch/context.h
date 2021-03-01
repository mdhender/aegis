//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_PATCH_CONTEXT_H
#define LIBAEGIS_PATCH_CONTEXT_H

#include <common/str_list.h>
#include <libaegis/input.h>

/**
  * The patch_context_ty class is used to represent a line buffered
  * input stream, with the ability to fetch individual lines of input.
  */
class patch_context_ty
{
public:
    /**
      * The destructor.
      */
    ~patch_context_ty();

    /**
      * The constructor.
      *
      * \param arg
      *     The input stream to read and buffer.
      */
    patch_context_ty(input &arg);

    /**
      * The getline method is used to grab the line with the given
      * numkber (zero based).
      *
      * \param n
      *     The line number to fatch (zero based), relative to the
      *     current position.
      */
    string_ty *getline(int n);

    /**
      * The discard method is used to consume some lines of inout.
      *
      * \param n
      *     The number of lines to discard from the front of the buffer.
      */
    void discard(int n);

    /**
      * The get_file_name method is used to get the name and line number
      * of the input stream.  This is mostly used for debugging.
      */
    nstring get_file_name() { return in->name(); }

private:
    /**
      * The input instance variable is used to remember where to get
      * more input lines from.
      */
    input in;

    /**
      * The buffer instance variable is used to remember lines which
      * have already been read from the input.
      */
    string_list_ty buffer;

    /**
      * The default constructor.  Do not use.
      */
    patch_context_ty();

    /**
      * The copy constructor.  Do not use.
      */
    patch_context_ty(const patch_context_ty&);

    /**
      * The assignment operator.  Do not use.
      */
    patch_context_ty &operator=(const patch_context_ty&);
};

inline patch_context_ty *
patch_context_new(input &ip)
{
    return new patch_context_ty(ip);
}

inline void
patch_context_delete(patch_context_ty *pcp)
{
    delete pcp;
}

inline string_ty *
patch_context_getline(patch_context_ty *pcp, int n)
{
    return pcp->getline(n);
}

inline void
patch_context_discard(patch_context_ty *pcp, int n)
{
    pcp->discard(n);
}

#endif // LIBAEGIS_PATCH_CONTEXT_H
