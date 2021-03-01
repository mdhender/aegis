//
// aegis - project change supervisor
// Copyright (C) 2008 Walter Franzini
// Copyright (C) 2008 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_CHANGE_FUNCTOR_FILE_FIND_H
#define LIBAEGIS_CHANGE_FUNCTOR_FILE_FIND_H

#include <common/symtab/template.h>
#include <libaegis/change/functor.h>

class nstring_list;

/**
  * The change_functor_file_find class is used to represent
  * a functor used to walk open changes and branches accumulating
  * file's UUIDs.
  */
class change_functor_file_find:
    public change_functor
{
public:
    /**
      * The constructor
      */
    change_functor_file_find(const nstring_list &);

    /**
      * The destructor.
      */
    virtual ~change_functor_file_find();

    // See base class for documentation.
    void operator()(change::pointer cp);

    /**
      * The query method is used to obtain the UUID already assigned
      * to the file name s, if any.
      *
      * @param fn
      *         The name of the file.
      * @returns
      *         A pointer to the nstring representing the UUID of the
      *         file or 0 if the file is not yet in the repository or
      *         it is in the repository without an UUID.
     */
    nstring* query(const nstring &fn) const;

    /**
      * The query method is used to obtain the UUID already assigned
      * to the file name s, if any.
      *
      * @param fn
      *         The name of the file.
      * @returns
      *         A pointer to the nstring representing the UUID of the
      *         file or 0 if the file is not yet in the repository or
      *         it is in the repository without an UUID.
      */
    nstring* query(string_ty *fn) const { return query(nstring(fn)); }

private:
    symtab<nstring> file_to_uuid;

    /**
      * The copy constructor.  Do not use.
      */
    change_functor_file_find(const change_functor_file_find &);

    /**
      * The assignment operator.  Do not use.
      */
    change_functor_file_find &operator=(const change_functor_file_find &);
};

#endif // LIBAEGIS_CHANGE_FUNCTOR_FILE_FIND_H
