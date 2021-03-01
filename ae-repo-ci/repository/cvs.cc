//
// aegis - project change supervisor
// Copyright (C) 2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/stdlib.h>

#include <common/env.h>
#include <common/error.h>
#include <common/trace.h>
#include <libaegis/file.h>
#include <libaegis/os.h>
#include <libaegis/undo.h>

#include <ae-repo-ci/repository/cvs.h>


repository_cvs::~repository_cvs()
{
    trace(("repository_cvs::~repository_cvs()\n"));
}


repository_cvs::repository_cvs()
{
    trace(("repository_cvs::repository_cvs()\n"));

    //
    // Make sure the necessary environment variables have been set, so
    // that we can proceed without user interaction.
    //
    const char *cp = getenv("CVSROOT");
    if (!cp || !*cp)
    {
        error_raw("the CVSROOT environment variable has not been set");
        cp = getenv("CVS_RSH");
        if (!cp || !*cp)
        {
            error_raw
            (
                "you probably need to set the CVS_RSH environment "
                "variable as well"
            );
        }
        exit(1);
    }

    env_unset("CVSREADONLYFS");
    env_unset("CVSREAD");

    // env_set("USER", ...name of change's developer);
    // env_set("LOGIN", ...name of change's developer);
}


void
repository_cvs::checkout(const nstring &a_mod, const nstring &a_dir)
{
    trace(("repository_cvs::checkout(module = %s, directory = %s)\n{\n",
        a_mod.quote_c().c_str(), a_dir.quote_c().c_str()));
    module = a_mod;
    assert(!module.empty());

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
    // Run the CVS checkout command.
    //
    nstring command = "cvs checkout -d " + directory.quote_shell() + " "
        + module.quote_shell();
    int flags = 0;
    os_execute(command, flags, os_dirname_relative(directory));
    trace(("}\n"));
}


nstring
repository_cvs::get_directory()
    const
{
    return directory;
}


void
repository_cvs::remove_file(const nstring &file_name)
{
    trace(("repository_cvs::remove_file(file_name = %s)\n{\n",
        file_name.quote_c().c_str()));
    nstring path = os_path_cat(directory, file_name);
    if (os_exists(path))
        os_unlink(path);
    nstring command = "cvs delete " + path.quote_shell();
    int flags = 0;
    os_execute(command, flags, directory);
    trace(("}\n"));
}


void
repository_cvs::add_file(const nstring &file_name, const nstring &from)
{
    trace(("repository_cvs::add_file(file_name = %s, from = %s)\n{\n",
        file_name.quote_c().c_str(), from.quote_c().c_str()));
    //
    // Make sure all the directories exist,
    // and cvs add them if they don't.
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

            nstring command = "cvs add " + s2.quote_shell();
            int flags = 0;
            os_execute(command, flags, directory);
        }
    }

    //
    // Copy the baseline file into the CVS work area.
    //
    nstring dest = os_path_cat(directory, file_name);
    copy_whole_file(from, dest, false);

    //
    // Now let CVS know it is supposed to manage this new file.
    //
    nstring keywords = (is_binary(dest) ? "-kb" : "-ko");
    nstring command = "cvs add " + keywords + " " + dest.quote_shell();
    int flags = 0;
    os_execute(command, flags, directory);
    trace(("}\n"));
}


void
repository_cvs::modify_file(const nstring &file_name, const nstring &from)
{
    trace(("repository_cvs::modify_file(file_name = %s, from = %s)\n{\n",
        file_name.quote_c().c_str(), from.quote_c().c_str()));
    //
    // Copy the baseline file into the CVS work area.
    // No special command lets CVS know it was changed.
    //
    copy_whole_file(from, os_path_cat(directory, file_name), false);
    trace(("}\n"));
}


void
repository_cvs::commit(const nstring &comment)
{
    trace(("repository_cvs::commit(comment = %s)\n{\n",
        comment.quote_c().c_str()));
    //
    // Give all of the files to CVS
    //
    // FIME: how do you set the ,v author field for the checkin?  It
    // would be nice to use that option to set the developer, rather
    // than the project owner.
    //
    nstring command = "cvs commit -m " + comment.quote_shell();
    int flags = 0;
    os_execute(command, flags, directory);

    //
    // Get rid of the work area.
    //
    os_rmdir_tree(directory);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
