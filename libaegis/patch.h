//
// aegis - project change supervisor
// Copyright (C) 2001-2006, 2008, 2012 Peter Miller
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

#ifndef LIBAEGIS_PATCH_H
#define LIBAEGIS_PATCH_H

#include <libaegis/common.fmtgen.h>
#include <libaegis/patch/hunk_list.h>
#include <common/str_list.h>

/**
  * The patch_ty class is used to represent a diff between one file
  * version and the next.  This is only for the one file.
  */
class patch_ty
{
public:
    /**
      * The destructor.
      */
    ~patch_ty();

    /**
      * The default constructor.
      */
    patch_ty();

    /**
      * The append method is used to add another hunk to the end of the
      * patch for this file.
      */
    void append(patch_hunk_ty *php);

// private:
    string_list_ty  name;
    file_action_ty  action;
    file_usage_ty   usage;
    patch_hunk_list_ty actions;

private:
    /**
      * The copy constructor.  Do not use.
      */
    patch_ty(const patch_ty &);

    /**
      * The assignment.  Do not use.
      */
    patch_ty &operator=(const patch_ty &);
};

struct input_ty; // forward

bool patch_apply(patch_ty *, string_ty *, string_ty *);

inline patch_ty *
patch_new(void)
{
    return new patch_ty();
}

inline void
patch_delete(patch_ty *pp)
{
    delete pp;
}

inline void
patch_append(patch_ty *pp, patch_hunk_ty *php)
{
    pp->append(php);
}

#endif // LIBAEGIS_PATCH_H
// vim: set ts=8 sw=4 et :
