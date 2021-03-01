//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
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
// MANIFEST: implementation of the repository_subversion class
//

#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/env.h>
#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/file.h>
#include <libaegis/os.h>
#include <libaegis/undo.h>

#include <ae-repo-ci/repository/subversion.h>


repository_subversion::~repository_subversion()
{
    trace(("repository_subversion::~repository_subversion()\n"));
}


repository_subversion::repository_subversion()
{
    trace(("repository_subversion::repository_subversion()\n"));
}


void
repository_subversion::checkout(const nstring &a_mod, const nstring &a_dir)
{
    trace(("repository_subversion::checkout(module = %s, directory = %s)\n{\n",
	a_mod.quote_c().c_str(), a_dir.quote_c().c_str()));
    module = a_mod;
    assert(!module.empty());
    if (!strstr(module.c_str(), ":/"))
	fatal_raw("must specify the repository URL for the module");

    directory = a_dir;
    if (directory.empty())
	directory = nstring(os_edit_filename(0));
    else if (directory[0] != '/')
	directory = os_path_cat(nstring(os_curdir()), directory);

    //
    // If anything goes wrong from here, remove the checkout directory
    // and everything below it.
    //
    undo_rmdir_bg(directory);

    //
    // Run the subversion checkout command.
    //
    nstring command = "svn checkout " + module.quote_shell() + " "
	+ directory.quote_shell();
    int flags = 0;
    os_execute(command, flags, os_dirname_relative("/tmp"));
    trace(("}\n"));
}


void
repository_subversion::remove_file(const nstring &file_name)
{
    trace(("repository_subversion::remove_file(file_name = %s)\n{\n",
	file_name.quote_c().c_str()));
    nstring path = os_path_cat(directory, file_name);
    nstring command = "svn delete --force " + path.quote_shell();
    int flags = 0;
    os_execute(command, flags, directory);
    trace(("}\n"));
}


void
repository_subversion::add_file(const nstring &file_name, const nstring &from)
{
    trace(("repository_subversion::add_file(file_name = %s, from = %s)\n{\n",
	file_name.quote_c().c_str(), from.quote_c().c_str()));
    //
    // Make sure all the directories exist,
    // and subversion add them if they don't.
    //
    for (const char *cp = file_name.c_str(); *cp; ++cp)
    {
	if (*cp != '/')
    	    continue;
	nstring s1(file_name.c_str(), cp - file_name.c_str());
	nstring s2(os_path_cat(directory, s1));
	if (!os_exists(s2))
	{
	    trace(("mkdir %s\n", s2.c_str()));
	    os_mkdir(s2, 0755);

	    nstring command = "svn add --non-recursive " + s2.quote_shell();
	    int flags = 0;
	    os_execute(command, flags, directory);
	}
    }

    //
    // Copy the baseline file into the subversion work area.
    //
    nstring dest = os_path_cat(directory, file_name);
    copy_whole_file(from, dest, false);

    //
    // Now let subversion know it is supposed to manage this new file.
    //
    nstring command = "svn add " + dest.quote_shell();
    int flags = 0;
    os_execute(command, flags, directory);

    //
    // FIXME: Subversion has file properties, do we want to set
    // properties from the file Aegis attributes?
    //
    trace(("}\n"));
}


void
repository_subversion::modify_file(const nstring &filename, const nstring &from)
{
    trace(("repository_subversion::modify_file(filename = %s, from = %s)\n{\n",
	filename.quote_c().c_str(), from.quote_c().c_str()));
    //
    // Copy the baseline file into the subversion work area.
    // No special command lets subversion know it was changed.
    //
    copy_whole_file(from, os_path_cat(directory, filename), false);
    trace(("}\n"));
}


void
repository_subversion::rename_file(const nstring &old_file_name,
    const nstring &new_file_name, const nstring &from)
{
    trace(("repository_subversion::rename_file(old_file_name = %s, "
	"new_file_name = %s, from = %s)\n{\n", old_file_name.quote_c().c_str(),
	new_file_name.quote_c().c_str(), from.quote_c().c_str()));

    //
    // Make sure all the directories exist,
    // and subversion add them if they don't.
    //
    for (const char *cp = new_file_name.c_str(); *cp; ++cp)
    {
	if (*cp != '/')
    	    continue;
	nstring s1(new_file_name.c_str(), cp - new_file_name.c_str());
	nstring s2(os_path_cat(directory, s1));
	if (!os_exists(s2))
	{
	    trace(("mkdir %s\n", s2.c_str()));
	    os_mkdir(s2, 0755);

	    nstring command = "svn add --non-recursive " + s2.quote_shell();
	    int flags = 0;
	    os_execute(command, flags, directory);
	}
    }

    //
    // Now let subversion know the file moved.
    //
    nstring command = "svn move " + old_file_name.quote_shell() + " "
	+ new_file_name.quote_shell();
    int flags = 0;
    os_execute(command, flags, directory);

    //
    // Copy the baseline file into the subversion work area.
    // There is not special subversion command to let it know he content
    // changed (if it did) subversion will work it out for itself.
    //
    nstring dest = os_path_cat(directory, new_file_name);
    copy_whole_file(from, dest, false);
    trace(("}\n"));
}


void
repository_subversion::commit(const nstring &comment)
{
    trace(("repository_subversion::commit(comment = %s)\n{\n",
	comment.quote_c().c_str()));
    //
    // Give all of the files to subversion
    //
    // FIME: how do you set the author field for the checkin?  It
    // would be nice to use that option to set the developer, rather
    // than the project owner.
    //
    nstring command = "svn commit -m " + comment.quote_shell()
	+ " --non-interactive";
    int flags = 0;
    os_execute(command, flags, directory);

    //
    // Get rid of the work area.
    //
    os_rmdir_tree(directory);
    trace(("}\n"));
}
