//
// aegis - project change supervisor
// Copyright (C) 2001-2009, 2011, 2012 Peter Miller
// Copyright (C) 2009 Walter Franzini
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
#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/gettime.h>
#include <common/now.h>
#include <common/progname.h>
#include <common/str.h>
#include <common/trace.h>
#include <common/version_stmp.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/attribute.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/change.h>
#include <libaegis/change/signedoffby.h>
#include <libaegis/compres_algo.h>
#include <libaegis/file/event.h>
#include <libaegis/file/event/list.h>
#include <libaegis/help.h>
#include <libaegis/input/file.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/output/conten_encod.h>
#include <libaegis/output/file.h>
#include <libaegis/output/filter/base64.h>
#include <libaegis/output/filter/bzip2.h>
#include <libaegis/output/filter/gzip.h>
#include <libaegis/output/filter/prefix.h>
#include <libaegis/output/wrap.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aepatch/send.h>
#include <aepatch/arglex3.h>


#define NO_TIME_SET ((time_t)(-1))


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s --send [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


void
patch_send(void)
{
    change_identifier cid;
    const char *compatibility = 0;
    string_ty *original_filename = 0;
    string_ty *input_filename = 0;
    string_ty *output_file_name;
    string_ty *output_filename = 0;
    string_ty *s;
    content_encoding_t ascii_armor = content_encoding_unset;
    string_ty *dev_null;
    compression_algorithm_t needs_compression = compression_algorithm_not_set;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_trunk:
            cid.command_line_parse(usage);
            continue;

        case arglex_token_output:
            if (output_filename)
                duplicate_option(usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_output, usage);
                // NOTREACHED

            case arglex_token_stdio:
                output_filename = str_from_c("");
                break;

            case arglex_token_string:
                output_filename = str_from_c(arglex_value.alv_string);
                break;
            }
            break;

        case arglex_token_ascii_armor:
            if (ascii_armor != content_encoding_unset)
            {
                duplicate_option_by_name
                (
                    arglex_token_content_transfer_encoding,
                    usage
                );
            }
            ascii_armor = content_encoding_base64;
            break;

        case arglex_token_ascii_armor_not:
            if (ascii_armor != content_encoding_unset)
            {
                duplicate_option_by_name
                (
                    arglex_token_content_transfer_encoding,
                    usage
                );
            }
            ascii_armor = content_encoding_none;
            break;

        case arglex_token_content_transfer_encoding:
            if (ascii_armor != content_encoding_unset)
                duplicate_option(usage);
            if (arglex() != arglex_token_string)
            {
                option_needs_string
                (
                    arglex_token_content_transfer_encoding,
                    usage
                );
            }
            ascii_armor = content_encoding_grok(arglex_value.alv_string);
            break;

        case arglex_token_compress:
            if (needs_compression != compression_algorithm_not_set)
            {
                duplicate_option_by_name
                (
                    arglex_token_compression_algorithm,
                    usage
                );
            }
            needs_compression = compression_algorithm_unspecified;
            break;

        case arglex_token_compress_not:
            if (needs_compression != compression_algorithm_not_set)
            {
                duplicate_option_by_name
                (
                    arglex_token_compression_algorithm,
                    usage
                );
            }
            needs_compression = compression_algorithm_none;
            break;

        case arglex_token_compression_algorithm:
            if (arglex() != arglex_token_string)
            {
                option_needs_string(arglex_token_compression_algorithm, usage);
                // NOTREACHED
            }
            else
            {
                compression_algorithm_t temp =
                    compression_algorithm_by_name(arglex_value.alv_string);

                //
                // We don't complain if the answer is going to be the same,
                // for compatibility with the old options.
                //
                if (temp == needs_compression)
                    break;

                switch (needs_compression)
                {
                case compression_algorithm_not_set:
                case compression_algorithm_unspecified:
                    needs_compression = temp;
                    break;

                case compression_algorithm_none:
                case compression_algorithm_gzip:
                case compression_algorithm_bzip2:
                    duplicate_option_by_name
                    (
                        arglex_token_compression_algorithm,
                        usage
                    );
                    // NOTREACHED
                }
            }
            break;

        case arglex_token_compatibility:
            if (compatibility)
                duplicate_option(usage);
            switch (arglex())
            {
            case arglex_token_string:
            case arglex_token_number:
                compatibility = arglex_value.alv_string;
                break;

            default:
                option_needs_string(arglex_token_compatibility, usage);
                // NOTREACHED
            }
            break;

        case arglex_token_signed_off_by:
        case arglex_token_signed_off_by_not:
            option_signed_off_by_argument(usage);
            break;
        }
        arglex();
    }

    //
    // reject illegal combinations of options
    //
    cid.command_line_check(usage);

    //
    // Translate the compatibility version number into a set of
    // capabilities.
    //
    int use_meta_data = 1;
    bool use_bzip2 = true;
    if (compatibility)
    {
        //
        // FIXME: should we check that it actually looks like a version
        // string?
        //

        //
        // Patch meta-data were introduced in Peter's 4.16.D081,
        // publicly in 4.17
        //
        use_meta_data = (strverscmp(compatibility, "4.17") >= 0);

        //
        // Use the bzip compression algorithm, introduced
        // in Peter's 4.21.D147, publicly in 4.22
        //
        use_bzip2 = (strverscmp(compatibility, "4.22") >= 0);
    }

    //
    // If the user asked for one, append a Signed-off-by line to this
    // change's description.  (Since we don't write the cstate back out,
    // it is safe to change the change's description.)
    //
    if (option_signed_off_by_get(false))
        change_signed_off_by(cid.get_cp(), cid.get_up());

    //
    // Check the change state.
    //
    cstate_ty *cstate_data = cid.get_cp()->cstate_get();
    project_file_roll_forward *historian = 0;
    switch (cstate_data->state)
    {
#ifndef DEBUG
    default:
#endif
    case cstate_state_awaiting_development:
        change_fatal(cid.get_cp(), 0, i18n("bad patch send state"));

    case cstate_state_completed:
        //
        // Need to reconstruct the appropriate file histories.
        //
        trace(("project = \"%s\"\n", cid.get_pp()->name_get()->str_text));
        historian = cid.get_historian(false);
        break;

    case cstate_state_being_integrated:
    case cstate_state_awaiting_integration:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_review:
    case cstate_state_being_developed:
        break;
    }

    // open the output
    os_become_orig();
    if (ascii_armor == content_encoding_unset)
        ascii_armor = content_encoding_base64;
    switch (needs_compression)
    {
    case compression_algorithm_not_set:
        if (ascii_armor == content_encoding_none)
        {
            needs_compression = compression_algorithm_none;
            break;
        }
        // Fall through...

    case compression_algorithm_unspecified:
        if (use_bzip2)
            needs_compression = compression_algorithm_bzip2;
        else
            needs_compression = compression_algorithm_gzip;
        break;

    case compression_algorithm_none:
        break;

    case compression_algorithm_gzip:
        use_bzip2 = false;
        break;

    case compression_algorithm_bzip2:
        use_bzip2 = true;
        break;
    }
    output::pointer ofp;
    if
    (
        ascii_armor == content_encoding_none
    &&
        needs_compression != compression_algorithm_none
    )
        ofp = output_file::binary_open(output_filename);
    else
        ofp = output_file::text_open(output_filename);
    ofp->fputs("MIME-Version: 1.0\n");
    ofp->fputs("Content-Type: application/aegis-patch\n");
    content_encoding_header(ofp, ascii_armor);
    nstring proj(cid.get_pp()->name_get());
    nstring s2(cstate_data->brief_description);
    ofp->fprintf
    (
        "Subject: %.*s - %.*s\n",
        proj.len_printable(40),
        proj.c_str(),
        s2.len_printable(80),
        s2.c_str()
    );
    ofp->fprintf
    (
        "Content-Name: %s.C%3.3ld.patch%s\n",
        cid.get_pp()->name_get()->str_text,
        cid.get_change_number(),
        compression_algorithm_extension(needs_compression)
    );
    ofp->fprintf
    (
        "Content-Disposition: attachment; filename=%s.C%3.3ld.patch%s\n",
        cid.get_pp()->name_get()->str_text,
        cid.get_change_number(),
        compression_algorithm_extension(needs_compression)
    );
    ofp->fprintf
    (
        "X-Aegis-Project-Name: %s\n",
        cid.get_pp()->name_get()->str_text
    );
    ofp->fprintf
    (
        "X-Aegis-Change-Number: %ld\n",
        cid.get_change_number()
    );
    ofp->fputc('\n');
    ofp = output_content_encoding(ofp, ascii_armor);
    switch (needs_compression)
    {
    case compression_algorithm_none:
        break;

    case compression_algorithm_not_set:
    case compression_algorithm_unspecified:
        assert(0);
        // Fall through...

    case compression_algorithm_bzip2:
        ofp = output_filter_bzip2::create(ofp);
        break;

    case compression_algorithm_gzip:
        ofp = output_filter_gzip::create(ofp);
        break;
    }

    //
    // Add the change details to the archive.
    // This is done as a simple comment.
    //
    {
        output::pointer t2 = output_filter_prefix::create(ofp, "#\t");
        t2 = output_wrap_open(t2, 70);
        os_become_undo();
        nstring brief = cid.get_cp()->brief_description_get();
        nstring desc = cid.get_cp()->description_get();
        os_become_orig();
        t2->fputs("\nSummary:\n");
        t2->fputs(brief);
        t2->end_of_line();
        t2->fputs("\nDescription:\n");
        t2->fputs(desc);
        t2->end_of_line();
        t2->fputc('\n');
    }
    os_become_undo();

    if (use_meta_data)
    {
        cstate_ty       *change_set;

        //
        // Build the meta-data to write to the output.
        //
        change_set = (cstate_ty *)cstate_type.alloc();
        change_set->brief_description =
            str_copy(cstate_data->brief_description);
        change_set->description = str_copy(cstate_data->description);
        change_set->cause = cstate_data->cause;
        change_set->test_exempt = cstate_data->test_exempt;
        change_set->test_baseline_exempt = cstate_data->test_baseline_exempt;
        change_set->regression_test_exempt =
            cstate_data->regression_test_exempt;
        if (cstate_data->attribute)
        {
            change_set->attribute =
                attributes_list_copy(cstate_data->attribute);
            //
            // Remove attributes that should not be shared with a
            // different repository.
            //
            attributes_list_remove(change_set->attribute, HISTORY_GET_COMMAND);
        }
        if (cstate_data->uuid)
            change_set->uuid = str_copy(cstate_data->uuid);
        // architecture
        // copyright years

        for (size_t j = 0;; ++j)
        {
            fstate_src_ty   *src_data;
            cstate_src_ty   **dst_data_p;
            cstate_src_ty   *dst_data;
            meta_type *type_p = 0;

            src_data = change_file_nth(cid.get_cp(), j, view_path_first);
            if (!src_data)
                break;
            switch (src_data->usage)
            {
            case file_usage_build:
                switch (src_data->action)
                {
                case file_action_modify:
                    continue;

                case file_action_create:
                case file_action_remove:
                case file_action_insulate:
                case file_action_transparent:
                    break;
                }
                // fall through...

            case file_usage_source:
            case file_usage_config:
            case file_usage_test:
            case file_usage_manual_test:
                switch (src_data->action)
                {
                case file_action_create:
                case file_action_modify:
                    if (cstate_data->state < cstate_state_completed)
                    {
                        s = cid.get_cp()->file_path(src_data->file_name);
                        assert(s);
                        if (s)
                        {
                            os_become_orig();
                            src_data->executable = os_executable(s);
                            os_become_undo();
                            str_free(s);
                        }
                    }
                    break;

                case file_action_remove:
                    break;

                case file_action_insulate:
                    continue;

                case file_action_transparent:
                    if
                    (
                        src_data->about_to_be_created_by
                    ||
                        src_data->about_to_be_copied_by
                    )
                        continue;
                    break;
                }
                break;
            }

            //
            // Add the file to the list.
            //
            trace(("add \"%s\"\n", src_data->file_name->str_text));
            if (!change_set->src)
                change_set->src =
                    (cstate_src_list_ty *)cstate_src_list_type.alloc();
            dst_data_p =
                (cstate_src_ty **)
                cstate_src_list_type.list_parse(change_set->src, &type_p);
            assert(type_p == &cstate_src_type);
            dst_data = (cstate_src_ty *)cstate_src_type.alloc();
            *dst_data_p = dst_data;
            dst_data->file_name = str_copy(src_data->file_name);
            dst_data->action = src_data->action;
            dst_data->usage = src_data->usage;
            dst_data->executable = src_data->executable;
            if (src_data->move)
                dst_data->move = str_copy(src_data->move);
            if (src_data->attribute)
                dst_data->attribute = attributes_list_copy(src_data->attribute);
            if (src_data->uuid)
                dst_data->uuid = str_copy(src_data->uuid);
        }
        if (!change_set->src || !change_set->src->length)
            change_fatal(cid.get_cp(), 0, i18n("bad send no files"));

        //
        // Now write the change set out.
        //
        os_become_orig();
        ofp->fputs("# Aegis-Change-Set-Begin\n");
        {
            output::pointer t2 = output_filter_prefix::create(ofp, "# ");
            t2 = output_filter_base64::create(t2);
            if (use_bzip2)
                t2 = output_filter_bzip2::create(t2);
            else
                t2 = output_filter_gzip::create(t2);
            cstate_write(t2, change_set);
        }
        ofp->fputs("# Aegis-Change-Set-End\n#\n");
        os_become_undo();

        cstate_type.free(change_set);
    }

    //
    // We need a whole bunch of temporary files.
    //
    output_file_name = os_edit_filename(0);
    os_become_orig();
    undo_unlink_errok(output_file_name);
    os_become_undo();
    dev_null = str_from_c("/dev/null");

    //
    // Add each of the relevant source files to the patch.
    //
    for (size_t j = 0;; ++j)
    {
        fstate_src_ty   *csrc;

        int original_filename_unlink = 0;
        int input_filename_unlink = 0;

        csrc = change_file_nth(cid.get_cp(), j, view_path_first);
        if (!csrc)
            break;
        trace(("fn = \"%s\"\n", csrc->file_name->str_text));
        switch (csrc->usage)
        {
        case file_usage_source:
        case file_usage_config:
        case file_usage_test:
        case file_usage_manual_test:
            break;

        case file_usage_build:
            continue;
        }

        //
        // Find a source file.  Depending on the change state,
        // it could be in the development directory, or in the
        // baseline or in history.
        //
        // original_filename
        //      The oldest version of the file.
        // input_filename
        //      The youngest version of the file.
        // input_filename
        //      Where to write the output.
        //
        // These names are taken from the substitutions for
        // the diff_command.  It's historical.
        //
        switch (cstate_data->state)
        {
        case cstate_state_awaiting_development:
            assert(0);
            continue;

        case cstate_state_being_developed:
        case cstate_state_awaiting_review:
        case cstate_state_being_reviewed:
        case cstate_state_awaiting_integration:
        case cstate_state_being_integrated:
            switch (csrc->action)
            {
            case file_action_create:
                original_filename = str_copy(dev_null);
                break;

            case file_action_modify:
            case file_action_remove:
            case file_action_insulate:
            case file_action_transparent:
#ifndef DEBUG
            default:
#endif
                original_filename =
                    project_file_version_path
                    (
                        cid.get_pp(),
                        csrc,
                        &original_filename_unlink
                    );
                break;
            }
            switch (csrc->action)
            {
            case file_action_create:
            case file_action_modify:
            case file_action_insulate:
            case file_action_transparent:
#ifndef DEBUG
            default:
#endif
                input_filename = cid.get_cp()->file_path(csrc->file_name);
                break;

            case file_action_remove:
                input_filename = str_copy(dev_null);
                break;
            }
            break;

        case cstate_state_completed:
            //
            // Both the versions to be diffed come out
            // of history.
            //
            switch (csrc->action)
            {
            case file_action_create:
                original_filename = dev_null;
                input_filename =
                    project_file_version_path
                    (
                        cid.get_pp(),
                        csrc,
                        &input_filename_unlink
                    );
                break;

            case file_action_remove:
                {
                    assert(historian);
                    file_event_list::pointer felp =
                        historian->get(csrc->file_name);

                    //
                    // It's tempting to say
                    //      assert(felp);
                    // but file file may not yet exist at this point in
                    // time, so there is no need (or ability) to create a
                    // patch for it.
                    //
                    if (!felp)
                    {
                        original_filename = str_copy(dev_null);
                        input_filename = str_copy(dev_null);
                        break;
                    }

                    //
                    // It is tempting to say
                    //     assert(felp->length >= 2);
                    // except that a file which is created and removed in
                    // the same branch, will result in only a remove record
                    // in its parent branch when integrated.
                    //
                    assert(!felp->empty());

                    if (felp->size() < 2)
                    {
                        original_filename = str_copy(dev_null);
                    }
                    else
                    {
                        file_event *fep = felp->get(felp->size() - 2);
                        fstate_src_ty *old_src = fep->get_src();
                        assert(old_src);
                        original_filename =
                            project_file_version_path
                            (
                                cid.get_pp(),
                                old_src,
                                &original_filename_unlink
                            );
                    }

                    input_filename = str_copy(dev_null);
                }
                break;

#ifndef DEBUG
            default:
#endif
            case file_action_modify:
            case file_action_insulate:
            case file_action_transparent:
                {
                    assert(historian);
                    file_event_list::pointer felp =
                        historian->get(csrc->file_name);

                    //
                    // It's tempting to say
                    //      assert(felp);
                    // but file file may not yet exist at this point in
                    // time, so there is no need (or ability) to create a
                    // patch for it.
                    //
                    if (!felp)
                    {
                        original_filename = str_copy(dev_null);
                        input_filename = str_copy(dev_null);
                        break;
                    }

                    assert(felp->size() >= 2);

                    file_event *fep = felp->get(felp->size() - 2);
                    fstate_src_ty *old_src = fep->get_src();
                    assert(old_src);
                    original_filename =
                        project_file_version_path
                        (
                            cid.get_pp(),
                            old_src,
                            &original_filename_unlink
                        );

                    fep = felp->back();
                    old_src = fep->get_src();
                    assert(old_src);
                    input_filename =
                        project_file_version_path
                        (
                            cid.get_pp(),
                            old_src,
                            &input_filename_unlink
                        );
                }
                break;
            }
            break;
        }

        //
        // If they are both /dev/null don't bother with a patch.
        //
        trace(("original_filename = \"%s\"\n", original_filename->str_text));
        trace(("input_filename = \"%s\"\n", input_filename->str_text));
        if
        (
            str_equal(original_filename, dev_null)
        &&
            str_equal(input_filename, dev_null)
        )
        {
            str_free(original_filename);
            str_free(input_filename);
            continue;
        }

        //
        // Generate the difference file.
        //
        assert(original_filename);
        assert(input_filename);
        change_run_patch_diff_command
        (
            cid.get_cp(),
            cid.get_up(),
            original_filename,
            input_filename,
            output_file_name,
            csrc->file_name
        );

        os_become_orig();
        if (original_filename_unlink)
        {
            os_unlink_errok(original_filename);
            str_free(original_filename);
        }
        if (input_filename_unlink)
        {
            os_unlink_errok(input_filename);
            str_free(input_filename);
        }

        //
        // Read the diff into the patch output.
        //
        trace(("open \"%s\"\n", output_file_name->str_text));
        input ifp = input_file_open(output_file_name, true);
        assert(ifp.is_open());
        if (ifp->length() != 0)
        {
            ofp->fputs("Index: ");
            ofp->fputs(csrc->file_name);
            ofp->fputc('\n');
            ofp << ifp;
        }
        ifp.close();
        os_become_undo();
        str_free(output_file_name);
    }

    //
    // Get rid of all the temporary files.
    //
    os_become_orig();
    os_unlink_errok(output_file_name);
    str_free(output_file_name);
    str_free(dev_null);

    //
    // Mark the end of the patch.
    //
    ofp.reset();
    os_become_undo();
}


// vim: set ts=8 sw=4 et :
