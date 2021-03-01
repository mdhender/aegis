//
//	aegis - project change supervisor
//	Copyright (C) 2004 Walter Franzini;
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
// MANIFEST: interface of the change_functor_invent_build class
//

#ifndef AEDIST_CHANGE_FUNCTOR_INVENT_BUILD_H
#define AEDIST_CHANGE_FUNCTOR_INVENT_BUILD_H

#pragma interface "change_functor_invent_build"

#include <change/functor.h>
#include <project.h>
#include <change.h>
#include <symtab/template.h>

/**
  * The change_functor_invent_build class is used to represent
  */
class change_functor_inventory_builder:
    public change_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~change_functor_inventory_builder();

    /**
      * The constructor.
      */
    change_functor_inventory_builder(project_ty *, symtab<change_ty> *);


    void operator()(change_ty *);

private:
    symtab<change_ty> *stp;
    project_ty *pp;

    /**
      * The copy constructor.
      */
    change_functor_inventory_builder
        (const change_functor_inventory_builder &);

    /**
      * The assignment operator.
      */
    change_functor_inventory_builder
        &operator=(const change_functor_inventory_builder &);
};

#endif // AEDIST_CHANGE_FUNCTOR_INVENT_BUILD_H
