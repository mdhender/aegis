//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2007 Peter Miller;
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License,
//      version 2, as published by the Free Software Foundation.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public
//      License along with this program; if not, write to the Free
//      Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//      MA 02111, USA.
//

#include <common/ac/unistd.h>

#include <aemakegen/target.h>


nstring_list target::view_path;
nstring_list target::scripts;


target::~target()
{
}


target::target()
{
}


void
target::vpath(const nstring &arg)
{
    view_path.push_back(arg + "/");
}


void
target::script(const nstring &arg)
{
    scripts.push_back(arg);
}


nstring
target::resolve(const nstring &relpath)
{
    if (relpath[0] == '/')
	return relpath;
    for (size_t j = 0; j < view_path.size(); ++j)
    {
	nstring path = view_path[j] + relpath;
	if (exists(path))
	    return path;
    }
    return relpath;
}


bool
target::exists(const nstring &path)
{
    return (access(path.c_str(), F_OK) == 0);
}


void
target::process(const nstring_list &filenames)
{
    begin();
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring filename = filenames[j];
        bool is_a_script = filename.gmatch(scripts);
	process(filename, is_a_script);
    }
    end();
}


bool
target::trim_script_suffix()
    const
{
    // FIXME: this may need to be controlled from a command line option,
    // one day.
    return true;
}
