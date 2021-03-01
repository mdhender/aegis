//
// aegis - project change supervisor
// Copyright (C) 2008, 2011 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/output/file.h>
#include <libaegis/output/filter/bzip2.h>
#include <libaegis/output/filter/gzip.h>
#include <libaegis/output/stdout.h>


output::pointer
output_file::compressed_text_open(const nstring &filename)
{
    if (filename.empty() || filename == "-")
        return output_stdout::create();
    nstring lcfn = filename.downcase();
    if (lcfn.ends_with(".gz"))
    {
        output::pointer op = binary_open(filename);
        return output_filter_gzip::create(op);
    }
    if (lcfn.ends_with(".bz") || lcfn.ends_with(".bz2") )
    {
        output::pointer op = binary_open(filename);
        return output_filter_bzip2::create(op);
    }
    return text_open(filename);
}
