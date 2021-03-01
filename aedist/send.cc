//
// aegis - project change supervisor
// Copyright (C) 1999-2009, 2011, 2012 Peter Miller
// Copyright (C) 2004, 2005, 2007-2010 Walter Franzini
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
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/gettime.h>
#include <common/mem.h>
#include <common/now.h>
#include <common/str.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <common/version_stmp.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/attribute.h>
#include <libaegis/change/attributes.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/functor/attribu_list.h>
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
#include <libaegis/output/cpio.h>
#include <libaegis/output/file.h>
#include <libaegis/output/filter/bzip2.h>
#include <libaegis/output/filter/gzip.h>
#include <libaegis/output/filter/indent.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/project/invento_walk.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aeannotate/usage.h>
#include <aedist/arglex3.h>
#include <aedist/send.h>


#define NO_TIME_SET ((time_t)(-1))
static bool use_uuid;


static bool
have_it_already(cstate_ty *change_set, fstate_src_ty *src_data)
{
    size_t          j;
    cstate_src_ty   *dst_data;

    if (!change_set->src)
        return false;

    //
    // first we look by uuid, we *must* use also the name in order to
    // properly handle renames.  With renames we have 2 entries with
    // the same UUID.
    //
    if (src_data->uuid)
    {
        for (j = 0; j < change_set->src->length; ++j)
        {
            dst_data = change_set->src->list[j];
            if
            (
                dst_data->uuid
            &&
                str_equal(dst_data->uuid, src_data->uuid)
            &&
                str_equal(dst_data->file_name, src_data->file_name)
            )
                return true;
        }
    }

    // second we look by name
    for (j = 0; j < change_set->src->length; ++j)
    {
        dst_data = change_set->src->list[j];
        if (str_equal(dst_data->file_name, src_data->file_name))
            return true;
    }
    return false;
}


static void
one_more_src(project_file_roll_forward &historian, cstate_ty *change_set,
    fstate_src_ty *src_data, int use_attr)
{
    cstate_src_ty   **dst_data_p;
    cstate_src_ty   *dst_data;
    meta_type *type_p = 0;

    trace(("add \"%s\" %s %s %d\n", src_data->file_name->str_text,
        file_action_ename(src_data->action),
        file_usage_ename(src_data->usage), use_attr));
    if (!change_set->src)
        change_set->src = (cstate_src_list_ty *)cstate_src_list_type.alloc();
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
    if (use_attr)
    {
        if (src_data->attribute)
            dst_data->attribute = attributes_list_copy(src_data->attribute);
        if (use_uuid && src_data->uuid)
            dst_data->uuid = str_copy(src_data->uuid);

        //
        // If the historian is not set we can not retrieve the needed
        // information, so we return.
        //
        trace(("historian = %d;\n", historian.is_set() ? 1 : 0));
        if (!historian.is_set())
            return;

        //
        // We handle only modified files.
        //
        // todo:
        // We should handle also file renames because the rename
        // operation may carry also file modifications.  However the
        // aemv command does not support the -delta option, yet.
        //
        trace(("src_data->action = %s;\n",
               file_action_ename(src_data->action)));
        switch (src_data->action)
        {
        case file_action_modify:
            break;

        case file_action_insulate:
            assert(0);
            // FALLTHROUGH

        case file_action_create:
        case file_action_transparent:
        case file_action_remove:
#ifndef DEBUG
        default:
#endif
            return;
            break;
        }

        trace(("fn=\"%s\";\nft = %s;\n",
               src_data->file_name->str_text,
               file_action_ename(src_data->action)));
        assert(historian.is_set());
        file_event *fep = historian.get_older(src_data->file_name);

        //
        // The fep *can* be NULL if the change is not completed
        // (e.g. for renamed files).  At the moment we assume this is
        // not the case.
        //
        if (!fep)
            return;

        assert(fep);
        if (!fep->get_change()->cstate_data->uuid || !use_uuid)
            return;
        if (!src_data->edit_origin)
            return;
        assert(src_data->edit_origin->revision);
        assert(fep->get_src());
        if (!fep->get_src()->edit || !fep->get_src()->edit->revision)
            return;
        trace_string(fep->get_change()->cstate_data->uuid->str_text);
        if
        (
            !str_equal
            (
                src_data->edit_origin->revision,
                fep->get_src()->edit->revision
            )
        )
        {
            //
            // It's tempting to say
            //
            // assert(0);
            //
            // but the assumption is not true for outstanding changes.
            //
            return;
        }

        if (!dst_data->attribute)
            dst_data->attribute =
                (attributes_list_ty*)attributes_list_type.alloc();
        attributes_list_append
        (
            dst_data->attribute,
            EDIT_ORIGIN_UUID,
            fep->get_change()->cstate_data->uuid->str_text
        );
    }
}


static int
cmp(const void *va, const void *vb)
{
    cstate_src_ty   *a;
    cstate_src_ty   *b;

    a = *(cstate_src_ty **)va;
    b = *(cstate_src_ty **)vb;
    return strcmp(a->file_name->str_text, b->file_name->str_text);
}


void
send_main(void)
{
    int             use_attributes;
    int             use_patch;
    int             use_change_number;
    int             use_config;
    int             use_rename_patch;
    string_ty       *project_name;
    long            change_number;
    output::pointer ofp;
    project      *pp;
    change::pointer cp;
    user_ty::pointer up;
    cstate_ty       *cstate_data;
    string_ty       *output_filename;
    cstate_ty       *change_set;
    time_t          when;
    size_t          j;
    int             description_header;
    int             baseline;
    int             entire_source;
    content_encoding_t ascii_armor;
    string_ty       *dev_null;
    string_ty       *diff_output_filename;
    long            delta_number;
    time_t          delta_date;
    const char      *delta_name;
    const char      *compatibility;
    bool undocumented_testing_flag = false;

    arglex();
    compatibility = 0;
    change_number = 0;
    project_name = 0;
    output_filename = 0;
    description_header = -1;
    baseline = 0;
    entire_source = -1;
    ascii_armor = content_encoding_unset;
    compression_algorithm_t needs_compression = compression_algorithm_not_set;
    delta_date = NO_TIME_SET;
    delta_number = -1;
    delta_name = 0;
    int use_mime_header = -1;
    int ignore_uuid = -1;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_baseline:
            if (baseline)
                duplicate_option(usage);
            baseline = 1;
            break;

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

        case arglex_token_change:
        case arglex_token_delta_from_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change(&project_name, &change_number, usage);
            continue;

        case arglex_token_project:
            arglex();
            arglex_parse_project(&project_name, usage);
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

        case arglex_token_description_header:
            if (description_header == 1)
                duplicate_option(usage);
            else if (description_header >= 0)
            {
                head_desc_yuck:
                mutually_exclusive_options
                (
                    arglex_token_description_header,
                    arglex_token_description_header_not,
                    usage
                );
            }
            description_header = 1;
            break;

        case arglex_token_description_header_not:
            if (description_header == 0)
                duplicate_option(usage);
            else if (description_header >= 0)
                goto head_desc_yuck;
            description_header = 0;
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

        case arglex_token_mime_header:
            if (use_mime_header >= 0)
                duplicate_option(usage);
            use_mime_header = 1;
            break;

        case arglex_token_mime_header_not:
            if (use_mime_header >= 0)
                duplicate_option(usage);
            use_mime_header = 0;
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

        case arglex_token_delta:
            if (delta_number >= 0 || delta_name)
                duplicate_option(usage);
            switch (arglex())
            {
            default:
                option_needs_number(arglex_token_delta, usage);
                // NOTREACHED

            case arglex_token_number:
                delta_number = arglex_value.alv_number;
                if (delta_number < 0)
                {
                    sub_context_ty *scp;

                    scp = sub_context_new();
                    sub_var_set_long(scp, "Number", delta_number);
                    fatal_intl(scp, i18n("delta $number out of range"));
                    // NOTREACHED
                    sub_context_delete(scp);
                }
                break;

            case arglex_token_string:
                delta_name = arglex_value.alv_string;
                break;
            }
            break;

        case arglex_token_delta_date:
            if (delta_date != NO_TIME_SET)
                duplicate_option(usage);
            if (arglex() != arglex_token_string)
            {
                option_needs_string(arglex_token_delta_date, usage);
                // NOTREACHED
            }
            delta_date = date_scan(arglex_value.alv_string);
            if (delta_date == NO_TIME_SET)
                fatal_date_unknown(arglex_value.alv_string);
            break;

        case arglex_token_patch:
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set_charstar
                (
                    scp,
                    "Name1",
                    arglex_token_name(arglex_token_patch)
                );
                sub_var_set_charstar
                (
                    scp,
                    "Name2",
                    arglex_token_name(arglex_token_compatibility)
                );
                error_intl
                (
                    scp,
                    i18n("warning: $name1 obsolete, use $name2 option")
                );
                sub_context_delete(scp);

                if (compatibility)
                    duplicate_option_by_name(arglex_token_compatibility, usage);
                compatibility = "99999999";
            }
            break;

        case arglex_token_patch_not:
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set_charstar
                (
                    scp,
                    "Name1",
                    arglex_token_name(arglex_token_patch)
                );
                sub_var_set_format
                (
                    scp,
                    "Name2",
                    "%s=4.6",
                    arglex_token_name(arglex_token_compatibility)
                );
                error_intl
                (
                    scp,
                    i18n("warning: $name1 obsolete, use $name2 option")
                );
                sub_context_delete(scp);

                if (compatibility)
                    duplicate_option_by_name(arglex_token_compatibility, usage);
                compatibility = "4.6";
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

        case arglex_token_ignore_uuid:
        case arglex_token_uuid_not:
            if (ignore_uuid > 0)
                duplicate_option(usage);
            if (ignore_uuid >= 0)
            {
                too_many_ignore_uuid:
                mutually_exclusive_options
                (
                    arglex_token_ignore_uuid,
                    arglex_token_ignore_uuid_not,
                    usage
                );
            }
            ignore_uuid = 1;
            break;

        case arglex_token_uuid:
        case arglex_token_ignore_uuid_not:
            if (ignore_uuid == 0)
                duplicate_option(usage);
            if (ignore_uuid >= 0)
                goto too_many_ignore_uuid;
            ignore_uuid = 0;
            break;

        case arglex_token_test:
            // This option may be used by the test suite to insert
            // zero-valued timestamps into the .ae file, so that
            // automated tests don't get false negatives just because
            // the timestamp has changed.
            undocumented_testing_flag = true;
            break;
        }
        arglex();
    }
    if (entire_source < 0)
        entire_source = baseline;

    //
    // Translate the compatibility version number into a set of
    // capabilities.
    //
    use_patch = 1;
    use_change_number = 1;
    use_config = 1;
    use_attributes = 1; // implies UUIDs as well
    use_rename_patch = 1;
    bool use_bzip2 = true;
    if (compatibility)
    {
        //
        // FIXME: should we check that it actually looks like a version
        // string?
        //

        //
        // Patches were introduced in Peter's 4.6.D022,
        // publicly in 4.7
        //
        use_patch = (strverscmp(compatibility, "4.7") >= 0);

        //
        // Change numbers were introduced in Peter's 4.9.D035,
        // publicly in 4.10
        //
        use_change_number = (strverscmp(compatibility, "4.10") >= 0);

        //
        // The config file usage was introduced in Peter's 4.16.D024,
        // publicly in 4.17
        //
        use_config = (strverscmp(compatibility, "4.17") >= 0);

        //
        // The file attributes and change attributes were added to
        // aedist -send in Peter's 4.16.D054, publicly in 4.17
        //
        // The change UUIDs were added to
        // aedist -send in Peter's 4.16.D089, publicly in 4.17
        //
        use_attributes = use_config;
        if (!use_attributes && ignore_uuid < 0)
            ignore_uuid = 1;

        //
        // The patch for renamed files were added to aedist -send in
        // Peter's 4.18.D004, publicly in 4.19
        //
        use_rename_patch = (strverscmp(compatibility, "4.19") >= 0);

        if (use_mime_header < 0)
        {
            // This was available in Peter's 4.21.Dnnn, publicly in 4.22
            use_mime_header = (strverscmp(compatibility, "4.22") >= 0);
        }

        //
        // Use the bzip compression algorithm.
        // Peter's 4.21.D148, publicly in 4.22
        //
        use_bzip2 = (strverscmp(compatibility, "4.22") >= 0);

        //
        // Add new compatibility tests above this comment.
        //
        // If you add anything more to this set of flags, you MUST
        // also update the aeget/get/change/download.cc file, so that
        // downloads are possible.
        //
    }
    if (entire_source)
    {
        use_patch = 0;
        use_rename_patch = 0;
    }

    //
    // reject illegal combinations of options
    //
    if (change_number && baseline)
    {
        mutually_exclusive_options
        (
            arglex_token_change,
            arglex_token_baseline,
            usage
        );
    }
    if
    (
        (
            (delta_name || delta_number >= 0)
        +
            !!change_number
        +
            (delta_date != NO_TIME_SET)
        )
    >
        1
    )
    {
        mutually_exclusive_options3
        (
            arglex_token_delta,
            arglex_token_delta_date,
            arglex_token_change,
            usage
        );
    }
    use_uuid = (ignore_uuid <= 0);

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
        project_name = n.get_ref_copy();
    }
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // it is an error if the delta does not exist
    //
    if (delta_name)
    {
        string_ty       *s1;

        s1 = str_from_c(delta_name);
        change_number = project_history_change_by_name(pp, s1, 0);
        str_free(s1);
        delta_name = 0;
    }
    when = now();
    if (delta_date != NO_TIME_SET)
    {
        //
        // If the time is in the future, you could get a different
        // answer for the same input at some point in the future.
        //
        // This is the "time safe" quality first described by
        // Damon Poole <damon@ede.com>
        //
        if (delta_date > when)
            project_error(pp, 0, i18n("date in the future"));

        //
        // Now find the change number corresponding.
        //
        change_number = project_history_change_by_timestamp(pp, delta_date);
    }
    if (delta_number >= 0)
    {
        // does not return if no such delta number
        change_number = project_history_change_by_delta(pp, delta_number);
        delta_number = 0;
    }

    //
    // locate change data
    //
    if (baseline)
        cp = change_copy(pp->change_get());
    else
    {
        if (!change_number)
            change_number = up->default_change(pp);
        cp = change_alloc(pp, change_number);
        change_bind_existing(cp);
    }

    //
    // If the user asked for one, append a Signed-off-by line to this
    // change's description.  (Since we don't write the cstate back out,
    // it is safe to change the change's description.)
    //
    if (option_signed_off_by_get(false))
        change_signed_off_by(cp, up);

    //
    // Check the change state.
    //
    cstate_data = cp->cstate_get();
    project_file_roll_forward historian;
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
#ifndef DEBUG
    default:
#endif
        change_fatal(cp, 0, i18n("bad send state"));
        // NOTREACHED

    case cstate_state_being_integrated:
    case cstate_state_awaiting_integration:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_review:
    case cstate_state_being_developed:
        break;

    case cstate_state_completed:
        //
        // Need to reconstruct the appropriate file histories even for
        // outstanding changes because some file may be renamed and we
        // need to extract the old file from the baseline.
        //
        historian.set
        (
            pp,
            (
                (delta_date != NO_TIME_SET)
            ?
                delta_date
            :
                cp->completion_timestamp()
            ),
            0
        );
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
    case compression_algorithm_gzip:
    case compression_algorithm_bzip2:
        break;
    }
    if (ascii_armor == content_encoding_none)
    {
        ofp = output_file::binary_open(output_filename);
    }
    else
    {
        ofp = output_file::text_open(output_filename);
    }

    //
    // The mime header is conditional.
    //
    if (use_mime_header < 0)
        use_mime_header = (ascii_armor != content_encoding_none);
    if (use_mime_header)
    {
        ofp->fputs("MIME-Version: 1.0\n");
        ofp->fputs("Content-Type: application/aegis-change-set\n");
        content_encoding_header(ofp, ascii_armor);
        nstring s1(project_name_get(pp));
        nstring s2(cstate_data->brief_description);
        if (entire_source)
            s2 = project_description_get(pp);
        ofp->fprintf
        (
            "Subject: %.*s - %.*s\n",
            s1.len_printable(40),
            s1.c_str(),
            s2.len_printable(80),
            s2.c_str()
        );
        if (change_number && !entire_source)
        {
            ofp->fprintf
            (
                "Content-Name: %s.C%3.3ld.ae\n",
                project_name_get(pp).c_str(),
                change_number
            );
            ofp->fprintf
            (
                "Content-Disposition: attachment; filename=%s.C%3.3ld.ae\n",
                project_name_get(pp).c_str(),
                change_number
            );
        }
        else
        {
            ofp->fprintf
            (
                "Content-Name: %s.ae\n",
                project_name_get(pp).c_str()
            );
            ofp->fprintf
            (
                "Content-Disposition: attachment; filename=%s.ae\n",
                project_name_get(pp).c_str()
            );
        }
        ofp->fputc('\n');
    }

    ofp = output_content_encoding(ofp, ascii_armor);
    switch (needs_compression)
    {
    case compression_algorithm_not_set:
        assert(0);
        break;

    case compression_algorithm_none:
        break;

    case compression_algorithm_unspecified:
        assert(0);
        // Fall through...

    case compression_algorithm_gzip:
        ofp = output_filter_gzip::create(ofp);
        break;

    case compression_algorithm_bzip2:
        ofp = output_filter_bzip2::create(ofp);
        break;
    }
    time_t archive_mtime = 0;
    if (!undocumented_testing_flag)
        archive_mtime = cp->completion_timestamp();
    output_cpio *cpio_p = new output_cpio(ofp, archive_mtime);

    //
    // Add the project name to the archive.
    //
    nstring childs_name = "etc/project-name";
    ofp = cpio_p->child(childs_name, -1);
    ofp->fprintf("%s\n", project_name_get(pp).c_str());
    ofp.reset();

    //
    // Add the change number to the archive.
    //
    if (use_change_number)
    {
        childs_name = "etc/change-number";
        output::pointer op = cpio_p->child(childs_name, -1);
        op->fprintf("%ld\n", change_number);
    }
    os_become_undo();

    //
    // Add the change details to the archive.
    //
    change_set = (cstate_ty *)cstate_type.alloc();
    if (entire_source)
    {
        change_set->brief_description =
            project_description_get(pp).get_ref_copy();
    }
    else
    {
        change_set->brief_description =
            str_copy(cstate_data->brief_description);
    }
    if (description_header)
    {
        string_ty       *warning;

        warning = 0;
        if
        (
            entire_source
        ?
            cstate_data->state < cstate_state_being_integrated
        :
            cstate_data->state != cstate_state_completed
        )
        {
            warning =
                str_format
                (
                    "Warning: the original change was in the '%s' state\n",
                    cstate_state_ename(cstate_data->state)
                );
        }
        if (cstate_data->state > cstate_state_being_developed)
        {
            cstate_history_list_ty *hlp;

            hlp = cstate_data->history;
            assert(hlp);
            assert(hlp->length > 0);
            assert(hlp->list);
            if (hlp && hlp->length > 0 && hlp->list)
            {
                cstate_history_ty *hp;

                hp = hlp->list[hlp->length - 1];
                assert(hp);
                when = hp->when;
            }
        }
        change_set->description =
            str_format
            (
                "From: %s\nDate: %.24s\n%s\n%s",
                up->get_email_address().c_str(),
                ctime(&when),
                (warning ? warning->str_text : ""),
                cstate_data->description->str_text
            );
        if (warning)
            str_free(warning);
    }
    else if (entire_source)
        change_set->description = project_description_get(pp).get_ref_copy();
    else
        change_set->description = str_copy(cstate_data->description);
    change_set->cause = cstate_data->cause;
    change_set->test_exempt = cstate_data->test_exempt;
    change_set->test_baseline_exempt = cstate_data->test_baseline_exempt;
    change_set->regression_test_exempt = cstate_data->regression_test_exempt;
    if (use_attributes)
    {
        change_set->attribute =
            (
                cstate_data->attribute
            ?
                attributes_list_copy(cstate_data->attribute)
            :
                (attributes_list_ty *)attributes_list_type.alloc()
            );
        //
        // Do not propagate attributes that may be wrong on the other
        // side.
        //
        attributes_list_remove(change_set->attribute, HISTORY_GET_COMMAND);

        if (use_uuid)
        {
            change_functor_attribute_list result(false, change_set->attribute);
            if (cp->was_a_branch())
            {
                //
                // For branches, add all of the constituent change sets'
                // UUIDs.  That way, if you resynch by grabbing a whole
                // branch as one change set, you still grab all of the
                // constituent change set UUIDs.
                //
                project_inventory_walk(pp, result);
            }
            if (entire_source)
            {
                //
                // If they said --entire-source, add all of the accumulated
                // change set UUIDs.  That way, if you resynch by grabbing
                // a whole project as one change set, you still grab all of
                // the constituent change set UUIDs.
                //
                time_t limit = cp->completion_timestamp();
                project_inventory_walk(pp, result, limit);
            }
        }
        else
        {
            for (;;)
            {
                attributes_ty *ap =
                    attributes_list_extract
                    (
                        change_set->attribute,
                        ORIGINAL_UUID
                    );
                if (!ap)
                    break;
                attributes_type.free(ap);
            }
        }
        if (change_set->attribute->length == 0)
        {
            attributes_list_type.free(change_set->attribute);
            change_set->attribute = 0;
        }
    }
    if (use_attributes && use_uuid && cstate_data->uuid)
        change_set->uuid = str_copy(cstate_data->uuid);
    // FIXME: architecture
    // FIXME: copyright years

    //
    // Scan for files to be added to the output.
    //
    bool aeget_inventory_hide =
        change_attributes_find_boolean(cp, "aeget:inventory:hide");
    for (j = 0;; ++j)
    {
        fstate_src_ty *src_data = change_file_nth(cp, j, view_path_first);
        if (!src_data)
            break;

        //
        // we only omit local source files from change sets which are
        // not composed solely of omitted files.  This is so that it is
        // possible to send a change set composed only of local sources
        // to another site (just not via the inventory listings).
        //
        if
        (
            !aeget_inventory_hide
        &&
            attributes_list_find_boolean
            (
                src_data->attribute,
                "local-source-hide"
            )
        )
            continue;

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
                    string_ty *s = cp->file_path(src_data->file_name);
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
        if (!use_config && src_data->usage == file_usage_config)
            src_data->usage = file_usage_source;
        one_more_src(historian, change_set, src_data, use_attributes);
    }
    if (entire_source)
    {
        if (historian.is_set())
        {
            nstring_list file_name_list;
            historian.keys(file_name_list);
            for (j = 0; j < file_name_list.size(); ++j)
            {
                nstring file_name = file_name_list[j];
                assert(file_name.length());
                file_event *fep = historian.get_last(file_name);
                assert(fep);
                if (!fep)
                    continue;
                assert(fep->get_src());
                if
                (
                    attributes_list_find_boolean
                    (
                        fep->get_src()->attribute,
                        "entire-source-hide"
                    )
                )
                {
                    continue;
                }
                if
                (
                    attributes_list_find_boolean
                    (
                        fep->get_src()->attribute,
                        "local-source-hide"
                    )
                )
                {
                    continue;
                }
                switch (fep->get_src()->usage)
                {
                case file_usage_build:
                    switch (fep->get_src()->action)
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
                    switch (fep->get_src()->action)
                    {
                    case file_action_create:
                    case file_action_modify:
                    case file_action_remove:
                        break;

                    case file_action_insulate:
                    case file_action_transparent:
                        // can't happen
                        assert(0);
                        continue;
                    }
                    break;
                }
                if (!have_it_already(change_set, fep->get_src()))
                {
                    if
                    (
                        !use_config
                    &&
                        fep->get_src()->usage == file_usage_config
                    )
                    {
                        fep->get_src()->usage = file_usage_source;
                    }
                    one_more_src
                    (
                        historian,
                        change_set,
                        fep->get_src(),
                        use_attributes
                    );
                }
            }
        }
        else
        {
            trace(("adding project files...\n"));
            for (j = 0;; ++j)
            {
                fstate_src_ty   *src_data;

                src_data = pp->file_nth(j, view_path_simple);
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
                if
                (
                    attributes_list_find_boolean
                    (
                        src_data->attribute,
                        "local-source-hide"
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
                    case file_action_remove:
                        break;

                    case file_action_insulate:
                    case file_action_transparent:
                        // can't happen
                        assert(0);
                        continue;
                    }
                    break;
                }
                if (!have_it_already(change_set, src_data))
                {
                    if (!use_config && src_data->usage == file_usage_config)
                        src_data->usage = file_usage_source;
                    one_more_src
                    (
                        historian,
                        change_set,
                        src_data,
                        use_attributes
                    );
                }
            }
        }
    }
    if (!change_set->src || !change_set->src->length)
        change_fatal(cp, 0, i18n("bad send no files"));

    //
    // Adjust the content of the change set to make it contains only
    // operations that can be performed in a 'change'.
    //
    if (entire_source)
    {
        //
        // We collapse multiple rename chains into just one rename
        // operation:
        //
        // A renamed to B renamed to C
        //
        // will become
        //
        // A renamed to C
        //
        trace(("%ld\n",(long)change_set->src->length));
        for (size_t i = 0; i < change_set->src->length; ++i)
        {
            if (!change_set->src->list[i])
                continue;

            cstate_src_ty *csrc = change_set->src->list[i];
            if (!csrc->move)
                continue;

            trace(("%s \"%s\" move \"%s\"\n", file_action_ename(csrc->action),
                csrc->file_name->str_text, csrc->move->str_text));
            cstate_src_ty *csrc2 = 0;
            for (size_t k = i + 1; k < change_set->src->length; ++k)
            {
                csrc2 = change_set->src->list[k];

                if (!csrc2)
                    continue;

                if (!csrc2->move)
                    continue;

                //
                // We have found a regular rename.
                //
                if
                (
                    str_equal(csrc->move, csrc2->file_name)
                &&
                    str_equal(csrc2->move, csrc->file_name)
                )
                {
                    trace(("normal move\n"));
                    goto loop_end;
                }

                //
                // We have found two unrelated files.
                //
                if
                (
                    !str_equal(csrc->move, csrc2->file_name)
                &&
                    !str_equal(csrc2->move, csrc->file_name)
                )
                    continue;

                //
                // If this is not a simple rename, things get
                // complicated, because the links (via the "move" field)
                // only make sense when you are doing a history roll
                // forward.  That is, if a file is moved twice,
                // you would need *two* move fields to record the
                // information, and there is only one.
                //
                // So we turn the move into a simple create or remove
                // action.  Some work will be needed to avoid trying to
                // both rename and remove a file.
                //
                if (csrc->uuid && csrc->action == file_action_remove)
                {
                    //
                    // The file has been renamed more than once.  We
                    // delete dangling removes because we know there
                    // is a complete rename pair elsewhere in the
                    // list.  Any such rename will be done correctly by
                    // consulting the UUID field of the files.
                    //
                    trace(("toss \"%s\" %s\n", csrc->file_name->str_text,
                        file_action_ename(csrc->action)));
                    cstate_src_type.free(csrc);
                    change_set->src->list[i] = 0;
                }
                else
                {
                    trace(("drop remove attribute\n"));
                    str_free(csrc->move);
                    csrc->move = 0;
                }
            }
            loop_end:;
        }

        //
        // Now the change_set may contain holes (0), we need to remove
        // them before further operations.  We are not forced to keep
        // the order because of the next qsort invocation.
        //
        for (size_t k = 0; k < change_set->src->length; ++k)
        {
            trace(("k = %ld of %ld\n", (long)k, (long)change_set->src->length));
            if (change_set->src->list[k])
                continue;

            while (!change_set->src->list[--change_set->src->length])
                ;
            change_set->src->list[k] =
                change_set->src->list[change_set->src->length];
        }

#ifdef DEBUG
        for (size_t k = 0; k < change_set->src->length; ++k)
        {
            assert(change_set->src->list[k]);
        }
#endif
    }

    //
    // sort the files by name
    //
    qsort
    (
        change_set->src->list,
        change_set->src->length,
        sizeof(change_set->src->list[0]),
        cmp
    );

    os_become_orig();
    childs_name = "etc/change-set";
    ofp = cpio_p->child(childs_name, -1);
    ofp = output_filter_indent::create(ofp);
    cstate_write(ofp, change_set);
    ofp.reset();
    os_become_undo();

    //
    // We need a whole bunch of temporary files.
    //
    diff_output_filename = os_edit_filename(0);
    os_become_orig();
    undo_unlink_errok(diff_output_filename);
    os_become_undo();
    dev_null = str_from_c("/dev/null");

    //
    // add each of the relevant source files to the archive
    //
    for (j = 0; j < change_set->src->length; ++j)
    {
        cstate_src_ty   *csrc;
        long            len;
        string_ty       *original_filename = 0;
        string_ty       *input_filename = 0;
        int             original_filename_unlink = 0;
        int             input_filename_unlink = 0;

        csrc = change_set->src->list[j];
        switch (csrc->usage)
        {
        case file_usage_build:
            continue;

        case file_usage_source:
        case file_usage_config:
        case file_usage_test:
        case file_usage_manual_test:
            break;
        }
        switch (csrc->action)
        {
        case file_action_remove:
            continue;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
        case file_action_transparent:
            break;
        }
        trace(("file name = \"%s\", action = %s, usage = %s\n",
            csrc->file_name->str_text, file_action_ename(csrc->action),
            file_usage_ename(csrc->usage)));
        if (csrc->move)
        {
            trace(("move = \"%s\"\n", csrc->move->str_text));
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
        // diff_output_filename
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
            //
            // Get the orginal file.
            //
            trace(("%s %s \"%s\"\n", file_usage_ename(csrc->usage),
                file_action_ename(csrc->action), csrc->file_name->str_text));
            switch (csrc->action)
            {
            case file_action_create:
                //
                // We include the need for a UUID because otherwise the
                // chain of renames can be an arbitrarily long one (and
                // could possibly loop).  If we only deal with UUIDs, we
                // simplify the problem immensely.
                //
                if (use_rename_patch && csrc->move && csrc->uuid)
                {
                    if (historian.is_set())
                    {
                        trace(("using historian, by uuid\n"));
                        file_event_list::pointer orig_felp =
                            historian.get(csrc);

                        //
                        // It's tempting to say
                        //     assert(felp);
                        // but file file may not yet exist at this point in
                        // time, so there is no need (or ability) to create a
                        // patch for it.
                        //
                        if (!orig_felp || orig_felp->size() < 2)
                        {
                            original_filename = str_copy(dev_null);
                            trace(("original filename /dev/null\n"));
                            break;
                        }

                        file_event *orig_fep = orig_felp->back();
                        fstate_src_ty *orig_src =
                            orig_fep->get_change()->file_find
                            (
                                nstring(csrc->move),
                                view_path_first
                            );
                        assert(orig_src);
                        original_filename =
                            project_file_version_path
                            (
                                pp,
                                orig_src,
                                &original_filename_unlink
                            );
                    }
                    else
                    {
                        fstate_src_ty *orig_src =
                            pp->file_find(csrc, view_path_extreme);
                        if (!orig_src)
                        {
                            original_filename = str_copy(dev_null);
                            trace(("original filename /dev/null\n"));
                            break;
                        }
                        original_filename =
                            project_file_version_path
                            (
                                pp,
                                orig_src,
                                &original_filename_unlink
                            );
                    }
                }
                else
                {
                    original_filename = str_copy(dev_null);
                    trace(("original filename /dev/null\n"));
                }
                break;

            case file_action_modify:
            case file_action_remove:
            case file_action_insulate:
            case file_action_transparent:
#ifndef DEBUG
            default:
#endif
                original_filename = project_file_path(pp, csrc->file_name);
                break;
            }
            assert(original_filename);

            //
            // Get the input file.
            //
            switch (csrc->action)
            {
            case file_action_remove:
                input_filename = str_copy(dev_null);
                trace(("input filename /dev/null\n"));
                break;

            case file_action_transparent:
                // FIXME: this is wrong, need version from grandparent
                // fall through...

            case file_action_create:
            case file_action_modify:
            case file_action_insulate:
#ifndef DEBUG
            default:
#endif
                input_filename = cp->file_path(csrc->file_name);
                if (!input_filename)
                    input_filename = project_file_path(pp, csrc->file_name);
                break;
            }
            assert(input_filename);
            break;

        case cstate_state_completed:
            //
            // Both the versions to be diffed come out
            // of history.
            //
            trace(("%s %s \"%s\"\n", file_usage_ename(csrc->usage),
                file_action_ename(csrc->action), csrc->file_name->str_text));
            switch (csrc->action)
            {
                // file_event_list::pointer felp;
                // file_event *fep;
                // fstate_src_ty *old_src;

            case file_action_create:
                {
                    assert(historian.is_set());
                    file_event_list::pointer felp = historian.get(csrc);

                    //
                    // It's tempting to say
                    //    assert(felp);
                    // but file file may not yet exist at this point in
                    // time, so there is no need (or ability) to create a
                    // patch for it.
                    //
                    if (!felp)
                    {
                        original_filename = str_copy(dev_null);
                        trace(("original filename /dev/null\n"));
                        input_filename = str_copy(dev_null);
                        trace(("input filename /dev/null\n"));
                        break;
                    }

                    assert(!felp->empty());

                    //
                    // Get the orginal file.  We handle the creation half
                    // of a file rename.
                    //
                    // We include the need for a uuid because otherwise the
                    // rename chain can be an arbitrarily long one (and
                    // could possibly loop).  If we only deal with UUIDs, we
                    // simplify the problem immensely.
                    //
                    if
                    (
                        use_rename_patch
                    &&
                        csrc->move
                    &&
                        csrc->uuid
                    &&
                        felp->size() >= 2
                    )
                    {
                        //
                        // Do the same as modify.
                        //
                        file_event *fep = felp->get(felp->size() - 2);
                        fstate_src_ty *old_src =
                            fep->get_change()->file_find
                            (
                                csrc,
                                view_path_first
                            );
                        assert(old_src);
                        original_filename =
                            project_file_version_path
                            (
                                pp,
                                old_src,
                                &original_filename_unlink
                            );
                    }
                    else
                        original_filename = str_copy(dev_null);

                    //
                    // Get the input file.
                    //
                    file_event *fep = felp->back();
                    fstate_src_ty *old_src =
                        fep->get_change()->file_find
                        (
                            csrc,
                            view_path_first
                        );
                    assert(old_src);
                    input_filename =
                        project_file_version_path
                        (
                            pp,
                            old_src,
                            &input_filename_unlink
                        );
                    assert(original_filename);
                    assert(input_filename);
                }
                break;

            case file_action_remove:
                {
                    //
                    // We ignore the remove half or a file rename.
                    //
                    if (use_rename_patch && csrc->move)
                    {
                        input_filename = str_copy(dev_null);
                        trace(("original filename /dev/null\n"));
                        original_filename = str_copy(dev_null);
                        trace(("input filename /dev/null\n"));
                        break;
                    }

                    assert(historian.is_set());
                    file_event_list::pointer felp = historian.get(csrc);

                    //
                    // It's tempting to say
                    //  assert(felp);
                    // but file file may not yet exist at this point in
                    // time, so there is no need (or ability) to create a
                    // patch for it.
                    //
                    // It is also tempting to say
                    //  assert(felp->length >= 2);
                    // except that a file which is created and removed in
                    // the same branch, will result in only a remove record
                    // in its parent branch when integrated.
                    //
                    assert(!felp || !felp->empty());
                    if (!felp || felp->size() < 2)
                    {
                        original_filename = str_copy(dev_null);
                    }
                    else
                    {
                        //
                        // Get the orginal file.
                        //
                        file_event *fep = felp->get(felp->size() - 2);
                        fstate_src_ty *old_src =
                            fep->get_change()->file_find
                            (
                                csrc,
                                view_path_first
                            );
                        assert(old_src);
                        original_filename =
                            project_file_version_path
                            (
                                pp,
                                old_src,
                                &original_filename_unlink
                            );
                    }

                    //
                    // Get the input file.
                    //
                    input_filename = str_copy(dev_null);
                    trace(("input filename /dev/null\n"));
                }
                break;

            case file_action_modify:
                {
                    assert(historian.is_set());
                    file_event_list::pointer felp = historian.get(csrc);

                    //
                    // It's tempting to say
                    //  assert(felp);
                    // but file file may not yet exist at this point in
                    // time, so there is no need (or ability) to create a
                    // patch for it.
                    //
                    assert(!felp || !felp->empty());
                    if (!felp)
                    {
                        original_filename = str_copy(dev_null);
                        trace(("original filename /dev/null\n"));
                        input_filename = str_copy(dev_null);
                        trace(("input filename /dev/null\n"));
                        break;
                    }

                    //
                    // Get the orginal file.
                    //
                    if (felp->size() < 2)
                    {
                        original_filename = str_copy(dev_null);
                    }
                    else
                    {
                        file_event *fep = felp->get(felp->size() - 2);
                        fstate_src_ty *old_src =
                            fep->get_change()->file_find
                            (
                                csrc,
                                view_path_first
                            );
                        assert(old_src);
                        original_filename =
                            project_file_version_path
                            (
                                pp,
                                old_src,
                                &original_filename_unlink
                            );
                    }

                    //
                    // Get the input file.
                    //
                    file_event *fep = felp->back();
                    fstate_src_ty *old_src =
                        fep->get_change()->file_find
                        (
                            csrc,
                            view_path_first
                        );
                    assert(old_src);
                    input_filename =
                        project_file_version_path
                        (
                            pp,
                            old_src,
                            &input_filename_unlink
                        );
                }
                break;

            case file_action_insulate:
                // this is supposed to be impossible
                trace(("insulate = \"%s\"\n", csrc->file_name->str_text));
                assert(0);
                original_filename = str_copy(dev_null);
                trace(("original filename /dev/null\n"));
                input_filename = str_copy(dev_null);
                trace(("input filename /dev/null\n"));
                break;

            case file_action_transparent:
                // no file content appears in the output
                trace(("transparent = \"%s\"\n", csrc->file_name->str_text));
                original_filename = str_copy(dev_null);
                trace(("original filename /dev/null\n"));
                input_filename = str_copy(dev_null);
                trace(("input filename /dev/null\n"));
                break;
            }
            assert(original_filename);
            assert(input_filename);
            break;
        }

        //
        // If they are both /dev/null don't bother with a patch.
        //
        assert(original_filename);
        assert(input_filename);
        if
        (
            !str_equal(original_filename, dev_null)
        ||
            !str_equal(input_filename, dev_null)
        )
        {
            //
            // Put a patch into the archive
            // for modified files.
            //
            // We don't bother with a patch for created files, because
            // we simply include the whole source in the next section.
            //
            bool is_a_rename = false;
            switch (csrc->action)
            {
            case file_action_remove:
            case file_action_transparent:
                break;

            case file_action_create:
                if (!use_rename_patch || !csrc->move)
                    break;
                is_a_rename = true;
                // fall through

            case file_action_modify:
            case file_action_insulate:
                if (entire_source)
                    break;
                if (!use_patch)
                    break;

                //
                // Generate the difference file only if the
                // original_filename is *not* /dev/null.
                //
                // If the original_filename is /dev/null then the
                // patch will be a full insert, causing a problem
                // duplicating almost completly the content of the
                // patched file.
                //
                assert(original_filename);
                trace(("original_filename = \"%s\"\n",
                    original_filename->str_text));
                assert(input_filename);
                trace(("input_filename = \"%s\"\n",
                    input_filename->str_text));
                assert(diff_output_filename);
                trace(("diff_output_filename = \"%s\"\n",
                    diff_output_filename->str_text));
                if (str_equal(original_filename, dev_null))
                    break;
                change_run_patch_diff_command
                (
                    cp,
                    up,
                    original_filename,
                    input_filename,
                    diff_output_filename,
                    csrc->file_name
                );

                //
                // Read the diff into the archive.
                //
                os_become_orig();
                input ifp = input_file_open(diff_output_filename, true);
                assert(ifp.is_open());
                len = ifp->length();
                assert(len >= 0);
                if (len > 0 || is_a_rename)
                {
                    childs_name =
                        nstring::format("patch/%s", csrc->file_name->str_text);
                    output::pointer op = cpio_p->child(childs_name, len);
                    op << ifp;
                }
                ifp.close();
                os_become_undo();

                // It's tempting to say:
                //
                // str_free(diff_output_filename);
                //
                // but this must really be freed once: out of the loop.
                break;
            }
        }

        //
        // Put the whole file into the archive,
        // for creates and modifies.
        //
        // Even though a patch is preferable, sometimes the
        // destination has never heard of the file, so we include
        // the whole file just in case.
        //
        switch (csrc->action)
        {
        case file_action_remove:
        case file_action_transparent:
            break;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
            os_become_orig();
            input ifp = input_file_open(input_filename);
            assert(ifp.is_open());
            len = ifp->length();
            childs_name =
                nstring::format("src/%s", csrc->file_name->str_text);
            ofp = cpio_p->child(childs_name, len);
            ofp << ifp;
            ifp.close();
            ofp.reset();
            os_become_undo();
            break;
        }

        //
        // Free a bunch of strings.
        //
        os_become_orig();
        if (original_filename_unlink)
            os_unlink_errok(original_filename);
        if (input_filename_unlink)
            os_unlink_errok(input_filename);
        os_become_undo();
        str_free(original_filename);
        str_free(input_filename);
    }
    cstate_type.free(change_set);

    //
    // Get rid of all the temporary files.
    //
    os_become_orig();
    assert(diff_output_filename);
    os_unlink_errok(diff_output_filename);
    os_become_undo();
    str_free(diff_output_filename);
    str_free(dev_null);

    // finish writing the cpio archive
    os_become_orig();
    delete cpio_p;
    os_become_undo();

    // clean up and go home
    change_free(cp);
    project_free(pp);
}


// vim: set ts=8 sw=4 et :
