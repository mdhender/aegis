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

#ifndef AEIMPORT_CHANGE_SET_H
#define AEIMPORT_CHANGE_SET_H

#include <common/ac/time.h>

#include <common/str_list.h>
#include <aeimport/change_set/file_list.h>

class change_set_ty
{
public:
    ~change_set_ty();
    change_set_ty();
    void validate() const;

// private:
    string_ty       *who;
    time_t	    when;
    string_ty       *description;
    change_set_file_list_ty file;
    string_list_ty  tag;

private:
    change_set_ty(const change_set_ty &);
    change_set_ty &operator=(const change_set_ty &);
};

#endif // AEIMPORT_CHANGE_SET_H
