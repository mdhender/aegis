//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller;
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License,
//      version 3, as published by the Free Software Foundation.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
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
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring filename = filenames[j];
	preprocess(filename);
    }
    begin();
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring filename = filenames[j];
        bool is_a_script = filename.gmatch(scripts);
	process(filename, is_a_script);
    }
    end();
}


void
target::preprocess(const nstring &)
{
    // Do nothing.
}


bool
target::trim_script_suffix()
    const
{
    // FIXME: this may need to be controlled from a command line option,
    // one day.
    return true;
}
