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

#ifndef AEIMPORT_CHANGE_SET_FILE_H
#define AEIMPORT_CHANGE_SET_FILE_H

#include <common/str_list.h>

enum change_set_file_action_ty
{
    change_set_file_action_create,
    change_set_file_action_modify,
    change_set_file_action_remove
};

/**
  * The change_set_file_ty class is used to represent a single operation
  * on a single file in a single change set.
  */
struct change_set_file_ty
{
public:
    /**
      * The destructpr.
      * DO NOT subclass me, I'm not virtual.
      */
    ~change_set_file_ty();

    /**
      * The constructor.
      */
    change_set_file_ty(string_ty *filename, string_ty *edit,
	change_set_file_action_ty action, string_list_ty *tag);

    void validate() const;

    void merge(const change_set_file_ty &);

// private:
    /**
      * The filename instance variable is used to remember the name of
      * the file in a change set.
      */
    string_ty *filename;

    /**
      * The edit instance variable is used to remember the edit number
      * of the file in a change set.
      */
    string_ty *edit;

    /**
      * The filename instance variable is used to remember what is being
      * done to the file in a change set.
      */
    change_set_file_action_ty action;

    /**
      * The filename instance variable is used to remember the tag names
      * attached to a file in a change set.
      */
    string_list_ty tag;

private:
    /**
      * The default constructor.  Do not use.
      */
    change_set_file_ty();

    /**
      * The copy constructor.  Do not use.
      */
    change_set_file_ty(const change_set_file_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    change_set_file_ty &operator=(const change_set_file_ty &);
};

const char *change_set_file_action_name(change_set_file_action_ty);

inline void
change_set_file_validate(change_set_file_ty *csfp)
{
    csfp->validate();
}

#endif // AEIMPORT_CHANGE_SET_FILE_H
