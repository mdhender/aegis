//
//	aegis - project change supervisor
//	Copyright (C) 2001-2006, 2008 Peter Miller
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

#include <common/str.h>
#include <common/stracc.h>


string_ty *
str_cat_three(string_ty *s1, string_ty *s2, string_ty *s3)
{
    static stracc_t ac;
    ac.clear();
    ac.push_back(s1->str_text, s1->str_length);
    ac.push_back(s2->str_text, s2->str_length);
    ac.push_back(s3->str_text, s3->str_length);
    return ac.mkstr();
}
