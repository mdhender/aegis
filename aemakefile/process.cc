//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the process class
//

#include <aemakefile/process.h>


process::~process()
{
}


process::process(printer &arg) :
    print(arg)
{
}


void
process::run(const nstring_list &filenames)
{
    prelude();
    per_file(filenames);
    postlude();
}


void
process::prelude()
{
}


void
process::per_file(const nstring_list &filenames)
{
    for (size_t j = 0; j < filenames.size(); ++j)
	per_file(filenames[j]);
}


void
process::postlude()
{
}
