//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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

#include <aemakegen/process_item.h>
#include <aemakegen/target.h>


process_item::~process_item()
{
}


process_item::process_item(target &a_tgt, target_method_ptr a_method) :
    data(a_tgt.data),
    tgt(a_tgt),
    method(a_method)
{
}


void
process_item::tgt_preprocess(const nstring &filename)
{
    tgt.processing.run_preprocess(filename);
}


bool
process_item::run_preprocess(const nstring &filename)
{
    if (condition(filename))
    {
        preprocess(filename);
        return true;
    }
    return false;
}


void
process_item::preprocess_end(void)
{
    // default implementation does nothing
}


bool
process_item::run_process(const nstring &filename)
{
    if (condition(filename))
    {
        process(filename);
        return true;
    }
    return false;
}


void
process_item::process(const nstring &filename)
{
    (tgt.*method)(filename);
}


bool
process_item::is_installable(const nstring &name)
{
    return tgt.is_installable(name);
}


nstring
process_item::get_project_name(void)
{
    return tgt.get_project_name();
}


bool
process_item::is_hash_bang(const nstring &path)
{
    return tgt.is_hash_bang(path);
}


bool
process_item::contains_dot_so_directive(const nstring &filename)
{
    return tgt.contains_dot_so_directive(filename);
}


bool
process_item::filename_implies_is_a_script(const nstring &filename)
{
    return tgt.filename_implies_is_a_script(filename);
}


// vim: set ts=8 sw=4 et :
