//
//	aegis - project change supervisor
//	Copyright (C) 2005 Walter Franzini;
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
// MANIFEST: interface of the change_functor_pendin_print class
//

#ifndef AEDIST_CHANGE_FUNCTOR_PENDIN_PRINT_H
#define AEDIST_CHANGE_FUNCTOR_PENDIN_PRINT_H

#include <libaegis/change/functor.h>
#include <libaegis/change.h>
#include <libaegis/project.h>
#include <common/symtab/template.h>
#include <common/nstring.h>

struct col_ty;
struct output_ty;
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
    virtual void operator()(change_ty *);

private:
    project_ty *pp;
    symtab<nstring> *remote_inventory;
    nstring_list *in_uuid_list;
    nstring_list *in_version_list;
    nstring_list *ex_uuid_list;
    nstring_list *ex_version_list;
    col_ty *colp;
    output_ty *vers_col;
    output_ty *uuid_col;
    output_ty *desc_col;
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
