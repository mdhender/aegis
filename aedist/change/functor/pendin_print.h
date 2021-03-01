//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
//	Copyright (C) 2005 Walter Franzini;
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

#ifndef AEDIST_CHANGE_FUNCTOR_PENDIN_PRINT_H
#define AEDIST_CHANGE_FUNCTOR_PENDIN_PRINT_H

#include <common/nstring.h>
#include <common/symtab/template.h>
#include <libaegis/change.h>
#include <libaegis/change/functor.h>
#include <libaegis/col.h>
#include <libaegis/project.h>

template <class valye_type_ty> class symtab;

/**
  * The change_functor_pendin_print class is used to represent
  */
class change_functor_pending_printer:
    public change_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~change_functor_pending_printer();

    /**
      *  The constructor.
      */
    change_functor_pending_printer(bool incbr, project_ty *proj,
	const nstring &repository, symtab<nstring> *remote,
	nstring_list *include_uuid_list, nstring_list *include_version_list,
	nstring_list *exclude_uuid_list, nstring_list *exclude_version_list);

    // See base class for documentation.
    virtual void operator()(change::pointer );

private:
    project_ty *pp;
    symtab<nstring> *remote_inventory;
    nstring_list *in_uuid_list;
    nstring_list *in_version_list;
    nstring_list *ex_uuid_list;
    nstring_list *ex_version_list;
    col::pointer colp;
    output::pointer vers_col;
    output::pointer uuid_col;
    output::pointer desc_col;
    int n;

    /**
      * The copy constructor.
      */
    change_functor_pending_printer(const change_functor_pending_printer &);

    /**
      * The assignment operator.
      */
    change_functor_pending_printer &operator=
        (const change_functor_pending_printer &);
};

#endif // AEDIST_CHANGE_FUNCTOR_PENDIN_PRINT_H
