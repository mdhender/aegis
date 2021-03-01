//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2002, 2004-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
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

#include <libaegis/dir.h>
#include <common/error.h>
#include <aeimport/format.h>
#include <aeimport/format/private.h>
#include <aeimport/format/search_list.h>
#include <common/mem.h>
#include <libaegis/os.h>


void
format_delete(format_ty *fp)
{
    assert(fp);
    assert(fp->vptr);
    if (fp->vptr->destructor)
        fp->vptr->destructor(fp);
    fp->vptr = 0; // paranoia
    mem_free(fp);
}


struct context_ty
{
    format_ty       *fp;
    string_ty       *base;
    format_search_list_ty *fslp;
};


static void
searcher(void *arg, dir_walk_message_ty msg, string_ty *filename,
    const struct stat *)
{
    static string_ty *the_config_file;

    if (!the_config_file)
        the_config_file = str_from_c("config");
    if (msg == dir_walk_file)
    {
        context_ty      *context;
        format_ty       *fp;

        context = (context_ty *)arg;
        fp = context->fp;
        if (fp->vptr->is_a_candidate(fp, filename))
        {
            string_ty       *relative;
            string_ty       *sanitized;
            format_search_ty *fsp;
            format_version_ty *fvp;

            relative = os_below_dir(context->base, filename);
            sanitized = format_sanitize(fp, relative, 1);
            str_free(relative);
            if (str_equal(sanitized, the_config_file))
                fvp = 0;
            else
                fvp = fp->vptr->read_versions(fp, filename, sanitized);
            fsp = format_search_new();
            fsp->filename_physical = str_copy(filename);
            fsp->filename_logical = sanitized;
            fsp->root = fvp;
            format_search_list_append(context->fslp, fsp);
        }
    }
}


format_search_list_ty *
format_search(format_ty *fp, string_ty *base)
{
    context_ty      context;

    context.fp = fp;
    context.base = base;
    context.fslp = format_search_list_new();
    assert(fp->vptr);
    assert(fp->vptr->is_a_candidate);
    assert(fp->vptr->sanitize);
    assert(fp->vptr->read_versions);
    dir_walk(base, searcher, &context);
    return context.fslp;
}


string_ty *
format_history_put(format_ty *fp)
{
    assert(fp);
    assert(fp->vptr);
    assert(fp->vptr->history_put);
    return fp->vptr->history_put(fp);
}


string_ty *
format_history_get(format_ty *fp)
{
    assert(fp);
    assert(fp->vptr);
    assert(fp->vptr->history_get);
    return fp->vptr->history_get(fp);
}


string_ty *
format_history_query(format_ty *fp)
{
    assert(fp);
    assert(fp->vptr);
    assert(fp->vptr->history_query);
    return fp->vptr->history_query(fp);
}


string_ty *
format_diff(format_ty *fp)
{
    assert(fp);
    assert(fp->vptr);
    assert(fp->vptr->diff);
    return fp->vptr->diff(fp);
}


string_ty *
format_merge(format_ty *fp)
{
    assert(fp);
    assert(fp->vptr);
    assert(fp->vptr->merge);
    return fp->vptr->merge(fp);
}


void
format_unlock(format_ty *fp, string_ty *filename)
{
    assert(fp);
    assert(fp->vptr);
    assert(fp->vptr->unlock);
    fp->vptr->unlock(fp, filename);
}


string_ty *
format_sanitize(format_ty *fp, string_ty *filename, int last_part)
{
    assert(fp);
    assert(fp->vptr);
    assert(fp->vptr->sanitize);
    return fp->vptr->sanitize(fp, filename, last_part);
}
