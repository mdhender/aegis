//
//      aegis - project change supervisor
//      Copyright (C) 1991-1995, 1997-1999, 2001-2008, 2011, 2012 Peter Miller
//      Copyright (C) 2007-2009 Walter Franzini
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

#include <common/ac/assert.h>
#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/unistd.h>

#include <common/env.h>
#include <common/error.h>
#include <common/libdir.h>
#include <common/mem.h>
#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/commit.h>
#include <libaegis/gonzo.h>
#include <libaegis/gstate.fmtgen.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>
#include <libaegis/ustate.fmtgen.h>

struct gonzo_ty
{
    string_ty       *dir;
    string_ty       *gstate_filename;
    gstate_ty       *gstate_data;
    int             is_a_new_file;
    int             modified;
    long            lock_magic;
    int             temporary;
};

static size_t   ngonzos;
static size_t   ngonzos_max;
static gonzo_ty **gonzo;
static int      done_tail;


static user_ty::pointer
gonzo_user(void)
{
    static user_ty::pointer u;

    if (!u)
    {
        u = user_ty::create(configured_aegis_uid(), configured_aegis_gid());
        u->umask_set(022);

        if (u->get_uid() >= AEGIS_MIN_UID)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set_long(scp, "Number1", u->get_uid());
            sub_var_set_long(scp, "Number2", AEGIS_MIN_UID);
            fatal_intl
            (
                scp,
     i18n("AEGIS_USER_UID ($number1) vs AEGIS_MIN_UID ($number2) misconfigured")
            );
            // NOTREACHED
            sub_context_delete(scp);
        }
        if (u->get_gid() >= AEGIS_MIN_GID)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set_long(scp, "Number1", u->get_gid());
            sub_var_set_long(scp, "Number2", AEGIS_MIN_GID);
            fatal_intl
            (
                scp,
     i18n("AEGIS_USER_GID ($number1) vs AEGIS_MIN_GID ($number2) misconfigured")
            );
            // NOTREACHED
            sub_context_delete(scp);
        }
    }
    return u;
}


static int
is_temporary(string_ty *s)
{
    return !!strstr(s->str_text, "/tmp/");
}


void
gonzo_library_append(const char *s)
{
    gonzo_ty        *gp;
    string_ty       *tmp;
    string_ty       *dir;

    //
    // resolve the pathname
    //
    trace(("gonzo_library_append(s = \"%s\")\n{\n", s));
    assert(s);
    assert(!done_tail);
    tmp = str_from_c(s);
    os_become_orig();
    dir = os_pathname(tmp, 1);
    os_become_undo();
    str_free(tmp);

    //
    // append the new entry to the end of the list
    //
    gp = (gonzo_ty *)mem_alloc_clear(sizeof(gonzo_ty));
    gp->dir = dir;
    gp->temporary = is_temporary(dir);
    gp->gstate_filename = str_format("%s/state", gp->dir->str_text);
    trace(("gonzo = %p;\n", gonzo));
    trace(("ngonzos = %ld;\n", (long)ngonzos));

    if (ngonzos >= ngonzos_max)
    {
        ngonzos_max = ngonzos_max * 2 + 4;
        gonzo_ty **new_gonzo = new gonzo_ty * [ngonzos_max];
        for (size_t k = 0; k < ngonzos; ++k)
            new_gonzo[k] = gonzo[k];
        delete [] gonzo;
        gonzo = new_gonzo;
    }
    gonzo[ngonzos++] = gp;
    trace(("}\n"));
}


static void
lock_sync(gonzo_ty *gp)
{
    trace(("lock_sync(gp = %p)\n{\n", gp));
    long n = lock_magic();
    if (gp->lock_magic == n)
        goto ret;
    assert(!gp->modified);
    gp->lock_magic = n;

    if (gp->gstate_data && !gp->is_a_new_file)
    {
        gstate_type.free(gp->gstate_data);
        gp->gstate_data = 0;
    }
    ret:
    trace(("}\n"));
}


static gstate_ty *
gonzo_gstate_get(gonzo_ty *gp)
{
    trace(("gonzo_gstate_get(gp = %p)\n{\n", gp));
    lock_sync(gp);
    if (!gp->gstate_data)
    {
        //
        // The problem here is if we get EACCES (Permission denied)
        // for the lstat to see if the file exists.  For now, we say
        // that EACCESS is OK (that's what the second argument says)
        // because most of the time this means we simply ignore global
        // state files we can't read.  But if there is a new project
        // being created (the only time one of these ignored files will
        // need writing) when the "new" file is opened, the EACCES will
        // happen again, so nothing will be damaged, and the user will
        // see an appropriate error.
        //
        gonzo_become();
        gp->is_a_new_file = !os_exists(gp->gstate_filename, true);
        if (!gp->is_a_new_file)
        {
            os_chown_check
            (
                gp->gstate_filename,
                0644,
                gonzo_user()->get_uid(),
                (gp->temporary ? -1 : gonzo_user()->get_gid())
            );
            gp->gstate_data = gstate_read_file(gp->gstate_filename);
        }
        else
            gp->gstate_data = (gstate_ty *)gstate_type.alloc();
        if (!gp->gstate_data->where)
            gp->gstate_data->where =
                (gstate_where_list_ty *)gstate_where_list_type.alloc();
        gonzo_become_undo();
    }
    trace(("return %p;\n", gp->gstate_data));
    trace(("}\n"));
    return gp->gstate_data;
}


//
// NAME
//      construct_library_directory
//
// SYNOPSIS
//      void construct_library_directory(gonzo_ty *gp);
//
// DESCRIPTION
//      The construct_library_directory function is used to
//      construct missing elements of the aegis library search path.
//      These elements must be constructed with great care so that
//      they may work on NFS mounted disks of "data-less" clients.
//
//      The path up to, but not including, the library directory must
//      be owned by the executing user.  The actions will fail if the
//      executing user does not have sufficient permissions, as
//      one would expect.
//
//      The library directory itself must be owned by AEGIS_USER.
//      This is to defend against tampering and ignorance.
//      Having this change of ownership requires some fast footwork
//      with chmod's to allow AEGIS_USER temporary access.
//
// ARGUMENTS
//      gp      - library directory to act upon
//
// CAVEAT
//      The chown() system call will not work on data-less clients,
//      because the remote hosts which owns the NFS mounted disks
//      will rarely trust the local "root" user, and map all
//      foreign "root" users to the "unknown" user instead.
//

static void
construct_library_directory(gonzo_ty *gp)
{
    int             exists;
    string_ty       *above;
    string_ty       *above2;
    string_ty       *root;
    int             mode;

    //
    // If the library directory already exists,
    // then we need do nothing.
    //
    os_become_orig();
    exists = os_exists(gp->dir);
    if (exists)
    {
        os_become_undo();
        return;
    }

    //
    // Construct all directories up to,
    // but not including, the library directory,
    // if they do not already exist.
    // Construct these directories as the original user,
    // possibly discovering relevant permission problems.
    //
    above = os_dirname(gp->dir);
    above2 = str_n_from_c(above->str_text + 1, above->str_length - 1);
    root = str_from_c("/");
    os_mkdir_between(root, above2, 0755); // NOT setgid
    str_free(root);
    str_free(above2);

    //
    // Get the mode of the directory containing the library directory,
    // so that we may restore it later.  Change the mode to world
    // writable, so that the library directory itself may be created
    // belonging to gonzo.
    //
    mode = os_chmod_query(above);
    mode |= 0111; // minimum: all may search
    undo_chmod_errok(above, mode);
    os_chmod(above, mode | 0777);
    os_become_undo();

    //
    // Create the library directory itself belonging to gonzo,
    // and make sure it is world accessable.
    //      (must be world writable in testing situations)
    //
    gonzo_become();
    os_mkdir(gp->dir, 0755);
    if (gp->temporary)
        os_chmod(gp->dir, 0777);
    gonzo_become_undo();

    //
    // Restore permissions for the containing directory.
    //
    os_become_orig();
    os_chmod(above, mode);
    os_become_undo();
    str_free(above);
}


static void
gonzo_gstate_write_sub(gonzo_ty *gp)
{
    string_ty       *filename_new;
    string_ty       *filename_old;
    static int      count;

    if (!gp->modified)
        return;
    trace(("gonzo_gstate_write_sub()\n{\n"));
    assert(gp->gstate_data);
    assert(gp->gstate_filename);
    filename_new = str_format("%s,%d", gp->gstate_filename->str_text, ++count);
    filename_old = str_format("%s,%d", gp->gstate_filename->str_text, ++count);
    if (gp->is_a_new_file)
    {
        construct_library_directory(gp);
        gonzo_become();
        undo_unlink_errok(filename_new);
        gstate_write_file(filename_new, gp->gstate_data, 0);
        commit_rename(filename_new, gp->gstate_filename);
        os_chmod(filename_new, 0644);
        gonzo_become_undo();
    }
    else
    {
        gonzo_become();
        undo_unlink_errok(filename_new);
        gstate_write_file(filename_new, gp->gstate_data, 0);
        commit_rename(gp->gstate_filename, filename_old);
        commit_rename(filename_new, gp->gstate_filename);
        commit_unlink_errok(filename_old);
        os_chmod(filename_new, 0644);
        gonzo_become_undo();
    }
    str_free(filename_new);
    str_free(filename_old);
    gp->modified = 0;
    trace(("}\n"));
}


static void
do_tail(void)
{
    string_ty       *s1;
    string_ty       *s2;
    char            *cp;
    size_t          j;
    size_t          max;

    //
    // only do this once
    //
    if (done_tail)
        return;

    //
    // fetch the environment variable
    //
    trace(("do_tail()\n{\n"));
    cp = getenv("AEGIS_PATH");
    if (!cp)
    {
        cp = getenv("AEGIS");
        if (cp)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set_charstar(scp, "Name1", "AEGIS");
            sub_var_set_charstar(scp, "Name2", "AEGIS_PATH");
            verbose_intl
            (
                scp,
            i18n("warning: $name1 is obsolete, use $name2 environment variable")
            );
            sub_context_delete(scp);
        }
    }
    if (cp)
    {
        s1 = str_from_c(cp);
        string_list_ty path;
        path.split(s1, ":", true);
        str_free(s1);
        for (j = 0; j < path.nstrings; ++j)
        {
            s1 = path.string[j];
            if (!os_testing_mode() || is_temporary(s1))
                gonzo_library_append(s1->str_text);
        }
    }

#ifndef SINGLE_USER
    if (os_testing_mode())
    {
        if (!ngonzos)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set_charstar
            (
                scp,
                "Name",
                arglex_token_name(arglex_token_library)
            );
            fatal_intl(scp, i18n("test mode needs $name"));
            // NOTREACHED
            sub_context_delete(scp);
        }
        max = ngonzos;
    }
    else
#endif
    {
        //
        // always have the system one last
        //      (this is where locks are taken)
        //
        gonzo_library_append(configured_comdir());
        max = ngonzos - 1;
    }

    //
    // build a new environment variable
    //
    string_list_ty path;
    for (j = 0; j < max; ++j)
        path.push_back(gonzo[j]->dir);
    s1 = path.unsplit(":");
    env_set("AEGIS_PATH", s1->str_text);
    str_free(s1);

    //
    // zap the obsolete one, if present
    //
    s1 = str_from_c(progname_get());
    s2 = str_upcase(s1);
    str_free(s1);
    env_unset(s2->str_text);
    str_free(s2);

    //
    // do not repeat
    //
    done_tail = 1;
    trace(("}\n"));
}


static gonzo_ty *
gonzo_nth(size_t j)
{
    gonzo_ty        *result;

    trace(("gonzo_nth(j = %ld)\n{\n", (long)j));
    do_tail();
    if (j >= ngonzos)
        result = 0;
    else
        result = gonzo[j];
    trace(("return %p;\n", result));
    trace(("}\n"));
    return result;
}


void
gonzo_gstate_write(void)
{
    size_t          j;
    gonzo_ty        *gp;

    trace(("gonzo_gstate_write()\n{\n"));
    for (j = 0;; ++j)
    {
        gp = gonzo_nth(j);
        if (!gp)
            break;
        gonzo_gstate_write_sub(gp);
    }
    trace(("}\n"));
}


static string_ty *
gonzo_project_home_path_sub(gonzo_ty *gp, string_ty *name)
{
    gstate_ty       *gstate_data;
    size_t          j;
    string_ty       *result;
    gstate_where_ty *addr =         0;

    //
    // find the project in the gstate
    //
    trace(("gonzo_project_home_path_sub(gp = %p, name = \"%s\")\n{\n",
        gp, name->str_text));
    gstate_data = gonzo_gstate_get(gp);
    assert(gstate_data->where);
    result = 0;
    for (j = 0; j < gstate_data->where->length; ++j)
    {
        addr = gstate_data->where->list[j];
        if (addr->alias_for)
            continue;
        if (str_equal(addr->project_name, name))
        {
            result = addr->directory;
            break;
        }
    }
    trace(("return \"%s\";\n", (result ? result->str_text : "")));
    trace(("}\n"));
    return result;
}


string_ty *
gonzo_project_home_path_from_name(string_ty *name)
{
    gonzo_ty        *gp;
    size_t          j;
    string_ty       *result;
    string_ty       *s;

    //
    // find the project in the gstate list
    //
    trace(("gonzo_project_home_path_from_name(name = \"%s\")\n{\n",
        name->str_text));
    result = 0;
    for (j = 0;; ++j)
    {
        gp = gonzo_nth(j);
        if (!gp)
            break;
        s = gonzo_project_home_path_sub(gp, name);
        if (s)
        {
            result = s;
            break;
        }
    }
    trace(("return \"%s\";\n", (result ? result->str_text : "")));
    trace(("}\n"));
    return result;
}


static string_ty *
gonzo_alias_to_actual_sub(gonzo_ty *gp, string_ty *name)
{
    gstate_ty       *gstate_data;
    size_t          j;
    string_ty       *result;

    //
    // find the project in the gstate
    //
    trace(("gonzo_alias_to_actual_sub(gp = %p, name = \"%s\")\n{\n",
        gp, name->str_text));
    gstate_data = gonzo_gstate_get(gp);
    assert(gstate_data->where);
    result = 0;
    for (j = 0; j < gstate_data->where->length; ++j)
    {
        gstate_where_ty *addr;

        addr = gstate_data->where->list[j];
        if (!addr->alias_for)
            continue;
        if (str_equal(addr->project_name, name))
        {
            result = addr->alias_for;
            break;
        }
    }
    trace(("return \"%s\";\n", (result ? result->str_text : "")));
    trace(("}\n"));
    return result;
}


string_ty *
gonzo_alias_to_actual(string_ty *name)
{
    gonzo_ty        *gp;
    size_t          j;
    string_ty       *result;
    string_ty       *s;

    //
    // find the project in the gstate list
    //
    trace(("gonzo_alias_to_actual(name = \"%s\")\n{\n", name->str_text));
    result = 0;
    for (j = 0;; ++j)
    {
        gp = gonzo_nth(j);
        if (!gp)
            break;
        s = gonzo_alias_to_actual_sub(gp, name);
        if (s)
        {
            result = s;
            break;
        }
    }
    trace(("return \"%s\";\n", (result ? result->str_text : "")));
    trace(("}\n"));
    return result;
}


void
gonzo_project_list(string_list_ty *result)
{
    size_t          n;
    size_t          j;
    gonzo_ty        *gp;
    gstate_ty       *gstate_data;

    trace(("gonzo_project_list(result = %p)\n{\n", result));
    result->clear();
    for (n = 0;; ++n)
    {
        gp = gonzo_nth(n);
        if (!gp)
            break;

        //
        // read the gstate file
        //
        gstate_data = gonzo_gstate_get(gp);

        //
        // list the projects
        //
        assert(gstate_data->where);
        for (j = 0; j < gstate_data->where->length; ++j)
        {
            gstate_where_ty *addr;

            addr = gstate_data->where->list[j];
            if (addr->alias_for)
                continue;
            result->push_back_unique(addr->project_name);
        }
    }
    trace(("}\n"));
}


void
gonzo_alias_list(string_list_ty *result)
{
    size_t          n;
    size_t          j;
    gonzo_ty        *gp;
    gstate_ty       *gstate_data;

    trace(("gonzo_project_list(result = %p)\n{\n", result));
    result->clear();
    for (n = 0;; ++n)
    {
        gp = gonzo_nth(n);
        if (!gp)
            break;

        //
        // read the gstate file
        //
        gstate_data = gonzo_gstate_get(gp);

        //
        // list the projects
        //
        assert(gstate_data->where);
        for (j = 0; j < gstate_data->where->length; ++j)
        {
            gstate_where_ty *addr;

            addr = gstate_data->where->list[j];
            if (!addr->alias_for)
                continue;
            result->push_back_unique(addr->project_name);
        }
    }
    trace(("}\n"));
}


void
gonzo_project_list_user(const nstring &uname, nstring_list &result)
{
    trace(("gonzo_project_list_user(uname = %s, result = @%p)\n{\n",
        uname.quote_c().c_str(), &result));
    result.clear();
    for (size_t n = 0;; ++n)
    {
        gonzo_ty *gp = gonzo_nth(n);
        if (!gp)
            break;

        //
        // check out the ustate
        //
        nstring ustate_path =
            nstring::format("%s/user/%s", gp->dir->str_text, uname.c_str());
        trace(("ustate_path = %s;\n", ustate_path.quote_c().c_str()));
        gonzo_become();
        bool ok = os_exists(ustate_path);
        gonzo_become_undo();
        if (!ok)
            continue;

        gonzo_become();
        ustate_ty *ustate_data = ustate_read_file(ustate_path);
        gonzo_become_undo();
        if (!ustate_data->own)
        {
            ustate_data->own =
                (ustate_own_list_ty *)ustate_own_list_type.alloc();
        }

        //
        // collect all projects this user owns changes in
        //
        for (size_t j = 0; j < ustate_data->own->length; ++j)
        {
            nstring pn(ustate_data->own->list[j]->project_name);
            trace(("remember %s;\n", pn.quote_c().c_str()));
            result.push_back_unique(pn);
        }
        ustate_type.free(ustate_data);
    }
    trace(("found %zd items\n", result.size()));
    trace(("}\n"));
}


static int
sort_gstate_cmp(const void *va, const void *vb)
{
    const gstate_where_ty *a = *(const gstate_where_ty **)va;
    const gstate_where_ty *b = *(const gstate_where_ty **)vb;
    return strcmp(a->project_name->str_text, b->project_name->str_text);
}


static void
sort_gstate_where(gonzo_ty *gp)
{
    assert(gp);
    gstate_ty *gstate_data = gonzo_gstate_get(gp);
    assert(gstate_data);
    assert(gstate_data->where);
    assert(gstate_data->where->length > 0);
    qsort
    (
        gstate_data->where->list,
        gstate_data->where->length,
        sizeof(gstate_data->where->list[0]),
        sort_gstate_cmp
    );
}


void
gonzo_project_add(project *pp)
{
    trace(("gonzo_project_add(pp = %p)\n{\n", pp));
    gonzo_ty *gp = gonzo_nth(0);
    gstate_ty *gstate_data = gonzo_gstate_get(gp);
    meta_type *type_p = 0;
    gstate_where_ty **addr_p =
        (gstate_where_ty **)
        gstate_where_list_type.list_parse(gstate_data->where, &type_p);
    assert(type_p == &gstate_where_type);
    gstate_where_ty *addr = (gstate_where_ty *)gstate_where_type.alloc();
    *addr_p = addr;
    trace_pointer(addr);
    addr->project_name = project_name_get(pp).get_ref_copy();
    addr->directory = str_copy(pp->home_path_get());
    gp->modified = 1;
    sort_gstate_where(gp);
    trace(("}\n"));
}


void
gonzo_alias_add(project *pp, string_ty *name)
{
    trace(("gonzo_alias_add(pp = %p)\n{\n", pp));
    gonzo_ty *gp = gonzo_nth(0);
    gstate_ty *gstate_data = gonzo_gstate_get(gp);
    meta_type *type_p = 0;
    gstate_where_ty **addr_p =
        (gstate_where_ty **)
        gstate_where_list_type.list_parse(gstate_data->where, &type_p);
    assert(type_p == &gstate_where_type);
    gstate_where_ty *addr = (gstate_where_ty *)gstate_where_type.alloc();
    *addr_p = addr;
    trace_pointer(addr);
    addr->project_name = str_copy(name);
    addr->alias_for = project_name_get(pp).get_ref_copy();
    gp->modified = 1;
    sort_gstate_where(gp);
    trace(("}\n"));
}


static int
gonzo_project_delete_sub(gonzo_ty *gp, project *pp)
{
    gstate_ty       *gstate_data;
    size_t          j;
    int             result;

    //
    // find the project in the gstate
    //
    trace(("gonzo_project_delete_sub(gp = %p, pp = %p)\n{\n",
        gp, pp));
    gstate_data = gonzo_gstate_get(gp);
    assert(gstate_data->where);
    result = 0;
    for (j = 0; j < gstate_data->where->length; ++j)
    {
        gstate_where_ty *addr;
        size_t          k;

        addr = gstate_data->where->list[j];
        if (addr->alias_for)
            continue;
        if (nstring(addr->project_name) != project_name_get(pp))
            continue;

        //
        // delete the item from the list
        // (keep the list sorted)
        //
        for (k = j + 1; k < gstate_data->where->length; ++k)
            gstate_data->where->list[k - 1] = gstate_data->where->list[k];
        gstate_data->where->length--;

        //
        // free the item
        //
        gstate_where_type.free(addr);

        //
        // mark this gstate file as modified
        //
        gp->modified = 1;
        result = 1;
        break;
    }
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


static int
is_leading_prefix(string_ty *s1, string_ty *s2)
{
    if (str_equal(s1, s2))
        return 1;
    return
        (
            s1->str_length < s2->str_length
        &&
            ispunct((unsigned char)s2->str_text[s1->str_length])
        &&
            !memcmp(s1->str_text, s2->str_text, s1->str_length)
        );
}


static void
gonzo_alias_delete_destination_sub(gonzo_ty *gp, string_ty *name)
{
    gstate_ty       *gstate_data;
    size_t          j;

    //
    // find the project in the gstate
    //
    trace(("gonzo_alias_delete_destination_sub(gp = %p, name = \"%s\")\n{\n",
           gp, name->str_text));
    gstate_data = gonzo_gstate_get(gp);
    assert(gstate_data->where);
    for (j = 0; j < gstate_data->where->length; ++j)
    {
        gstate_where_ty *addr;
        size_t          k;

        addr = gstate_data->where->list[j];
        if (!addr->alias_for)
            continue;
        if (!is_leading_prefix(name, addr->alias_for))
            continue;

        trace_string (addr->alias_for->str_text);
        //
        // delete the item from the list
        //
        for (k = j + 1; k < gstate_data->where->length; ++k)
            gstate_data->where->list[k - 1] = gstate_data->where->list[k];
        gstate_data->where->length--;
        --j;

        //
        // free the item
        //
        gstate_where_type.free(addr);

        //
        // mark this gstate file as modified
        //
        gp->modified = 1;
    }
    trace(("}\n"));
}


void
gonzo_project_delete(project *pp)
{
    gonzo_ty        *gp;
    long            j;

    trace(("gonzo_project_delete(pp = %p)\n{\n", pp));
    for (j = 0;; ++j)
    {
        gp = gonzo_nth(j);
        if (!gp)
            break;
        gonzo_alias_delete_destination_sub(gp, project_name_get(pp).get_ref());
        if (gonzo_project_delete_sub(gp, pp))
            break;
    }
    trace(("}\n"));
}


static int
gonzo_alias_delete_sub(gonzo_ty *gp, string_ty *name)
{
    gstate_ty       *gstate_data;
    size_t          j;
    int             result;

    //
    // find the project in the gstate
    //
    trace(("gonzo_alias_delete_sub(gp = %p, name = \"%s\")\n{\n",
        gp, name->str_text));
    gstate_data = gonzo_gstate_get(gp);
    assert(gstate_data->where);
    result = 0;
    for (j = 0; j < gstate_data->where->length; ++j)
    {
        gstate_where_ty *addr;
        size_t          k;

        addr = gstate_data->where->list[j];
        if (!addr->alias_for)
            continue;
        if (!str_equal(addr->project_name, name))
            continue;

        //
        // delete the item from the list
        //
        for (k = j + 1; k < gstate_data->where->length; ++k)
            gstate_data->where->list[k - 1] = gstate_data->where->list[k];
        gstate_data->where->length--;

        //
        // free the item
        //
        gstate_where_type.free(addr);

        //
        // mark this gstate file as modified
        //
        gp->modified = 1;
        result = 1;
        break;
    }
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


void
gonzo_alias_delete(string_ty *name)
{
    gonzo_ty        *gp;
    long            j;

    trace(("gonzo_alias_delete(name = \"%s\")\n{\n", name->str_text));
    for (j = 0;; ++j)
    {
        gp = gonzo_nth(j);
        if (!gp)
            break;
        gonzo_alias_delete_destination_sub(gp, name);
        if (gonzo_alias_delete_sub(gp, name))
            break;
    }
    trace(("}\n"));
}


static void
waiting_callback(void *)
{
    if (user_ty::create()->lock_wait())
        error_intl(0, i18n("waiting for global state lock"));
    else
        fatal_intl(0, i18n("global state lock not available"));
}


void
gonzo_gstate_lock_prepare_new(void)
{
    trace(("gonzo_gstate_lock_prepare_new()\n{\n"));
    lock_prepare_gstate(waiting_callback, 0);
    trace(("}\n"));
}


string_ty *
gonzo_lockpath_get(void)
{
    static string_ty *path;
    gonzo_ty        *gp;

    trace(("gonzo_lockpath_get()\n{\n"));
    if (!path)
    {
        do_tail();
        gp = gonzo[ngonzos - 1];
        construct_library_directory(gp);
        path = str_format("%s/lockfile", gp->dir->str_text);
    }
    trace(("return \"%s\";\n", path->str_text));
    trace(("}\n"));
    return path;
}


static int
gonzo_ustate_path_sub(gonzo_ty *gp, string_ty *project_name)
{
    gstate_ty       *gstate_data;
    size_t          j;
    int             result;

    //
    // find the project in the gstate
    //
    trace(("gonzo_ustate_path_sub(gp = %p)\n{\n", gp));
    gstate_data = gonzo_gstate_get(gp);
    assert(gstate_data->where);
    result = 0;
    for (j = 0; j < gstate_data->where->length; ++j)
    {
        gstate_where_ty *addr;

        addr = gstate_data->where->list[j];
        if (addr->alias_for)
            continue;
        if (str_equal(addr->project_name, project_name))
        {
            result = 1;
            break;
        }
    }
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


nstring
gonzo_ustate_path(const nstring &project_name, const nstring &login_name)
{
    //
    // find the project in the gstate list
    //      the user state file contains an index into the project files
    //      and is thus kept in the same directory
    //
    trace(("gonzo_ustate_path(project_name = %s, login_name = %s)\n{\n",
        project_name.quote_c().c_str(), login_name.quote_c().c_str()));
    gonzo_ty *gp = 0;
    for (size_t j = 0;; ++j)
    {
        gp = gonzo_nth(j);
        if (!gp)
        {
            fatal_raw("project %s unknown (bug)",
                project_name.quote_c().c_str());
        }
        if (gonzo_ustate_path_sub(gp, project_name.get_ref()))
            break;
    }

    //
    // make sure that the directory for the user state files exists
    //      (must be world writable in testing situations)
    //
    gonzo_become();
    assert(os_exists(gp->dir));
    nstring dir = nstring::format("%s/user", gp->dir->str_text);
    if (!os_exists(dir))
    {
        os_mkdir(dir, 0755);
        if (gp->temporary)
            os_chmod(dir, 0777);
    }
    gonzo_become_undo();

    //
    // build the user state file name
    //
    nstring result =
        nstring::format("%s/user/%s", gp->dir->str_text, login_name.c_str());
    trace(("return %s;\n", result.quote_c().c_str()));
    trace(("}\n"));
    return result;
}


void
gonzo_become(void)
{
    trace(("gonzo_become()\n{\n"));
    gonzo_user()->become_begin();
    trace(("}\n"));
}


void
gonzo_become_undo(void)
{
    trace(("gonzo_become_undo()\n{\n"));
    gonzo_user()->become_end();
    trace(("}\n"));
}


void
gonzo_report_path(string_list_ty *p)
{
    long            j;
    gonzo_ty        *gp;
    string_ty       *s;

    p->clear();
    for (j = 0;; ++j)
    {
        gp = gonzo_nth(j);
        if (!gp)
            break;
        p->push_back_unique(gp->dir);
    }

    s = str_from_c(configured_datadir());
    p->push_back_unique(s);
    str_free(s);
}


int
gonzo_alias_acceptable(string_ty *name)
{
    string_ty *s = str_quote_shell(name);
    bool ok = str_equal(s, name);
    str_free(s);
    return ok;
}


// vim: set ts=8 sw=4 et :
