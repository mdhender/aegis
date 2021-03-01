//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: functions to impliment the simple version tool checkout
//

#include <input/svt_checkout.h>
#include <output/file.h>
#include <simpverstool.h>


void
simple_version_tool::checkout(const nstring &output_file_name,
    const nstring &version)
{
    output_ty *op = output_file_open(output_file_name, true);
    checkout(*op, version);
    delete op;
}


void
simple_version_tool::checkout(output_ty &dest, const nstring &version)
{
    input_svt_checkout src(history_file_name, version);
    dest << src;
}
