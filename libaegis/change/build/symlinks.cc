//
// aegis - project change supervisor
// Copyright (C) 1999, 2001-2008, 2011, 2012 Peter Miller
// Copyright (C) 2007-2009 Walter Franzini
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
#include <common/ac/unistd.h>
#include <common/ac/sys/stat.h>

#include <common/mem.h>
#include <common/nstring.h>
#include <common/symtab/template.h>
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/dir_stack.h>
#include <libaegis/file.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


struct slink_info_ty
{
    string_list_ty  stack;
    change::pointer cp;
    pconf_ty        *pconf_data;
    user_ty::pointer up;
    const work_area_style_ty *style;
    int             umask;

    ~slink_info_ty()
    {
    }

    slink_info_ty() :
        cp(0),
        pconf_data(0),
        style(0),
        umask(022)
    {
    }
};

static string_ty *dot;
static symtab<nstring> *derived_symlinks;

enum file_status
{
    file_status_not_related,
    file_status_up_to_date,
    file_status_out_of_date
};


static file_status
check_symbolic_link_to_baseline(string_ty *path_rel, string_list_ty *stack,
    int start)
{
    //
    // We know it's a symbolic link.
    // We know it's in the development directory.
    //
    nstring path_abs(os_path_cat(stack->string[0], path_rel));
    nstring dest_abs(os_readlink(path_abs.get_ref()));
    if (dest_abs[0] != '/')
    {
        //
        // It's relative, so Aegis didn't create it.  Leave it alone.
        //
        return file_status_not_related;
    }

    nstring dest_rel(dir_stack_relative(stack, dest_abs.get_ref()));
    if (dest_rel.empty())
    {
        //
        // It doesn't point into the directory stack, so Aegis didn't
        // create it.  Leave it alone.
        //
        return file_status_not_related;
    }

    if (dest_rel != nstring(path_rel))
    {
        //
        // It points into the directory stack, bit it doesn't point at
        // the corresponding file, so Aegis didn't create it.  Leave it
        // alone.
        //
        return file_status_not_related;
    }

    //
    // The symlink points to *a* corresponding file.  See if it points
    // to the *correct* level of the directory stack.
    //
    nstring supposed_to_be(dir_stack_find(stack, start, path_rel, 0, 0, 1));
    if (supposed_to_be != dest_abs)
        return file_status_out_of_date;
    return file_status_up_to_date;
}


static bool
stat_same_file(const struct stat &st1, const struct stat &st2)
{
    return (st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino);
}


static file_status
check_hard_link_to_baseline(const nstring &, const struct stat &dst_st,
    const nstring &, const struct stat &src_st)
{
    //
    // We know it's a regular file.
    // We know it's in the development directory.
    //

    //
    // If it isn't a hard link, it isn't related.
    //
    if (!S_ISREG(src_st.st_mode) || !S_ISREG(dst_st.st_mode))
        return file_status_not_related;

    //
    // See that the correct file is, and see if the link goes to the
    // correct file.
    //
    if (stat_same_file(src_st, dst_st))
        return file_status_up_to_date;
    return file_status_out_of_date;
}


static file_status
check_copy_of_baseline(const nstring &dst_abs, const struct stat &dst_st,
    const nstring &src_abs, const struct stat &src_st)
{
    //
    // Make sure the files are identical.
    //
    if
    (
        dst_st.st_size != src_st.st_size
    ||
        files_are_different(dst_abs.get_ref(), src_abs.get_ref())
    )
        return file_status_out_of_date;

    //
    // Looks good.
    //
    return file_status_up_to_date;
}


static bool
try_to_make_hard_link_to_baseline(const nstring &dst, const nstring &src)
{
    os_become_must_be_active();
    int err = glue_link(src.c_str(), dst.c_str());
    if (err == 0)
        return true;
    int errno_old = errno;
    switch (errno_old)
    {
    case EXDEV:
        //
        // Cross device (file system) hard link.
        //
        return false;

    case ENOSYS:
#ifdef EOPNOTSUPP
    case EOPNOTSUPP:
#endif
        //
        // This is a guess.  The only scenario this will happen for is a
        // system which has hard links, using a network file system to a
        // server which does NOT have hard links.
        //
        return false;
    }
    sub_context_ty *scp = sub_context_new();
    sub_errno_setx(scp, errno_old);
    sub_var_set_string(scp, "File_Name1", src.get_ref());
    sub_var_set_string(scp, "File_Name2", dst.get_ref());
    fatal_intl(scp, i18n("link(\"$filename1\", \"$filename2\"): $errno"));
    // NOTREACHED
    return false;
}


static bool
try_to_make_symbolic_link_to_baseline(const nstring &dst, const nstring &src)
{
#ifdef S_IFLNK
    os_become_must_be_active();
    int err = glue_symlink(src.c_str(), dst.c_str());
    if (err == 0)
    {
#ifdef HAVE_LUTIME
        //
        // If possible, set the mtime of the symlink.  It appears that
        // GNU make looks at symlink mod times when figuring out what to
        // build.  Sigh.
        //
        // Ignore any error returned, it doesn't matter if it can't be
        // set correctly.
        //
        struct utimes utb;
        utb.actime =  os_mtime_actual(src.get_ref());
        utb.modtime = utb.actime;
        glue_lutime(&utb, dst.c_str());
#endif
        return true;
    }
    int errno_old = errno;
    switch (errno_old)
    {
    case EPERM:
        // The filesystem containing the destination does not support
        // the creation of symbolic links.
    case ENOSYS:
        // This operating doesn't grok symlinks.
#ifdef EOPNOTSUPP
    case EOPNOTSUPP:
        // This operating doesn't grok symlinks.
#endif
        //
        // This is a guess.  The only scenario this will happen for is a
        // system which has symlinks, using a network file system to a
        // server which does NOT have symbolic links.
        //
        return false;
    }
    sub_context_ty *scp = sub_context_new();
    sub_errno_setx(scp, errno_old);
    sub_var_set_string(scp, "File_Name1", src.get_ref());
    sub_var_set_string(scp, "File_Name2", dst.get_ref());
    fatal_intl(scp, i18n("symlink(\"$filename1\", \"$filename2\"): $errno"));
    // NOTREACHED
#endif
    return false;
}


static bool
comma_d(const nstring &s)
{
    return (s.size() > 2 && s[s.size() - 2] == ',' && s[s.size() - 1] == 'D');
}


static bool
is_a_symlink_exception(const nstring &path, slink_info_ty *sip)
{
    pconf_symlink_exceptions_list_ty *lp = sip->pconf_data->symlink_exceptions;
    assert(lp);
    if (!lp)
        return false;
    for (size_t j = 0; j < lp->length; ++j)
    {
        if (path.gmatch(lp->list[j]->str_text))
            return true;
    }
    return false;
}


static void
os_symlink_repair(string_ty *value, string_ty *filename)
{
    string_ty   *s;

    //
    // Most of the time, this results in one system call, because the
    // symlink already exists.
    //
    // Only rarely will it result in three system calls.
    //
    // The case for creating new links takes a different code path, and
    // never enters this function.  It, too, only uses one system call.
    //
    s = os_readlink(filename);
    trace(("readlink \"%s\" -> \"%s\"\n", filename->str_text, s->str_text));
    if (!str_equal(s, value))
    {
        trace(("rm \"%s\"\n", filename->str_text));
        os_unlink(filename);
        trace(("ln -s \"%s\" \"%s\"\n", value->str_text, filename->str_text));
        os_symlink(value, filename);
    }
    str_free(s);
}


static void
os_lstat(const nstring &path, struct stat &st)
{
    os_become_must_be_active();
#ifdef S_IFLNK
    int oret = glue_lstat(path.c_str(), &st);
#else
    int oret = glue_stat(path.c_str(), &st);
#endif
    if (oret)
    {
        int errno_old = errno;
        sub_context_ty sc;
        sc.errno_setx(errno_old);
        sc.var_set_string("File_Name", path);
        sc.fatal_intl(i18n("stat $filename: $errno"));
        // NOTREACHED
    }
}


static bool
is_an_aegis_symlink(string_ty *path_rel, string_list_ty *stack, int start)
{
    trace(("is_an_aegis_symlink(\"%s\", stack, %d)\n{\n",
           path_rel->str_text, start));

    nstring path_abs(os_path_cat(stack->string[start], path_rel));
    nstring dest_abs(os_readlink(path_abs.get_ref()));
    if (dest_abs[0] != '/')
    {
        //
        // It's relative, so Aegis didn't create it.  Leave it alone.
        //
        trace(("return false; /* relative path */\n}\n"));
        return false;
    }
    if (os_isa_special_file(dest_abs.get_ref()))
    {
        //
        // The destination file is not a regular file, so Aegis didn't
        // create it.
        //
        trace(("return false; /* special file */\n}\n"));
        return false;
    }
    for (size_t c = start; c < stack->nstrings; ++c)
    {
        nstring test_path(os_path_cat(stack->string[c], path_rel));
        if (str_equal(test_path, dest_abs))
        {
            //
            // We have found a match so this test pass.
            //
            break;
        }
        if (c == stack->nstrings-1)
        {
            //
            // The destination file is not into the stack, so Aegis
            // did't create it.
            //
            trace(("return false; /* not into the stack */\n}\n"));
            return false;
        }
    }

    trace(("return true;\n"));
    trace(("}\n"));
    return true;
}


//
// NAME
//      maintain
//
// SYNOPSIS
//      void maintain(void *arg, dir_stack_walk_message_t msg, string_ty *path,
//              struct stat *st, int depth);
//
// DESCRIPTION
//      The maintain function is used to maintain symbolic links in the
//      development directory.  It is called once for each file by the
//      dir_stack_walk function.
//
//      arg     The argument passed to dir_stack_walk
//      msg     The message indicating what the file is
//      path    The RELATIVE path name of the file, relative to the
//              directory stack.
//      st      The stat structure describing the file.
//      depth   The depth down the directory stack.  Zero means in the
//              development directory (or integration directory).
//

static void
maintain(void *p, dir_stack_walk_message_t msg, string_ty *path_rel,
    struct stat *st, int depth, int)
{
    trace(("maintain(path_rel = \"%s\", depth = %d)\n{\n", path_rel->str_text,
        depth & ~TOP_LEVEL_SYMLINK));
    bool top_level_symlink = !!(depth & TOP_LEVEL_SYMLINK);
    trace(("top_level_symlink = %d\n", top_level_symlink));
    depth &= ~TOP_LEVEL_SYMLINK;
    slink_info_ty *sip = (slink_info_ty *)p;
    nstring path_abs(os_path_cat(sip->stack.string[0], path_rel));
    change::pointer cp = sip->cp;

    sip->up->become_end();
    fstate_src_ty *c_src = cp->file_find(nstring(path_rel), view_path_first);
    project *pp = cp->pp;
    project *ppp = (pp->is_a_trunk() ? 0 : pp->parent_get());
    if (cp->is_being_integrated())
    {
        if (!c_src)
        {
            c_src =
                pp->change_get()->file_find(nstring(path_rel), view_path_first);
        }
        pp = ppp;
    }
    fstate_src_ty *p_src = (pp ? pp->file_find(path_rel, view_path_simple) : 0);
    sip->up->become_begin();

    switch (msg)
    {
    case dir_stack_walk_dir_before:
        //
        // We don't do anything here, because we want to create as few
        // directories as possible.  See the os_mkdir_between calls, below.
        //
        trace(("dir before\n"));
        break;

    case dir_stack_walk_dir_after:
        trace(("dir after\n"));
        break;

    case dir_stack_walk_symlink:
        trace(("symlink\n"));

        //
        // The dir_stack_walk function was told to ignore symlinks,
        // so if it returns a symlink it is a derived file, or a
        // symlink maintained by this function in a deeper branch.
        //
        if
        (
            !sip->style->derived_file_link
        &&
            !sip->style->derived_file_symlink
        &&
            !sip->style->derived_file_copy
        )
            break;
        if (top_level_symlink)
            break;
        if (!is_an_aegis_symlink(path_rel, &sip->stack, depth))
        {
            nstring src_abs(os_path_cat(sip->stack.string[depth], path_rel));
            nstring path_target(os_readlink(src_abs.get_ref()));
            if (!os_exists(path_abs.get_ref()))
            {
                os_mkdir_between(sip->stack.string[0], path_rel, 02755);
                os_symlink(path_target.get_ref(), path_abs.get_ref());
            }
            if (sip->style->during_build_only)
            {
                assert(derived_symlinks);
                derived_symlinks->assign(nstring(path_rel), path_target);
            }
        }
        break;

    case dir_stack_walk_special:
        //
        // This can't be a source file, it has to be a derived file.
        // Let them build it again if it isn't in the top level
        // directory; we ignore it.
        //
        trace(("special\n"));
        break;

    case dir_stack_walk_file:
        trace(("dir_stack_walk_file\n"));
        bool file_restored = false;
        if
        (
            (
                sip->style->derived_file_link
            ||
                sip->style->derived_file_symlink
            ||
                sip->style->derived_file_copy
            )
        &&
            p_src
        &&
            (p_src->action == file_action_remove)
        )
        {
            //
            // Ralph Smith: If the visible file is shallower than the
            // removal, then it has presumably been restored as a
            // derived file.  If it is stale, the user gets to deal with
            // it.
            //
            // Can't use project_file_path() for a removed file.
            // Is there a simpler way to do this?
            //
            int rem_depth = 1;
            project *ptmp;
            fstate_src_ty *tmp_src;
            for
            (
                ptmp = pp->is_a_trunk() ? 0 : pp->parent_get();
                ptmp;
                ptmp = (ptmp->is_a_trunk() ? 0 : ptmp->parent_get())
            )
            {
                sip->up->become_end();
                tmp_src = ptmp->file_find(path_rel, view_path_simple);
                sip->up->become_begin();

                if (tmp_src && tmp_src->action == file_action_remove)
                    rem_depth++;
                else
                    break;
            }
            trace(("rem_depth: %d\n",rem_depth));
            if (rem_depth >= depth)
                file_restored = true;
        }
        if (top_level_symlink)
        {
            trace(("top level symlink\n"));
            if (file_restored)
            {
                trace(("symlink to a restored derived file"));
                goto done;
            }
            if (c_src)
            {
                switch (c_src->action)
                {
                case file_action_remove:
                    trace(("file_action_remove\n"));
                    trace(("rm %s\n", path_abs.c_str()));
                    os_unlink(path_abs.get_ref());
                    break;

                case file_action_transparent:
                    {
                        trace(("transparent\n"));
                        if (!ppp)
                        {
                            trace(("rm %s\n", path_abs.c_str()));
                            os_unlink(path_abs.get_ref());
                            break;
                        }
                        //
                        // Note: we don't use dir_stack_find, there could be
                        // stale files too early in the stack.
                        //
                        sip->up->become_end();
                        nstring origin(project_file_path(ppp, path_rel));
                        sip->up->become_begin();
                        if (origin.empty())
                        {
                            trace(("rm %s\n", path_abs.c_str()));
                            os_unlink(path_abs.get_ref());
                            break;
                        }
                        os_mkdir_between(sip->stack.string[0], path_rel, 02755);
                        os_symlink_repair(origin.get_ref(), path_abs.get_ref());
                    }
                    break;

                case file_action_create:
                case file_action_modify:
                case file_action_insulate:
                    //
                    // This symlink should not be here, but where the
                    // heck did the real file go?  Not having a good
                    // answer, we do nothing.
                    //
                    break;
                }
                goto done;
            }
            if (p_src)
            {
                //
                // Note: we don't use dir_stack_find, there could be
                // stale files too early in the stack.
                //
                trace(("project file\n"));
                if (p_src->action == file_action_remove)
                {
                    trace(("rm %s\n", path_abs.c_str()));
                    os_unlink(path_abs.get_ref());
                }
                else
                {
                    switch(p_src->usage)
                    {
                    case file_usage_build:
                        //
                        // Do not repair links related to build files.
                        // They should follow the derived_files_* style.
                        //
                        break;

                    case file_usage_config:
                    case file_usage_source:
                    case file_usage_manual_test:
                    case file_usage_test:
#ifndef DEBUG
                    default:
#endif
                        {
                            sip->up->become_end();
                            nstring origin(project_file_path(pp, path_rel));
                            sip->up->become_begin();
                            assert(!origin.empty());
                            os_mkdir_between
                            (
                                sip->stack.string[0],
                                path_rel,
                                02755
                            );
                            os_symlink_repair
                            (
                                origin.get_ref(),
                                path_abs.get_ref()
                            );
                        }
                        break;
                    }
                }
                goto done;
            }
            trace(("check symbolic link to baseline\n"));
            switch (check_symbolic_link_to_baseline(path_rel, &sip->stack, 1))
            {
            case file_status_not_related:
            case file_status_up_to_date:
                goto done;

            case file_status_out_of_date:
                //
                // Do not unlink derived file registered into aegis.
                //
                if (p_src && p_src->usage == file_usage_build)
                    goto done;
                trace(("rm %s\n", path_abs.c_str()));
                os_unlink(path_abs.get_ref());
                depth = 666;
                break;
            }
        }
        if (depth == 0)
        {
            //
            // Note: there is no os_mkdir_between call required, because
            // the file exists at the top level (in the development
            // directory or integration directory).
            //
            trace(("file at top\n"));
            if (c_src)
            {
                trace(("change file\n"));
                switch (c_src->action)
                {
                case file_action_remove:
                    // This is probably whiteout.
                    // Leave it alone.
                    break;

                case file_action_transparent:
                    //
                    // Note: we don't use dir_stack_find, there could be
                    // stale files too early in the stack.
                    //
                    p_src = 0;
                    if (ppp)
                    {
                        sip->up->become_end();
                        p_src = ppp->file_find(path_rel, view_path_extreme);
                        sip->up->become_begin();
                    }
                    if (!p_src)
                    {
                        trace(("rm %s\n", path_abs.c_str()));
                        os_unlink(path_abs.get_ref());
                    }
                    break;

                case file_action_create:
                case file_action_modify:
                case file_action_insulate:
                    break;
                }
                goto done;
            }
            if (p_src)
            {
                trace(("project file at top\n"));
                if (p_src->action == file_action_remove)
                {
                    // This is probably whiteout.
                    // Leave it alone.
                    goto done;
                }

                //
                // Note: we don't use dir_stack_find, there could be
                // stale files too early in the stack.
                //
                sip->up->become_end();
                nstring origin(project_file_path(pp, path_rel));
                sip->up->become_begin();
                assert(!origin.empty());
                struct stat st1;
                os_lstat(origin, st1);

                //
                // If the file is an accurate reflection of the
                // baseline, we do not need to do anything else.
                //
                if
                (
                    (
                        check_hard_link_to_baseline(path_abs, *st, origin, st1)
                    ==
                        file_status_up_to_date
                    )
                ||
                    (
                        check_copy_of_baseline(path_abs, *st, origin, st1)
                    ==
                        file_status_up_to_date
                    )
                )
                    goto done;

                //
                // If the file is a registered derived file then we do
                // not do anything else.
                //
                if (p_src->usage == file_usage_build)
                    goto done;

                //
                // Not an accurate reflection of the baseline,
                // get rid of it and start again.
                //
                trace(("rm %s\n", path_abs.c_str()));
                os_unlink(path_abs.get_ref());

                if
                (
                    (
                        !sip->style->source_file_link
                    ||
                        !try_to_make_hard_link_to_baseline(path_abs, origin)
                    )
                &&
                    (
                        !sip->style->source_file_symlink
                    ||
                        !try_to_make_symbolic_link_to_baseline(path_abs, origin)
                    )
                &&
                    sip->style->source_file_copy
                )
                {
                    //
                    // This is a bit subtle.  When aecpu or aemtu call
                    // change_maintain_symlinks_to_baseline they tell
                    // it that they are undoing, which in turn causes
                    // the style to be altered to copy source files and
                    // prevents hard links or symbolic links.  The aecpu
                    // and aemtu commands DO NOT want to copy mod time
                    // (cmt) of source files in the baseline, but they
                    // do want a copy of the file contents to attach
                    // the alternate mod time to.
                    //
                    bool cmt =
                        (
                            !sip->style->source_file_copy
                        &&
                            !sip->style->derived_file_copy
                        );

                    copy_whole_file(origin.get_ref(), path_abs.get_ref(), cmt);
                    int file_mode = 0444;
                    if (p_src->executable)
                        file_mode |= 0111;
                    os_chmod(path_abs.get_ref(), file_mode & ~sip->umask);
                }
                goto done;
            }
            trace(("derived file\n"));

            //
            // It isn't possible to distinguish a copy which is out
            // of date because the file in the baseline changed from
            // a derived file in the development directory.
            //
            // It isn't possible to distinguish a hard link which is
            // out of date because the file in the baseline changed
            // (and thus broke the link) from a derived file in the
            // development directory.
            //
            // In each case we do nothing, and trust that the build will
            // bring it back up-to-date.
            //
            goto done;
        }
        trace(("file NOT at top\n"));
        assert(depth > 0);
        if (c_src)
        {
            trace(("change file not at top?\n"));
            switch (c_src->action)
            {
            case file_action_create:
            case file_action_modify:
            case file_action_insulate:
                //
                // This source file is supposed to be here, but where
                // the heck did the real file go?  Not having a good
                // answer, we do nothing.
                //
                break;

            case file_action_remove:
                break;

            case file_action_transparent:
                //
                // This file should be a copy of the grandparent (or
                // deeper) file, or a hard link to the file, but it
                // isn't there.
                //
                if (!ppp)
                    break;

                //
                // Note: we don't use dir_stack_find, there could be
                // stale files too early in the stack.
                //
                sip->up->become_end();
                nstring origin(project_file_path(ppp, path_rel));
                fstate_src_ty *ppp_src =
                    ppp->file_find(path_rel, view_path_extreme);
                assert(ppp_src);
                sip->up->become_begin();
                if (origin.empty())
                    break;

                //
                // Do not process derived files registered within Aegis.
                //
                if (c_src->usage == file_usage_build)
                    break;

                os_mkdir_between(sip->stack.string[0], path_rel, 02755);
                if
                (
                    !try_to_make_hard_link_to_baseline(path_abs, origin)
                &&
                    !try_to_make_symbolic_link_to_baseline(path_abs, origin)
                )
                {
                    copy_whole_file(origin.get_ref(), path_abs.get_ref(), 0);
                    int file_mode = 0444;
                    if (ppp_src->executable)
                        file_mode |= 0111;
                    os_chmod(path_abs.get_ref(), file_mode & ~sip->umask);
                }
                break;
            }
            goto done;
        }

        if (p_src && !file_restored)
        {
            //
            // There is no file in the development directory, there is
            // no change file, but there is a project source file.
            //
            trace(("project file not at top\n"));
            if (p_src->action == file_action_remove)
                break;
            if (p_src->usage == file_usage_build)
                break;
            os_mkdir_between(sip->stack.string[0], path_rel, 02755);

            //
            // Note: we don't use dir_stack_find, there could be
            // stale files too early in the stack.
            //
            sip->up->become_end();
            nstring origin(project_file_path(pp, path_rel));
            sip->up->become_begin();
            assert(!origin.empty());
            if
            (
                (
                    !sip->style->source_file_link
                ||
                    !try_to_make_hard_link_to_baseline(path_abs, origin)
                )
            &&
                (
                    !sip->style->source_file_symlink
                ||
                    !try_to_make_symbolic_link_to_baseline(path_abs, origin)
                )
            &&
                sip->style->source_file_copy
            )
            {
                copy_whole_file(origin.get_ref(), path_abs.get_ref(), 1);
                int file_mode = 0444;
                if (p_src->executable)
                    file_mode |= 0111;
                os_chmod(path_abs.get_ref(), file_mode & ~sip->umask);
            }
        }
        else if
        (
            sip->cp->is_being_developed()
        &&
            comma_d(nstring(path_rel))
        )
        {
            //
            // Do not make links or symlinks or copies of the difference
            // files produced by aed.  They just make the work area
            // busier for no good reason (and stop aedless from working).
            //
        }
        else if (!is_a_symlink_exception(nstring(path_rel), sip))
        {
            //
            // There is no file in the development directory, there is
            // no change file and there is no project file.  Therefore,
            // this is a derived file in the baseline, created by an
            // integration build.
            //
            // Make a link or copy, if we have been asked to, and if
            // this file isn't one of the exceptions.  The name "symlink
            // exceptions" reveals the history of this functionality,
            // since generalised to cover hard links and cipies as well.
            //
            trace(("derived file not at top\n"));
            os_mkdir_between(sip->stack.string[0], path_rel, 02755);
            nstring origin(os_path_cat(sip->stack.string[depth], path_rel));
            if
            (
                (
                    !sip->style->derived_file_link
                ||
                    !try_to_make_hard_link_to_baseline(path_abs, origin)
                )
            &&
                (
                    !sip->style->derived_file_symlink
                ||
                    !try_to_make_symbolic_link_to_baseline(path_abs, origin)
                )
            &&
                sip->style->derived_file_copy
            )
            {
                copy_whole_file(origin.get_ref(), path_abs.get_ref(), 1);
                int file_mode = 0644;
                if (os_executable(origin.get_ref()))
                    file_mode |= 0111;
                os_chmod(path_abs.get_ref(), file_mode & ~sip->umask);
            }
        }
        break;
    }
    done:
    trace(("}\n"));
}


//
// NAME
//      change_create_symlinks_to_baseline
//
// SYNOPSIS
//      void change_create_symlinks_to_baseline(change::pointer cp,
//          project *pp, user_ty::pointer up, work_area_style_ty *style);
//
// DESCRIPTION
//      The change_create_symlinks_to_baseline function is used to create
//      symbolic links between a development directory (or integration
//      directory) and the baseline (and ancestor baselines).  It does
//      this in two passes, the first creates the links, and the second
//      removes dead links.
//

void
change_create_symlinks_to_baseline(change::pointer cp, user_ty::pointer up,
    const work_area_style_ty &style)
{
    slink_info_ty   si;

    trace(("change_create_symlinks_to_baseline(cp = %p)\n{\n", cp));
    trace(("source_file_link = %d\n", style.source_file_link));
    trace(("source_file_symlink = %d\n", style.source_file_symlink));
    trace(("source_file_copy = %d\n", style.source_file_copy));
    trace(("derived_file_link = %d\n", style.derived_file_link));
    trace(("derived_file_symlink = %d\n", style.derived_file_symlink));
    trace(("derived_file_copy = %d\n", style.derived_file_copy));
    if
    (
        !style.source_file_link
    &&
        !style.source_file_symlink
    &&
        !style.source_file_copy
    &&
        !style.derived_file_link
    &&
        !style.derived_file_symlink
    &&
        !style.derived_file_copy
    )
    {
        trace(("}\n"));
        return;
    }
    if (style.during_build_only)
        derived_symlinks = new symtab<nstring>;
    trace(("during_build_only = %d\n", style.during_build_only));
    trace(("derived_at_start_only = %d\n", style.derived_at_start_only));
    assert(cp->reference_count >= 1);
    change_verbose(cp, 0, i18n("creating symbolic links to baseline"));

    //
    // Work out the search path.
    //
    cp->search_path_get(&si.stack, false);
#ifdef DEBUG
    {
        size_t          k;
        for (k = 0; k < si.stack.nstrings; ++k)
            trace(("si.stack.string[%ld] = \"%s\"\n", (long)k,
                    si.stack.string[k]->str_text));
    }
#endif

    //
    // For each ancestor, create symlinks from the development
    // directory to that ancestor's baseline if the file does not
    // already exist.
    //
    si.cp = cp;
    si.pconf_data = change_pconf_get(cp, 0);
    si.style = &style;
    si.up = up;
    si.umask = cp->umask_get();
    if (!dot)
        dot = str_from_c(".");
    up->become_begin();
    dir_stack_walk(&si.stack, dot, maintain, &si, 1);
    up->become_end();
    trace(("}\n"));
}


static void
unmaintain(void *p, dir_stack_walk_message_t msg, string_ty *path,
    struct stat *, int depth, int)
{
    trace(("unmaintain(path = \"%s\", msg = %d, depth = %d)\n{\n",
        path->str_text, msg, depth & ~TOP_LEVEL_SYMLINK));
    bool top_level_symlink = !!(depth & TOP_LEVEL_SYMLINK);
    depth &= ~TOP_LEVEL_SYMLINK;
    if (depth && !top_level_symlink)
    {
        trace(("}\n"));
        return;
    }
    slink_info_ty *sip = (slink_info_ty *)p;
    trace(("mark\n"));
    assert(sip->style->during_build_only);
    switch (msg)
    {
    case dir_stack_walk_dir_before:
    case dir_stack_walk_dir_after:
        trace(("is a directory\n"));
        break;

    case dir_stack_walk_special:
        break;

    case dir_stack_walk_symlink:
        trace(("is a symlink\n"));
        if
        (
            top_level_symlink
        ||
            !is_an_aegis_symlink(path, &sip->stack, depth)
        )
        {
            //
            // Remember: the top-level-symlink flags means that the
            // development directory contains a symlink over the top of
            // some other type of file.
            //
            trace(("mark\n"));
            nstring path_abs(os_path_cat(sip->stack.string[0], path));
            nstring dest(os_readlink(path_abs.get_ref()));
            nstring dest_rel(dir_stack_relative(&sip->stack, dest.get_ref()));
            //
            // We keep track of derived symlinks Aegis create in the
            // working directory and, after the build, remove those
            // not modified.
            //
            if (is_an_aegis_symlink(path, &sip->stack, depth))
            {
                if (dest_rel.empty() || nstring(path) != dest_rel)
                    break;
            }
            else
            {
                assert(derived_symlinks);
                nstring *old_path = derived_symlinks->query(path);
                if (old_path == NULL || old_path->empty())
                    break;
                if (*old_path != dest)
                    break;
            }

            trace(("rm %s\n", path_abs.c_str()));
            os_unlink(path_abs.get_ref());
        }
        break;

    case dir_stack_walk_file:
        trace(("is a file\n"));
        {
            // scope for c_src:
            sip->up->become_end();
            fstate_src_ty *c_src =
                sip->cp->file_find(nstring(path), view_path_first);
            sip->up->become_begin();
            if (c_src)
                break;
        }
        trace(("mark\n"));
        if (top_level_symlink)
        {
            //
            // Remove symlinks in the development directory which point
            // to a counterpart in the baseline (any counterpart, not
            // necessarily the right one).
            //
            trace(("mark\n"));
            nstring path_abs(os_path_cat(sip->stack.string[0], path));
            nstring dest(os_readlink(path_abs.get_ref()));
            nstring dest_rel(dir_stack_relative(&sip->stack, dest.get_ref()));
            if (!dest_rel.empty() && nstring(path) == dest_rel)
            {
                trace(("rm %s\n", path_abs.c_str()));
                os_unlink(path_abs.get_ref());
            }
        }
        else
        {
            //
            // Remove hard links in the development directory which
            // have a counterpart in the baseline (any counterpart, not
            // necessarily the right one, and the link could have been
            // broken).
            //
            // Watch out for integration builds, which need to check for
            // grandparent files BUT not nuke project files from this branch.
            //
            trace(("mark\n"));
            project *pp = sip->cp->pp;
            sip->up->become_end();
            if (sip->cp->is_being_integrated())
            {
                change::pointer branch_cp = sip->cp->pp->change_get();
                if (branch_cp->file_find(nstring(path), view_path_first))
                {
                    sip->up->become_begin();
                    break;
                }
                if (pp->is_a_trunk())
                {
                    sip->up->become_begin();
                    break;
                }
                pp = pp->parent_get();
            }
            fstate_src_ty *p_src =
                pp->file_find(path, view_path_simple);
            sip->up->become_begin();
            if (p_src)
            {
                nstring path_abs(os_path_join(sip->stack.string[0], path));
                trace(("rm %s\n", path_abs.c_str()));
                os_unlink(path_abs.get_ref());
            }
        }
        break;
    }
    trace(("}\n"));
}


void
change_remove_symlinks_to_baseline(change::pointer cp, user_ty::pointer up,
    const work_area_style_ty &style)
{
    slink_info_ty   si;

    if (cp->is_being_integrated() && cp->pp->is_a_trunk())
        return;
    if
    (
        !style.source_file_link
    &&
        !style.source_file_symlink
    &&
        !style.source_file_copy
    &&
        !style.derived_file_link
    &&
        !style.derived_file_symlink
    &&
        !style.derived_file_copy
    )
        return;
    if (!style.during_build_only)
        return;
    trace(("change_remove_symlinks_to_baseline(cp = %p)\n{\n",
        cp));
    assert(cp->reference_count >= 1);
    assert(derived_symlinks);
    change_verbose(cp, 0, i18n("removing symbolic links to baseline"));

    //
    // Work out the search path.
    //
    cp->search_path_get(&si.stack, false);

    //
    // walk the tree
    //
    si.cp = cp;
    si.up = up;
    si.style = &style;
    si.umask = cp->umask_get();
    up->become_begin();
    if (!dot)
        dot = str_from_c(".");
    dir_stack_walk(&si.stack, dot, unmaintain, &si, 0);
    up->become_end();
    delete(derived_symlinks);
    derived_symlinks = NULL;
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
