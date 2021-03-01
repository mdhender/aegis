//
// aegis - project change supervisor
// Copyright (C) 2002-2008, 2010-2012 Peter Miller
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
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/gettime.h>
#include <common/now.h>
#include <common/nstring.h>
#include <common/progname.h>
#include <common/str_list.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/attribute.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/change.h>
#include <libaegis/compres_algo.h>
#include <libaegis/file/event.h>
#include <libaegis/help.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/output/filter/bzip2.h>
#include <libaegis/output/filter/gzip.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aedist/send.h>
#include <aetar/arglex3.h>
#include <aetar/output/tar.h>


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
tar_send(void)
{
    int include_build = -1;
    string_ty *output_filename = 0;
    int entire_source = -1;
    compression_algorithm_t needs_compression = compression_algorithm_not_set;
    nstring path_prefix;
    const char *compatibility = 0;
    change_identifier cid;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_baseline:
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

        case arglex_token_entire_source:
            if (entire_source > 0)
                duplicate_option(usage);
            if (entire_source >= 0)
            {
                mutually_exclusive_options
                (
                    arglex_token_entire_source,
                    arglex_token_entire_source_not,
                    usage
                );
            }
            entire_source = 1;
            break;

        case arglex_token_entire_source_not:
            if (entire_source == 0)
                duplicate_option(usage);
            if (entire_source >= 0)
            {
                mutually_exclusive_options
                (
                    arglex_token_entire_source,
                    arglex_token_entire_source_not,
                    usage
                );
            }
            entire_source = 0;
            break;

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

        case arglex_token_path_prefix_add:
            if (!path_prefix.empty())
                duplicate_option(usage);
            if (arglex() != arglex_token_string)
            {
                option_needs_string(arglex_token_delta_date, usage);
                // NOTREACHED
            }
            path_prefix = arglex_value.alv_string;
            break;

        case arglex_token_include_build:
            if (include_build > 0)
                duplicate_option(usage);
            if (include_build >= 0)
            {
                mutually_exclusive_options
                (
                    arglex_token_include_build,
                    arglex_token_include_build_not,
                    usage
                );
            }
            include_build = 1;
            break;

        case arglex_token_include_build_not:
            if (include_build == 0)
                duplicate_option(usage);
            if (include_build >= 0)
            {
                mutually_exclusive_options
                (
                    arglex_token_include_build,
                    arglex_token_include_build_not,
                    usage
                );
            }
            include_build = 0;
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
        }
        arglex();
    }
    cid.command_line_check(usage);

    if (entire_source < 0)
        entire_source = 0;

    //
    // Check the change state.
    //
    project_file_roll_forward *historian = cid.get_historian();

    //
    // Figure what to do for compatibility.
    //
    bool use_bzip2 = true;
    if (compatibility)
    {
        //
        // The bzip compression algorithm was first available in
        // Peter's 4.21.D186, publicly in 4.22
        //
        use_bzip2 = (strverscmp(compatibility, "4.22") >= 0);

        //
        // Add new compatibility tests above this comment.
        //
    }

    //
    // Refine the compression to be used.
    //
    switch (needs_compression)
    {
    case compression_algorithm_not_set:
    case compression_algorithm_unspecified:
        needs_compression =
            (
                use_bzip2
            ?
                compression_algorithm_bzip2
            :
                compression_algorithm_gzip
            );
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

    //
    // open the output
    //
    os_become_orig();
    output::pointer out_fp = output_file::binary_open(output_filename);
    switch (needs_compression)
    {
    case compression_algorithm_not_set:
    case compression_algorithm_unspecified:
        assert(0);
        // fall through...

    case compression_algorithm_none:
        break;

    case compression_algorithm_gzip:
        out_fp = output_filter_gzip::create(out_fp);
        break;

    case compression_algorithm_bzip2:
        out_fp = output_filter_bzip2::create(out_fp);
        break;
    }
    output_tar *tar_p = new output_tar(out_fp);
    os_become_undo();

    //
    // Scan for files to be added to the output.
    //
    string_list_ty wl;
    for (size_t j = 0;; ++j)
    {
        fstate_src_ty   *src_data;

        src_data = change_file_nth(cid.get_cp(), j, view_path_first);
        if (!src_data)
            break;
        switch (src_data->action)
        {
        case file_action_create:
        case file_action_remove:
            break;

        case file_action_insulate:
        case file_action_transparent:
            continue;

        case file_action_modify:
            switch (src_data->usage)
            {
            case file_usage_source:
            case file_usage_config:
            case file_usage_test:
            case file_usage_manual_test:
                break;

            case file_usage_build:
                if (include_build <= 0)
                    continue;
                break;
            }
            break;
        }
        wl.push_back(src_data->file_name);
    }
    if (entire_source)
    {
        //
        // Actually, this list needs to be at the time of the delta.
        // (So, we keep almost all files, and toss them later.)
        //
        if (historian->is_set())
        {
            nstring_list file_name_list;
            historian->keys(file_name_list);
            for (size_t j = 0; j < file_name_list.size(); ++j)
            {
                nstring file_name = file_name_list[j];
                assert(file_name);
                file_event *fep = historian->get_last(file_name);
                assert(fep);
                if (!fep)
                    continue;
                fstate_src_ty *src_data = fep->get_src();
                assert(src_data);
                if (!src_data)
                    continue;
                if
                (
                    attributes_list_find_boolean
                    (
                        src_data->attribute,
                        "entire-source-hide"
                    )
                )
                {
                    continue;
                }
                switch (src_data->usage)
                {
                case file_usage_build:
                    switch (src_data->action)
                    {
                    case file_action_modify:
                    case file_action_remove:
                        continue;

                    case file_action_create:
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
                        break;

                    case file_action_remove:
                        continue;

                    case file_action_insulate:
                    case file_action_transparent:
                        // can't happen
                        assert(0);
                        continue;
                    }
                    break;
                }
                wl.push_back_unique(src_data->file_name);
            }
        }
        else
        {
            for (size_t j = 0;; ++j)
            {
                fstate_src_ty   *src_data;

                src_data = cid.get_pp()->file_nth(j, view_path_extreme);
                if (!src_data)
                    break;
                if
                (
                    attributes_list_find_boolean
                    (
                        src_data->attribute,
                        "entire-source-hide"
                    )
                )
                {
                    continue;
                }
                wl.push_back_unique(src_data->file_name);
            }
        }
    }
    if (!wl.nstrings)
        change_fatal(cid.get_cp(), 0, i18n("bad send no files"));

    //
    // sort the files by name
    //
    wl.sort();

    //
    // add each of the relevant source files to the archive
    //
    if (!path_prefix.empty() && path_prefix[path_prefix.size() - 1] != '/')
        path_prefix = path_prefix + "/";
    for (size_t j = 0; j < wl.nstrings; ++j)
    {
        string_ty       *filename;
        fstate_src_ty   *csrc = 0;
        long            len;
        string_ty       *abs_filename = 0;
        int             abs_filename_unlink = 0;

        filename = wl.string[j];

        switch (cid.get_cp()->cstate_get()->state)
        {
            file_event  *fep;

        case cstate_state_awaiting_development:
            assert(0);
            continue;

        case cstate_state_being_developed:
        case cstate_state_awaiting_review:
        case cstate_state_being_reviewed:
        case cstate_state_awaiting_integration:
        case cstate_state_being_integrated:
            //
            // Get the input file.
            //
            csrc = cid.get_cp()->file_find(nstring(filename), view_path_first);
            if (!csrc)
            {
                csrc = cid.get_pp()->file_find(filename, view_path_simple);
            }
            switch (csrc->action)
            {
            case file_action_create:
            case file_action_modify:
            case file_action_transparent:
                break;

            case file_action_remove:
            case file_action_insulate:
                continue;
            }
            switch (csrc->usage)
            {
            case file_usage_build:
                if (include_build <= 0)
                    continue;
                break;

            case file_usage_source:
            case file_usage_config:
            case file_usage_test:
            case file_usage_manual_test:
                break;
            }
            if (csrc->deleted_by)
                continue;
            if (csrc->about_to_be_created_by)
                continue;

            abs_filename = cid.get_cp()->file_path(csrc->file_name);
            if (!abs_filename)
                abs_filename = project_file_path(cid.get_pp(), csrc->file_name);
            abs_filename_unlink = 0;

            //
            // The executable field is only set by aeipass, so we need
            // to go and look at the file itself for accurate information
            // for changes which are not yet in the completed state.
            //
            os_become_orig();
            csrc->executable = os_executable(abs_filename);
            os_become_undo();
            break;

        case cstate_state_completed:
            //
            // Extract the file from history.
            //
            fep = historian->get_last(filename);
            if (!fep)
                continue;
            csrc = fep->get_src();
            assert(csrc);
            if (!csrc)
                continue;
            switch (csrc->action)
            {
            case file_action_create:
            case file_action_modify:
            case file_action_transparent:
                break;

            case file_action_remove:
            case file_action_insulate:
                continue;
            }
            switch (csrc->usage)
            {
            case file_usage_source:
            case file_usage_config:
            case file_usage_test:
            case file_usage_manual_test:
                break;

            case file_usage_build:
                if (include_build <= 0)
                    continue;
                break;
            }
            abs_filename =
                project_file_version_path
                (
                    cid.get_pp(),
                    csrc,
                    &abs_filename_unlink
                );
            break;
        }

        //
        // Put the whole file into the archive.
        //
        assert(csrc);
        switch (csrc->action)
        {
        case file_action_remove:
        case file_action_transparent:
            break;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
            {
                os_become_orig();
                input ifp = input_file_open(abs_filename);
                assert(ifp.is_open());
                len = ifp->length();
                nstring tar_name = path_prefix + nstring(filename);
                output::pointer ofp =
                    tar_p->child(tar_name, len, csrc->executable);
                ofp << ifp;
                ifp.close();
                ofp.reset();
                os_become_undo();
            }
            break;
        }

        //
        // Free a bunch of strings.
        //
        if (abs_filename_unlink)
        {
            os_become_orig();
            os_unlink_errok(abs_filename);
            os_become_undo();
        }
        str_free(abs_filename);
    }

    //
    // finish writing the tar archive
    //
    os_become_orig();
    delete tar_p;
    os_become_undo();
}


// vim: set ts=8 sw=4 et :
