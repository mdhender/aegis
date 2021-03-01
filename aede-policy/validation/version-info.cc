//
// aegis - project change supervisor
// Copyright (C) 2008, 2009, 2011, 2012 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/sys/stat.h>
#include <common/ac/unistd.h>

#include <common/nstring/list.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/version-info.h>


validation_version_info::~validation_version_info()
{
}


validation_version_info::validation_version_info()
{
}


validation::pointer
validation_version_info::create()
{
    return pointer(new validation_version_info());
}


static nstring
resolve(const nstring &filename, const string_list_ty &search_path)
{
    for (size_t j = 0; j < search_path.nstrings; ++j)
    {
        struct stat st;
        nstring path = nstring(search_path.string[j]) + "/" + filename;
        if (stat(path.c_str(), &st) == 0)
            return path;
    }
    return (nstring(search_path[0]) + "/" + filename);
}


static nstring
safe_readlink(const nstring &path)
{
    char buf[2000];
    int n = readlink(path.c_str(), buf, sizeof(buf));
    if (n < 0)
        n = 0;
    return nstring(buf, n);
}


static nstring
version_info_unsplit(long c, long r, long a)
{
    return nstring::format("%ld:%ld:%ld", c, r, a);
}


static bool
extsym(const nstring &t)
{
    if (t.size() != 1)
        return false;
    unsigned char c = t[0];
    return (isupper(c) && c != 'U');
}


static void
grope_shared_library(const nstring &filename,
    const string_list_ty &search_path, nstring &version_info,
    nstring_list &symbols)
{
    version_info = "0:0:0";
    symbols.clear();

    nstring path = resolve(filename, search_path);

    //
    // libtool creates a symbolic link
    // libname.so -> libname.so.1.2.3
    //
    {
        nstring link = safe_readlink(path);
        if (link.empty())
            return;
        nstring_list parts;
        parts.split(link, ".");
        size_t idx = 0;
        while (idx < parts.size())
        {
            ++idx;
            if (parts[idx - 1] == "so")
                break;
        }
        if (idx + 3 == parts.size())
        {
            long n1 = parts[idx].to_long();
            long n2 = parts[idx + 1].to_long();
            long n3 = parts[idx + 2].to_long();

            //
            // Libtool actually hacks this differently for each
            // operating system.  Sheesh.  This code un-hacks the Linux
            // jiggery pokery.  Worry about other OSs when someone asks.
            //
            version_info = version_info_unsplit(n1 + n2, n3, n2);
        }
    }

    //
    // Run the nm -D command to extract the symbols
    // and then read the symbols from the temporary file.
    //
    os_become_orig();
    nstring temp_file(os_edit_filename(0));
    nstring command = "nm -D " + path.quote_shell() + " > " +
        temp_file.quote_shell();
    int mode = OS_EXEC_FLAG_ERROK + OS_EXEC_FLAG_SILENT;
    if (os_execute_retcode(command, mode, ".") == 0)
    {
        FILE *fp = fopen(temp_file.c_str(), "r");
        if (fp)
        {
            for (;;)
            {
                char line[1000];
                if (!fgets(line, sizeof(line), fp))
                    break;
                nstring_list args;
                args.split(nstring(line), " ", true);
                if (args.size() == 3 && extsym(args[1]))
                    symbols.push_back(args[2]);
            }
            fclose(fp);
            symbols.sort();
        }
    }
    os_unlink(temp_file);
    os_become_undo();
}


static nstring
massage_library_filename(const nstring &fn)
{
    nstring_list components;
    components.split(fn, "/");
    nstring last = components.back();
    components.pop_back();
    if (components.back() != ".libs")
        components.push_back(".libs");
    components.push_back(last.trim_extension() + ".so");
    return components.unsplit("/");
}


static void
version_info_split(const nstring &s, long &c, long &r, long &a)
{
    nstring_list parts;
    parts.split(s, ":");
    c = parts.size() < 1 ? 0 : parts[0].to_long();
    if (c < 0)
        c = 0;
    r = parts.size() < 2 ? 0 : parts[1].to_long();
    if (r < 0)
        r = 0;
    a = parts.size() < 3 ? 0 : parts[2].to_long();
    if (a < 0)
        a = 0;
    else if (a > r)
        a = r;
}


bool
validation_version_info::run(change::pointer cp)
{
    if (cp->is_a_branch())
    {
        // For this to be true, the branch is still being developed.
        // When a branch is being integrated this will be false (but
        // cp->was_a_branch() will still be true).
        return true;
    }

    nstring library_filename =
        cp->pconf_attributes_find("aede-policy:version-info:library");
    if (library_filename.empty())
    {
        nstring name(cp->pp->trunk_get()->name_get());
        library_filename = "lib" + name + "/lib" + name + ".la";
    }

    nstring ugly_library_filename = massage_library_filename(library_filename);

    nstring version_info =
        cp->pconf_attributes_find("aemakegen:version-info");
    if (version_info.empty())
        version_info = "0:0:0";

    //
    // we want two versions of the library: the new one, and the ancestor one,
    // on the assumption that the ancestor one
    // was the one previosuly released.
    //
    // It is not an error if the libraries do not exist yet.
    //

    project *ancestor = cp->pp;
    if (cp->is_being_developed() && !ancestor->is_a_trunk())
        ancestor = ancestor->parent_get();
    string_list_ty old_search_path;
    ancestor->search_path_get(&old_search_path, true);

    nstring old_version_info;
    nstring_list old_symbols;
    grope_shared_library
    (
        ugly_library_filename,
        old_search_path,
        old_version_info,
        old_symbols
    );

    string_list_ty new_search_path;
    cp->search_path_get(&new_search_path, true);

    nstring new_version_info;
    nstring_list new_symbols;
    grope_shared_library
    (
        ugly_library_filename,
        new_search_path,
        new_version_info,
        new_symbols
    );

    //
    // 1. Start with version information of `0:0:0' for each libtool library.
    //    This has the form C=current:R=revision:A=age
    //
    long old_current = 0;
    long old_revision = 0;
    long old_age = 0;
    version_info_split(old_version_info, old_current, old_revision, old_age);

    //
    // 2. Update the version information only immediately before a public
    //    release of your software.  More frequent updates are
    //    unnecessary, and only guarantee that the current interface
    //    number gets larger faster.
    //
    long test_current = old_current;
    long test_revision = old_revision;
    long test_age = old_age;

    if (!old_symbols.empty())
    {
        //
        // 3. If the library source code has changed at all since the
        //    last update, then increment REVISION
        //
        ++test_revision;

        //
        // 4. If any interfaces have been added, removed, or changed
        //    since the last update, increment CURRENT, and set REVISION
        //    to 0.
        //
        if (old_symbols != new_symbols)
        {
            ++test_current;
            test_revision = 0;
        }

        //
        // 5. If any interfaces have been added since the last public
        //    release, then increment AGE.
        //
        nstring_list additions = new_symbols - old_symbols;
        if (!additions.empty())
            ++test_age;

        //
        // 6. If any interfaces have been removed since the last public
        //    release, then set AGE to 0.
        //
        nstring_list removals = old_symbols - new_symbols;
        if (!removals.empty())
            test_age = 0;
    }
    nstring test_version_info =
        version_info_unsplit(test_current, test_revision, test_age);

    bool result = true;
    if (strverscmp(new_version_info.c_str(), test_version_info.c_str()) < 0)
    {
        sub_context_ty sc;
        sc.var_set_string("File_Name", library_filename);
        sc.var_set_string("Number1", test_version_info);
        sc.var_set_string("Number2", new_version_info);
        change_error
        (
            cp,
            &sc,
            i18n("$filename: version should be $number1, not $number2")
        );
        result = false;
    }
    if (strverscmp(version_info.c_str(), test_version_info.c_str()) < 0)
    {
        sub_context_ty sc;
        sc.var_set_string("File_Name", "aemakegen:version-info");
        sc.var_set_string("Number1", test_version_info);
        sc.var_set_string("Number2", version_info);
        change_error
        (
            cp,
            &sc,
            i18n("$filename: version should be $number1, not $number2")
        );
        result = false;
    }
    return result;
}


// vim: set sw=4 ts=8 et :
