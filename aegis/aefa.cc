//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2011, 2012 Peter Miller
// Copyright (C) 2008 Walter Franzini
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
#include <common/ac/libintl.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/language.h>
#include <common/nstring.h>
#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/str_list.h>
#include <common/symtab/template.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <libaegis/ael/build_header.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/formeditnum.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/attribute.h>
#include <libaegis/attrlistveri.h>
#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/fattr.fmtgen.h>
#include <libaegis/help.h>
#include <libaegis/io.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aefa.h>


#define WITH_EDIT_COL 0


static void
file_attributes_usage(void)
{
    const char *progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -File_ATtributes -File <attr-file> [ <option>... ] "
            "<filename>\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -File_ATtributes -Edit [ <option>... ] <filename>\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -File_ATtributes name=value <filename>\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -File_ATtributes -List [ <option>... ] <filename>\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -File_ATtributes -UUID <number> -File <filename>\n",
        progname
    );
    fprintf(stderr, "       %s -File_ATtributes -Help\n", progname);
    quit(1);
}


static void
file_attributes_help(void)
{
    help("aefa", file_attributes_usage);
}


static void
change_fatal_unknown_file(change::pointer cp, const nstring &filename)
{
    sub_context_ty sc(__FILE__, __LINE__);
    sc.var_set_string("File_Name", filename);
    fstate_src_ty *src = cp->file_find_fuzzy(filename, view_path_first);
    if (src)
    {
        sc.var_set_string("Guess", src->file_name);
        change_fatal(cp, &sc, i18n("no $filename, closest is $guess"));
        // NOTREACHED
    }
    else
    {
        change_fatal(cp, &sc, i18n("no $filename"));
        // NOTREACHED
    }
}


static void
fattr_assign(fattr_ty *fattr_data, const nstring &name, const nstring &value)
{
    attributes_list_ty *alp = fattr_data->attribute;
    if (!alp)
    {
        alp = (attributes_list_ty *)attributes_list_type.alloc();
        fattr_data->attribute = alp;
    }
    attributes_list_insert(alp, name.c_str(), value.c_str());
}


static bool
fattr_exists(fattr_ty *fattr_data, const nstring &name)
{
    return !!attributes_list_find(fattr_data->attribute, name.c_str());
}


static fattr_ty *
fattr_construct(fstate_src_ty *src)
{
    fattr_ty *fattr_data = (fattr_ty *)fattr_type.alloc();
    if (src->attribute)
        fattr_data->attribute = attributes_list_copy(src->attribute);
    else
        fattr_data->attribute =
            (attributes_list_ty *)attributes_list_type.alloc();

    fattr_assign(fattr_data, "usage", file_usage_ename(src->usage));
    return fattr_data;
}


static void
file_attributes_list(void)
{
    trace(("file_attributes_list()\n{\n"));
    change_identifier cid;
    nstring filename;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
            default:
                generic_argument(file_attributes_usage);
            continue;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument
            (
                file_attributes_usage
            );
            break;

        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_development_directory:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_trunk:
            cid.command_line_parse(file_attributes_usage);
            continue;

        case arglex_token_string:
            if (!filename.empty())
                error_intl(0, i18n("too many files"));
            filename = nstring(arglex_value.alv_string);
            break;
        }
        arglex();
    }
    cid.command_line_check(file_attributes_usage);
    if (filename.empty())
    {
        error_intl(0, i18n("no file names"));
        file_attributes_usage();
    }

    //
    // Resolve the file name.
    //
    filename = cid.get_cp()->file_resolve_name(cid.get_up(), filename);

    //
    // build the fattr data
    //
    fstate_src_ty *src = cid.get_cp()->file_find(filename, view_path_first);
    if (!src)
        change_fatal_unknown_file(cid.get_cp(), filename);
    fattr_ty *fattr_data = fattr_construct(src);

    //
    // For changes which are still being developed, we also add the
    // "Content-Type" attribute.
    //
    if
    (
        cid.get_cp()->is_being_developed()
    &&
        !fattr_exists(fattr_data, "Content-Type")
    )
    {
        nstring path(cid.get_cp()->file_path(filename));
        os_become_orig();
        nstring content_type = os_magic_file(path);
        os_become_undo();
        assert(!content_type.empty());
        fattr_assign(fattr_data, "content-type", content_type);
    }

    //
    // print the fattr data
    //
    language_human();
    fattr_write_file((string_ty *)0, fattr_data, 0);
    language_C();
    fattr_type.free(fattr_data);
    trace(("}\n"));
}


static void
file_attributes_report(void)
{
    trace(("file_attributes_report()\n{\n"));
    change_identifier cid;
    nstring_list filenames;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(file_attributes_usage);
            continue;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument
            (
                file_attributes_usage
            );
            break;

        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_development_directory:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_trunk:
            cid.command_line_parse(file_attributes_usage);
            continue;

        case arglex_token_string:
            filenames.push_back_unique(arglex_value.alv_string);
            break;
        }
        arglex();
    }
    cid.command_line_check(file_attributes_usage);
    if (filenames.empty())
        filenames.push_back(".");

    //
    // build the list of places to look
    // when resolving the file name
    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    os_become_orig();
    nstring curdir(os_curdir());
    os_become_undo();
    nstring_list search_path;
    cid.get_pp()->search_path_get(search_path, true);
    assert(search_path.size() >= 1);

    //
    // Find the base for relative filenames.
    //
    bool base_relative =
        (
            cid.get_up()->relative_filename_preference
            (
                uconf_relative_filename_preference_base
            )
        ==
            uconf_relative_filename_preference_base
        );
    nstring base = base_relative ? search_path.back() : curdir;

    //
    // resolve the path of each file
    // 1. the absolute path of the file name is obtained
    // 2. if the file is inside the search list
    // 3. if neither, error
    //
    nstring_list wl2;
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring filename = filenames[j];
        nstring s2 =
            (filename[0] == '/' ? filename : os_path_cat(base, filename));
        cid.get_up()->become_begin();
        nstring s1 = os_pathname(s2, true);
        cid.get_up()->become_end();
        s2.clear();
        nstring base_relative_filename;
        for (size_t k = 0; k < search_path.size(); ++k)
        {
            base_relative_filename = os_below_dir(search_path[k], s1);
            if (!base_relative_filename.empty())
                break;
        }
        if (base_relative_filename.empty())
        {
            sub_context_ty sc;
            sc.var_set_string("File_Name", filename);
            change_fatal(cid.get_cp(), &sc, i18n("$filename unrelated"));
        }
        nstring_list wl_in;
        nstring_list wl_out;
        cid.get_pp()->file_directory_query
        (
            base_relative_filename,
            wl_in,
            wl_out,
            view_path_simple
        );
        if (!wl_in.empty())
        {
            wl2.push_back_unique(wl_in);
        }
        else
        {
            wl2.push_back_unique(base_relative_filename);
        }
    }
    filenames = wl2;

    //
    // ensure that each file named on the command line
    // exists in the change set
    //
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring filename = filenames[j];
        fstate_src_ty *src_data =
            cid.get_cp()->file_find(filename, view_path_simple);
        if (!src_data || src_data->deleted_by)
            change_fatal_unknown_file(cid.get_cp(), filename);
    }

    //
    // create the columns
    //
    col::pointer colp = col::open((string_ty *)0);
    nstring line1 =
        (
            cid.get_cp()->number == TRUNK_CHANGE_NUMBER
        ?
            nstring::format
            (
                "Project \"%s\", Trunk",
                cid.get_pp()->name_get()->str_text
            )
        :
            nstring::format
            (
                "Project \"%s\", Change %ld",
                cid.get_pp()->name_get()->str_text,
                cid.get_change_number()
            )
        );
    colp->title(line1, "List File Attributes");

    int left = 0;
    output::pointer usage_col =
        colp->create(left, left + USAGE_WIDTH, "Type\n-------");
    left += USAGE_WIDTH + 1;

    output::pointer action_col =
        colp->create(left, left + ACTION_WIDTH, "Action\n--------");
    left += ACTION_WIDTH + 1;

#if WITH_EDIT_COL
    output::pointer edit_col =
        colp->create(left, left + EDIT_WIDTH, "Edit\n-------");
    left += EDIT_WIDTH + 1;
#endif

    //
    // Note that the number of columns can vary depending on which files
    // have which attributes.  This is not good for scripting.  It is
    // suggested that you use aexml(1) for scripting.
    //
    symtab<output::pointer> attribute_columns;
    attribute_columns.set_reaper();
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring filename = filenames[j];
        fstate_src_ty *src_data =
            cid.get_cp()->file_find(filename, view_path_simple);
        assert(src_data);
        assert(src_data->file_name);
        if (src_data->attribute)
        {
            for (size_t k = 0; k < src_data->attribute->length; ++k)
            {
                attributes_ty *ap = src_data->attribute->list[k];
                nstring lc_name = nstring(ap->name).downcase();
                if (!attribute_columns.query(lc_name))
                {
                    nstring title(ael_build_header(ap->name));
                    output::pointer op =
                        colp->create(left, left + ATTR_WIDTH, title.c_str());
                    attribute_columns.assign(lc_name, op);
                    left += ATTR_WIDTH + 1;
                }
            }
        }
    }
    output::pointer file_name_col =
        colp->create(left, 0, "File Name\n-----------");

    //
    // list the change's files
    //
    for (size_t j = 0; j < filenames.size() ; ++j)
    {
        nstring filename = filenames[j];
        fstate_src_ty *src_data =
            cid.get_cp()->file_find(filename, view_path_simple);
        assert(src_data);
        assert(src_data->file_name);
        fstate_src_ty *psrc_data = 0;

        usage_col->fputs(file_usage_ename(src_data->usage));
        action_col->fputs(file_action_ename(src_data->action));
#if WITH_EDIT_COL
        list_format_edit_number(edit_col, src_data);
#endif

        //
        // We use view_path_none because we want the transparent
        // files which exist simply to host the locked_by field.
        // But if the file has been removed, toss it.
        //
        psrc_data =
            cid.get_pp()->file_find(src_data, view_path_none);
        if (psrc_data && psrc_data->action == file_action_remove)
            psrc_data = 0;

#if WITH_EDIT_COL
        if
        (
            cid.get_cp()->is_being_developed()
        &&
            // watch out for ael cf -bl
            (cid.get_cp() != cid.get_pp()->change_get())
        &&
            !change_file_up_to_date(cid.get_pp(), src_data)
        )
        {
            //
            // The current head revision of the branch may not equal
            // the version "originally" copied.
            //
            if (psrc_data && psrc_data->edit)
            {
                assert(psrc_data->edit->revision);
                edit_col->fprintf
                (
                    " (%s)",
                    psrc_data->edit->revision->str_text
                );
            }
        }
        if (src_data->edit_origin_new)
        {
            //
            // The "cross branch merge" version.
            //
            assert(src_data->edit_origin_new->revision);
            edit_col->end_of_line();
            edit_col->fprintf
            (
                "{cross %4s}",
                src_data->edit_origin_new->revision->str_text
            );
        }
#endif

        file_name_col->fputs(src_data->file_name);
        if
        (
            cid.get_cp()->is_being_developed()
        &&
            psrc_data
        &&
            psrc_data->locked_by
        &&
            psrc_data->locked_by != cid.get_cp()->number
        )
        {
            file_name_col->end_of_line();
            file_name_col->fprintf
            (
                "Locked by change %ld.",
                magic_zero_decode(psrc_data->locked_by)
            );
        }
        if (src_data->about_to_be_created_by)
        {
            file_name_col->end_of_line();
            file_name_col->fprintf
            (
                "About to be created by change %ld.",
                magic_zero_decode(src_data->about_to_be_created_by)
            );
        }
        if (src_data->deleted_by)
        {
            file_name_col->end_of_line();
            file_name_col->fprintf
            (
                "Deleted by change %ld.",
                magic_zero_decode(src_data->deleted_by)
            );
        }
        if (src_data->locked_by)
        {
            file_name_col->end_of_line();
            file_name_col->fprintf
            (
                "Locked by change %ld.",
                magic_zero_decode(src_data->locked_by)
            );
        }
        if (src_data->about_to_be_copied_by)
        {
            file_name_col->end_of_line();
            file_name_col->fprintf
            (
                "About to be copied by change %ld.",
                magic_zero_decode(src_data->about_to_be_copied_by)
            );
        }
#if 0
        if (src_data->move)
        {
            switch (src_data->action)
            {
            case file_action_create:
                file_name_col->end_of_line();
                file_name_col->fputs("Moved from ");
                file_name_col->fputs(src_data->move->str_text);
                break;

            case file_action_remove:
                file_name_col->end_of_line();
                file_name_col->fputs("Moved to ");
                file_name_col->fputs(src_data->move->str_text);
                break;

            case file_action_modify:
            case file_action_insulate:
            case file_action_transparent:
                assert(0);
                break;
            }
        }
#endif
        if (src_data->attribute)
        {
            for (size_t k = 0; k < src_data->attribute->length; ++k)
            {
                attributes_ty *ap = src_data->attribute->list[k];
                if (ap->name && ap->value)
                {
                    nstring lc_name = nstring(ap->name).downcase();
                    output::pointer op = attribute_columns.get(lc_name);
                    assert(op);
                    if (op)
                        op->fputs(ap->value);
                }
            }
        }
        colp->eoln();
    }
    trace(("}\n"));
}


static void
check_permissions(change::pointer cp, user_ty::pointer up)
{
    if
    (
        !cp->is_being_developed()
    ||
        nstring(cp->developer_name()) != up->name()
    )
    {
        change_fatal(cp, 0, i18n("bad fa, not auth"));
    }
}


static void
file_attributes_edit(fattr_ty **dp, edit_ty et)
{
    sub_context_ty  *scp;
    fattr_ty        *d;
    string_ty       *filename;
    string_ty       *msg;

    //
    // write attributes to temporary file
    //
    d = *dp;
    assert(d);
    filename = os_edit_filename(1);
    os_become_orig();
    language_human();
    fattr_write_file(filename, d, 0);
    language_C();
    fattr_type.free(d);

    //
    // an error message to issue if anything goes wrong
    //
    scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", filename);
    msg = subst_intl(scp, i18n("attributes in $filename"));
    sub_context_delete(scp);
    undo_message(msg);
    str_free(msg);
    os_become_undo();

    //
    // edit the file
    //
    os_edit(filename, et);

    //
    // read it in again
    //
    os_become_orig();
    d = fattr_read_file(filename);
    commit_unlink_errok(filename);
    str_free(filename);
    os_become_undo();
    attributes_list_verify(d->attribute);
    *dp = d;
}


static attributes_ty *
fattr_extract(fattr_ty *fattr_data, const nstring &name)
{
    return attributes_list_extract(fattr_data->attribute, name.c_str());
}


static void
file_attributes_main(void)
{
    trace(("file_attributes_main()\n{\n"));
    change_identifier cid;
    arglex();
    edit_ty edit = edit_not_set;
    fattr_ty *fattr_data = 0;
    string_ty *inp = 0;
    nstring filename;
    nstring_list name_value_pairs;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(file_attributes_usage);
            continue;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument
            (
                file_attributes_usage
            );
            break;

        case arglex_token_string:
            if (strchr(arglex_value.alv_string, '='))
            {
                name_value_pairs.push_back(arglex_value.alv_string);
                break;
            }
            if (!filename.empty())
                fatal_too_many_files();
            filename = nstring(arglex_value.alv_string);
            break;

        case arglex_token_file:
            if (inp)
                duplicate_option(file_attributes_usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_file, file_attributes_usage);
                // NOTREACHED

            case arglex_token_string:
                inp = str_from_c(arglex_value.alv_string);
                break;

            case arglex_token_stdio:
                inp = str_from_c("");
                break;
            }
            break;

        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_development_directory:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_trunk:
            cid.command_line_parse(file_attributes_usage);
            continue;

        case arglex_token_edit:
            if (edit == edit_foreground)
                duplicate_option(file_attributes_usage);
            if (edit != edit_not_set)
            {
                too_many_edits:
                mutually_exclusive_options
                (
                    arglex_token_edit,
                    arglex_token_edit_bg,
                    file_attributes_usage
                );
            }
            edit = edit_foreground;
            break;

        case arglex_token_edit_bg:
            if (edit == edit_background)
                duplicate_option(file_attributes_usage);
            if (edit != edit_not_set)
                goto too_many_edits;
            edit = edit_background;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(file_attributes_usage);
            break;
        }
        arglex();
    }
    if (filename.empty())
    {
        error_intl(0, i18n("no file names"));
        file_attributes_usage();
    }
    if (inp)
    {
        os_become_orig();
        fattr_data = fattr_read_file(inp);
        os_become_undo();
        assert(fattr_data);
    }
    if (name_value_pairs.empty())
    {
        if (!fattr_data && edit == edit_not_set)
        {
            sub_context_ty sc(__FILE__, __LINE__);
            sc.var_set_charstar("Name1", arglex_token_name(arglex_token_file));
            sc.var_set_charstar("Name2", arglex_token_name(arglex_token_edit));
            error_intl(&sc, i18n("warning: no $name1, assuming $name2"));
            edit = edit_foreground;
        }
    }
    else
    {
        if (fattr_data || edit != edit_not_set)
            file_attributes_usage();
    }
    if (edit != edit_not_set && !fattr_data)
    {
        fattr_data = (fattr_ty *)fattr_type.alloc();
        fattr_data->attribute =
            (attributes_list_ty *)attributes_list_type.alloc();
    }

    //
    // Resolve the file name.
    //
    filename = cid.get_cp()->file_resolve_name(cid.get_up(), filename);

    //
    // edit the attributes
    //
    if (edit != edit_not_set)
    {
        //
        // make sure they are allowed to,
        // to avoid a wasted edit
        //
        check_permissions(cid.get_cp(), cid.get_up());

        //
        // fill in any other fields
        //
        fstate_src_ty *src =
            cid.get_cp()->file_find(nstring(filename), view_path_first);
        if (!src)
            change_fatal_unknown_file(cid.get_cp(), filename);
        fattr_data = fattr_construct(src);

        //
        // For changes which are still being developed, we also add the
        // "content-type" attribute.
        //
        if
        (
            cid.get_cp()->is_being_developed()
        &&
            !fattr_exists(fattr_data, "content-type")
        )
        {
            nstring path(cid.get_cp()->file_path(filename));
            os_become_orig();
            nstring content_type = os_magic_file(path);
            os_become_undo();
            fattr_assign(fattr_data, "content-type", content_type);
        }

        //
        // edit the attributes
        //
        {
            sub_context_ty sc(__FILE__, __LINE__);
            sc.var_set_string("Name", project_name_get(cid.get_pp()));
            sc.var_set_long("Number", magic_zero_decode(cid.get_cp()->number));
            sc.var_set_string("File_Name", filename);
            io_comment_append
            (
                &sc,
                i18n("Project $name, Change $number, File $filename")
            );
        }

        file_attributes_edit(&fattr_data, edit);
    }

    //
    // lock the change
    //
    change_cstate_lock_prepare(cid.get_cp());
    lock_take();

    //
    // make sure they are allowed to
    // (even if edited, could have changed during edit)
    //
    check_permissions(cid.get_cp(), cid.get_up());

    //
    // copy the attributes across
    //
    fstate_src_ty *src =
        cid.get_cp()->file_find(filename, view_path_first);
    if (!src)
        change_fatal_unknown_file(cid.get_cp(), filename);
    if (name_value_pairs.empty())
    {
        if (src->attribute)
        {
            attributes_list_type.free(src->attribute);
            src->attribute = 0;
        }
        if (fattr_data->attribute)
        {
            //
            // We need to extract the "usage" pseudo-attribute,
            // and assign it to the file's usage if it is legal.
            //
            attributes_ty *ap = fattr_extract(fattr_data, "usage");
            if (ap)
            {
                if (ap->value)
                {
                    file_usage_type.enum_parse(ap->value, &src->usage);
                }
                attributes_type.free(ap);
            }

            //
            // Copy the rest in the normal way.
            //
            if (fattr_data->attribute->length)
                src->attribute = attributes_list_copy(fattr_data->attribute);
        }
        fattr_type.free(fattr_data);
    }
    else
    {
        if (!src->attribute)
            src->attribute = (attributes_list_ty *)attributes_list_type.alloc();
        for (size_t j = 0; j < name_value_pairs.size(); ++j)
        {
            nstring pair = name_value_pairs[j];
            const char *eqp = strchr(pair.c_str(), '=');
            assert(eqp);
            if (eqp)
            {
                nstring name(pair.c_str(), eqp - pair.c_str());
                nstring value(eqp + 1);

                //
                // Note that this will replace the first attribute with
                // that name.  If there is more than one of that name,
                // the second and subsequent attributes are unchanged.
                // If there is no attribute of that name, it will be
                // appended.
                //
                attributes_list_insert
                (
                    src->attribute,
                    name.c_str(),
                    value.c_str()
                );
            }
        }
    }

    //
    // Write it all out again.
    //
    cid.get_cp()->cstate_write();
    commit();
    lock_release();
    sub_context_ty sc;
    sc.var_set_string("File_Name", filename);
    change_verbose(cid.get_cp(), &sc, i18n("$filename attributes changed"));
    trace(("}\n"));
}


static void
file_attributes_uuid(void)
{
    trace(("file_attributes_uuid()\n{\n"));
    arglex();
    change_identifier cid;
    nstring uuid;
    nstring filename;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(file_attributes_usage);
            continue;

        case arglex_token_string:
            if (uuid.empty())
            {
                uuid = nstring(arglex_value.alv_string);
                if (!universal_unique_identifier_valid(uuid))
                    option_needs_uuid(arglex_token_uuid, file_attributes_usage);
                uuid = uuid.downcase();
            }
            else if (filename.empty())
            {
                filename = nstring(arglex_value.alv_string);
            }
            else
            {
                duplicate_option_by_name
                (
                    arglex_token_uuid,
                    file_attributes_usage
                );
            }
            break;

        case arglex_token_file:
            if (arglex() != arglex_token_string)
                option_needs_file(arglex_token_file, file_attributes_usage);
            if (filename.empty())
            {
                filename = nstring(arglex_value.alv_string);
            }
            else
            {
                duplicate_option_by_name
                (
                    arglex_token_file,
                    file_attributes_usage
                );
            }
            break;

        case arglex_token_change:
        case arglex_token_number:
        case arglex_token_project:
            cid.command_line_parse(file_attributes_usage);
            continue;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(file_attributes_usage);
            break;

        case arglex_token_base_relative:
        case arglex_token_current_relative:
            user_ty::relative_filename_preference_argument
            (
                file_attributes_usage
            );
            break;
        }
        arglex();
    }
    if (filename.empty())
    {
        error_intl(0, i18n("no file names"));
        file_attributes_usage();
    }
    cid.command_line_check(file_attributes_usage);

    //
    // As a special case, if no UUID string was given,
    // make one up on the spot.
    //
    if (uuid.empty())
        uuid = nstring(universal_unique_identifier());

    //
    // lock the change
    //
    change_cstate_lock_prepare(cid.get_cp());
    lock_take();

    //
    // Resolve the file name.
    //
    filename = cid.get_cp()->file_resolve_name(cid.get_up(), filename);

    //
    // Unlike other change attributes, the UUID may *only* be edited by
    // the developer when the change is in the "being developed" state.
    // This is because it should only ever be done by aepatch or aedist,
    // immediately after the files have been unpacked.
    //
    if
    (
        !cid.get_cp()->is_being_developed()
    ||
        nstring(cid.get_cp()->developer_name()) != cid.get_up()->name()
    )
    {
        change_fatal(cid.get_cp(), 0, i18n("bad fa, not auth"));
    }

    //
    // If the file already has a UUID, this command obviously isn't
    // being used by aepatch or aedist, so tell the human to take a hike.
    //
    fstate_src_ty *src = cid.get_cp()->file_find(filename, view_path_first);
    if (!src)
        change_fatal_unknown_file(cid.get_cp(), filename);
    if (src->uuid)
    {
        sub_context_ty *scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", filename);
        change_fatal(cid.get_cp(), scp, i18n("$filename uuid already set"));
        // NOTREACHED
    }

    //
    // You can't set the UUID of a file that is not being created in
    // this change set.  Doing so would be too confusing for humans
    // looking at edit numbers, or looking at file histories, because it
    // would cause discontinuities in the lineages.
    //
    if (src->action != file_action_create)
    {
        change_fatal(cid.get_cp(), 0, i18n("bad fa, not auth"));
    }

    //
    // Make sure this file UUID has not been used before, anywhere in
    // the project.  For a genuine UUID this is unlikely, but humans
    // tend to do silly things at times, so this check is necessary.
    //
    fstate_src_ty *src2 =
        cid.get_cp()->file_find_uuid(uuid, view_path_simple);
    if (src2)
    {
        sub_context_ty sc(__FILE__, __LINE__);
        sc.var_set_string("Other", src2->file_name);
        sc.var_optional("Other");
        change_fatal(cid.get_cp(), &sc, i18n("$filename uuid duplicate"));
        // NOTREACHED
    }

    //
    // Assign the UUID.  Can't do this before the has-it-been-used test,
    // or the UUID would show as a duplicate.
    //
    // Make sure it is in lower case.
    //
    src->uuid = str_copy(uuid.get_ref());

    //
    // Write the cstate state back out.
    //
    cid.get_cp()->cstate_write();
    commit();
    lock_release();
    sub_context_ty *scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", filename);
    change_verbose(cid.get_cp(), scp, i18n("$filename attributes changed"));
    sub_context_delete(scp);
    trace(("}\n"));
}


void
file_attributes(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, file_attributes_help, 0 },
        { arglex_token_list, file_attributes_list, 0 },
        { arglex_token_uuid, file_attributes_uuid, 0 },
        { arglex_token_report, file_attributes_report, 0 },
    };

    trace(("file_attributes()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), file_attributes_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
