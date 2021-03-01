//
// aegis - project change supervisor
// Copyright (C) 2001-2008, 2011, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/debug.h>
#include <common/now.h>
#include <common/nstring/list.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/col.h>
#include <libaegis/fstate.fmtgen.h>
#include <libaegis/getgr_cache.h>
#include <libaegis/getpw_cache.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/output.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>

#include <aels/list.h>
#include <aels/stack.h>


int show_removed_files = -1;
int show_dot_files = -1;
int recursive_flag;
int long_flag = 0;
int mode_flag = -1;
static output::pointer mode_col;
int attr_flag = -1;
static output::pointer attr_col;
int user_flag = -1;
static output::pointer user_col;
int group_flag = -1;
static output::pointer group_col;
int size_flag = -1;
static output::pointer size_col;
int when_flag = -1;
static output::pointer when_col;
static output::pointer name_col;
static col::pointer col_ptr;
static nstring_list dirs;
static change::pointer cp;
static project *pp;
static time_t oldest;
static time_t youngest;


//
// NAME
//      stat_stack
//
// SYNOPSIS
//      string_ty *stat_stack(string_ty *path, struct stat *st);
//
// DESCRIPTION
//      The stat_stack function is used to walk the directory search
//      stack, looking for the named file.  On success, the stat structure
//      is filled in, and the resolved file name is returned.
//
//      It is a fatal error if the file cannot be found.
//

static nstring
stat_stack(const nstring &path, struct stat &st)
{
    for (size_t j = 0;; ++j)
    {
        nstring dir = stack_nth(j);
        if (dir.empty())
            break;
        nstring resolved_path = os_path_cat(dir, path);

#ifdef S_IFLNK
        if (!glue_lstat(resolved_path.c_str(), &st))
            return resolved_path;
        int errno_old = errno;
        if (errno_old != ENOENT)
        {
            sub_context_ty sc;
            sc.errno_setx(errno_old);
            sc.var_set_string("File_Name", resolved_path);
            sc.fatal_intl(i18n("lstat $filename: $errno"));
            // NOTREACHED
        }
#else
        if (!glue_stat(resolved_path.c_str(), &st))
            return resolved_path;
        int errno_old = errno;
        if (errno_old != ENOENT)
        {
            sub_context_ty sc;
            sc.errno_setx(errno_old);
            sc.var_set_string("File_Name", resolved_path);
            sc.fatal_intl(i18n("stat $filename: $errno"));
            // NOTREACHED
        }
#endif
    }
    sub_context_ty sc;
    sc.errno_setx(ENOENT);
    sc.var_set_string("File_Name", path);
    sc.fatal_intl(i18n("stat $filename: $errno"));
    // NOTREACHED
    return 0;
}


//
// NAME
//      readdir_stack
//
// SYNOPSIS
//      void readdir_stack(string_ty *path, string_list_ty *result);
//
// DESCRIPTION
//      The readdir_stack function is used to walk the directory search
//      stack reading the named directory at each level.  The results are
//      "unioned" together to provide a single view.
//

static void
readdir_stack(const nstring &path, nstring_list &result)
{
    result.clear();
    for (size_t j = 0;; ++j)
    {
        nstring dir = stack_nth(j);
        if (!dir)
            break;
        nstring s = os_path_cat(dir, path);
        if (read_whole_dir__wla(s, result))
        {
            int errno_old = errno;
            if (errno_old == ENOENT)
            {
                continue;
            }
            sub_context_ty *scp = sub_context_new();
            sub_errno_setx(scp, errno_old);
            sub_var_set_string(scp, "File_Name", path);
            fatal_intl(scp, i18n("read $filename: $errno"));
            // NOTREACHED
        }
    }
}


static void
print_mode_column(const struct stat &st)
{
    //
    // First the type indicator
    //
    if (S_ISDIR(st.st_mode))
        mode_col->fputc('d');
    else if (S_ISCHR(st.st_mode))
        mode_col->fputc('c');
    else if (S_ISBLK(st.st_mode))
        mode_col->fputc('b');
#ifdef S_ISFIFO
    else if (S_ISFIFO(st.st_mode))
        mode_col->fputc('p');
#endif
#ifdef S_ISLNK
    else if (S_ISLNK(st.st_mode))
        mode_col->fputc('l');
#endif
#ifdef S_ISSOCK
    else if (S_ISSOCK(st.st_mode))
        mode_col->fputc('s');
#endif
    else
        mode_col->fputc('-');

    //
    // Now the user bits
    //
    if (st.st_mode & S_IRUSR)
        mode_col->fputc('r');
    else
        mode_col->fputc('-');
    if (st.st_mode & S_IWUSR)
        mode_col->fputc('w');
    else
        mode_col->fputc('-');
    if (st.st_mode & S_IXUSR)
    {
        if (st.st_mode & S_ISUID)
            mode_col->fputc('s');
        else
            mode_col->fputc('x');
    }
    else
    {
        if (st.st_mode & S_ISUID)
            mode_col->fputc('S');
        else
            mode_col->fputc('-');
    }

    //
    // Now the group bits
    //
    if (st.st_mode & S_IRGRP)
        mode_col->fputc('r');
    else
        mode_col->fputc('-');
    if (st.st_mode & S_IWGRP)
        mode_col->fputc('w');
    else
        mode_col->fputc('-');
    if (st.st_mode & S_IXGRP)
    {
        if (st.st_mode & S_ISGID)
            mode_col->fputc('s');
        else
            mode_col->fputc('x');
    }
    else
    {
        if (st.st_mode & S_ISGID)
            mode_col->fputc('S');
        else
            mode_col->fputc('-');
    }

    //
    // Now the other bits
    //
    if (st.st_mode & S_IROTH)
        mode_col->fputc('r');
    else
        mode_col->fputc('-');
    if (st.st_mode & S_IWOTH)
        mode_col->fputc('w');
    else
        mode_col->fputc('-');
    if (st.st_mode & S_IXOTH)
    {
#ifdef S_ISVTX
        if (st.st_mode & S_ISVTX)
            mode_col->fputc('t');
        else
#endif
            mode_col->fputc('x');
    }
    else
    {
#ifdef S_ISVTX
        if (st.st_mode & S_ISVTX)
            mode_col->fputc('T');
        else
#endif
            mode_col->fputc('-');
    }
}


//
// NAME
//      list_file
//
// SYNOPSIS
//      void list_file(string_ty *long_name, string-ty *short_name,
//              struct stat *st);
//
// DESCRIPTION
//      The list_file function is used to print the information about
//      the file onto the columnised output.
//

static void
list_file(const nstring &long_name, const nstring &short_name, struct stat &st,
    const nstring &resolved_name)
{
    fstate_src_ty *c_src =
        cp ? cp->file_find(long_name, view_path_first) : 0;
    if (c_src && c_src->about_to_be_created_by)
        c_src = 0;
    if (c_src)
    {
        switch (c_src->action)
        {
        case file_action_remove:
            if (show_removed_files <= 0)
            {
                return;
            }
            break;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
        case file_action_transparent:
#ifndef DEBUG
        default:
#endif
            // should be file_action_remove
            assert(!c_src->deleted_by);
            if (c_src->deleted_by)
            {
                if (show_removed_files <= 0)
                {
                    return;
                }
            }
            break;
        }
    }

    fstate_src_ty *p_src = pp->file_find(long_name.get_ref(), view_path_simple);
    if (p_src)
    {
        assert(!p_src->about_to_be_created_by); // hidden by viewpath
        switch (p_src->action)
        {
        case file_action_remove:
            if (show_removed_files <= 0)
            {
                return;
            }
            break;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
        case file_action_transparent:
#ifndef DEBUG
        default:
#endif
            // should be file_action_remove
            assert(!p_src->deleted_by);
            if (p_src->deleted_by)
            {
                if (show_removed_files <= 0)
                {
                    return;
                }
            }
            break;
        }
    }

    nstring link;
    if (mode_col)
    {
        if (S_ISLNK(st.st_mode))
        {
            //
            // If a link points to a relative file of exactly
            // the same name, then it's a link to a baseline.
            // Pretend it isn't a line.
            //
            os_become_orig();
            link = os_readlink(resolved_name);
            os_become_undo();

            if (link[0] != '/')
            {
                os_become_orig();
                link = os_path_cat(os_dirname(resolved_name), link);
                os_become_undo();
            }
            nstring s2 = stack_relative(link);
            if (!s2.empty())
            {
                if (s2 == long_name)
                {
                    // nuke the link-ness
                    st.st_mode = (st.st_mode & ~S_IFMT) | S_IFREG;
                    link.clear();
                }
            }
        }
        print_mode_column(st);
    }

    if (attr_col)
    {
        if (c_src)
            attr_col->fputc('C');
        else if (p_src)
            attr_col->fputc('P');
        else
            attr_col->fputc('-');

        if (c_src)
        {
            char action_indicator = '?';
            switch (c_src->action)
            {
            case file_action_create:
                action_indicator = 'c';
                break;

            case file_action_modify:
                action_indicator = 'm';
                break;

            case file_action_remove:
                action_indicator = 'r';
                break;

            case file_action_insulate:
                action_indicator = 'i';
                break;

            case file_action_transparent:
                action_indicator = 't';
                break;
            }
            attr_col->fputc(action_indicator);
        }
        else
        {
            attr_col->fputc('-');
        }

        fstate_src_ty *src = c_src ? c_src : p_src;
        if (src)
        {
            char usage_indicator = '?';
            switch (src->usage)
            {
            case file_usage_source:
                usage_indicator = 's';
                break;

            case file_usage_config:
                usage_indicator = 'c';
                break;

            case file_usage_build:
                usage_indicator = 'b';
                break;

            case file_usage_test:
                usage_indicator = 't';
                break;

            case file_usage_manual_test:
                //
                // should this be 'm' ?
                // all the others are lower case
                //
                usage_indicator = 'T';
                break;
            }
            attr_col->fputc(usage_indicator);
        }
        else
        {
            attr_col->fputc('-');
        }
    }
    if (user_col)
    {
        struct passwd *pw = getpwuid_cached(st.st_uid);
        if (pw)
            user_col->fputs(pw->pw_name);
        else
            user_col->fprintf("%d", (int)st.st_uid);
    }
    if (group_col)
    {
        struct group *gr = getgrgid_cached(st.st_uid);
        if (gr)
            group_col->fputs(gr->gr_name);
        else
            group_col->fprintf("%d", (int)st.st_gid);
    }
    if (size_col)
    {
        size_col->fprintf("%8ld", (long)st.st_size);
    }
    if (when_col)
    {
        struct tm *the_time = localtime(&st.st_mtime);
        char buffer[100];
        if (st.st_mtime < oldest || st.st_mtime > youngest)
            strftime(buffer, sizeof(buffer), "%b %d  %Y", the_time);
        else
            strftime(buffer, sizeof(buffer), "%b %d %H:%M", the_time);
        when_col->fputs(buffer);
    }

    //
    // output the name
    //
    name_col->fputs(short_name);
    if (!link.empty() && long_flag)
    {
        name_col->fputs(" -> ");
        name_col->fputs(link);
    }
    col_ptr->eoln();
}


//
// NAME
//      list_dir
//
// SYNOPSIS
//      void list_dir(string_ty *dirname);
//
// DESCRIPTION
//      The list_dir function is used to list the contents of a directory
//      onto the columnised output.
//

static void
list_dir(const nstring &dirname)
{
    os_become_orig();
    nstring_list wl;
    readdir_stack(dirname, wl);
    os_become_undo();
    wl.sort();
    nstring_list more_dirs;
    for (size_t j = 0; j < wl.size(); ++j)
    {
        if (show_dot_files <= 0 && wl[j][0] == '.')
            continue;
        nstring s = os_path_cat(dirname, wl[j]);
        os_become_orig();
        struct stat st;
        nstring resolved_path = stat_stack(s, st);
        os_become_undo();
        if (recursive_flag && (st.st_mode & S_IFMT) == S_IFDIR)
            more_dirs.push_back(s);
        list_file(s, wl[j], st, resolved_path);
    }
    dirs.push_front(more_dirs);
}


//
// NAME
//      list
//
// SYNOPSIS
//      void list(string_list_ty *paths);
//
// DESCRIPTION
//      The list function is used to list the named files and the contents
//      of the named directories onto the columnised output.
//

void
list(const nstring_list &paths, project *a_pp, const change::pointer &a_cp)
{
    time_t when = now();
    oldest = when - 6L * 30 * 24 * 60 * 60;
    youngest = when + 6L * 30 * 24 * 60 * 60;
    pp = a_pp;
    cp = a_cp;

    if (long_flag > 0)
    {
        if (mode_flag < 0)
            mode_flag = 1;
        if (attr_flag < 0)
            attr_flag = 1;
        if (user_flag < 0)
            user_flag = 1;
        if (group_flag < 0)
            group_flag = 1;
        if (size_flag < 0)
            size_flag = 1;
        if (when_flag < 0)
            when_flag = 1;
    }

    col_ptr = col::open((string_ty *)0);
    int column = 0;
    if (mode_flag > 0)
    {
        int width = 10;
        mode_col = col_ptr->create(column, column + width, "Mode");
        column += width + 1;
    }
    if (attr_flag)
    {
        int width = 4;
        attr_col = col_ptr->create(column, column + width, "Attr");
        column += width + 1;
    }
    if (user_flag > 0)
    {
        int width = 8;
        user_col =
            col_ptr->create(column, column + width, "User\n--------");
        column += width + 1;
    }
    if (group_flag > 0)
    {
        int width = 8;
        group_col =
            col_ptr->create(column, column + width, "Group\n--------");
        column += width + 1;
    }
    if (size_flag > 0)
    {
        int width = 8;
        size_col =
            col_ptr->create(column, column + width, "Size\n--------");
        column += width + 1;
    }
    if (when_flag > 0)
    {
        int width = 12;
        when_col =
            col_ptr->create(column, column + width, "When\n------------");
        column += width + 1;
    }
    name_col = col_ptr->create(column, 0, "File Name\n-----------");
    nstring t1 = "Project " + nstring(pp->name_get()).quote_c();
    if (cp)
        t1 += nstring::format(", Change %ld", magic_zero_decode(cp->number));
    nstring t2 = "Annotated Listing";
    col_ptr->title(t1, t2);

    bool need_eject = false;
    for (size_t j = 0; j < paths.size(); ++j)
    {
        nstring path = paths[j];
        os_become_orig();
        struct stat st;
        nstring resolved_path = stat_stack(path, st);
        os_become_undo();
        if ((st.st_mode & S_IFMT) == S_IFDIR)
            dirs.push_back(path);
        else
        {
            list_file(path, path, st, resolved_path);
            need_eject = true;
        }
    }

    while (!dirs.empty())
    {
        if (need_eject)
            col_ptr->eject();
        nstring path = dirs[0];
        dirs.remove(path);
        col_ptr->title
        (
            t1,
            "Annotated " + path.quote_c() + " Directory Listing"
        );
        list_dir(path);
        need_eject = true;
    }

    mode_col.reset();
    attr_col.reset();
    user_col.reset();
    group_col.reset();
    size_col.reset();
    when_col.reset();
    name_col.reset();
    col_ptr.reset();
}


// vim: set ts=8 sw=4 et :
