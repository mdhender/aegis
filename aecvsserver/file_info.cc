//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <aecvsserver/file_info.h>
#include <common/mem.h>
#include <common/str.h>


file_info_ty *
file_info_new(void)
{
    file_info_ty    *fip;

    fip = (file_info_ty *)mem_alloc(sizeof(file_info_ty));
    fip->version = 0;
    fip->modified = -1;
    fip->mode = 0644;
    fip->questionable = 0;
    return fip;
}


void
file_info_reset(file_info_ty *fip)
{
    if (fip->version)
	str_free(fip->version);
    fip->version = 0;
    fip->modified = -1;
    fip->mode = 0644;
    fip->questionable = 0;
}


void
file_info_delete(file_info_ty *fip)
{
    file_info_reset(fip);
    mem_free(fip);
}
