//
// aegis - project change supervisor
// Copyright (C) 2001-2009, 2011, 2012 Peter Miller
// Copyright (C) 2008, 2009 Walter Franzini
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
#include <common/ac/unistd.h>

#include <common/nstring.h>
#include <common/progname.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/cattr.fmtgen.h>
#include <libaegis/change/attributes.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/change/lock_sync.h>
#include <libaegis/help.h>
#include <libaegis/input/base64.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/input/string.h>
#include <libaegis/meta_parse.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/patch.h>
#include <libaegis/pconf.fmtgen.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/trojan.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>

#include <aepatch/arglex3.h>
#include <aepatch/receive.h>
#include <aepatch/slurp.h>


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s --receive [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


static string_ty *
is_a_path_prefix(string_ty *haystack, string_ty *needle)
{
    if
    (
        haystack->str_length > needle->str_length
    &&
        haystack->str_text[needle->str_length] == '/'
    &&
        0 == memcmp(haystack->str_text, needle->str_text, needle->str_length)
    )
    {
        return
            str_n_from_c
            (
                haystack->str_text + needle->str_length + 1,
                haystack->str_length - needle->str_length - 1
            );
    }
    return 0;
}


static string_ty *path_prefix_add;
static string_list_ty path_prefix_remove;


static void
mangle_file_names(patch_list_ty *plp, project *pp)
{
    struct cmp_t
    {
        int             npaths;
        int             idx;
    };

    size_t          j;
    cmp_t           best;
    patch_ty        *p;
    string_ty       *s;
    int             npaths;
    size_t          idx;
    char            *cp;
    string_ty       *dev_null;

    //
    // First we chew over the filenames according to the path prefix
    // options on the command line.
    //
    // First remove any path prefixes we have been asked to remove.
    // Second add a path prefix if we have been asked to.
    //
    trace(("mangle_file_names()\n{\n"));
    for (j = 0; j < plp->length; ++j)
    {
        p = plp->item[j];
        for (idx = 0; idx < p->name.nstrings; ++idx)
        {
            size_t          k;

            for (k = 0; k < path_prefix_remove.nstrings; ++k)
            {
                s =
                    is_a_path_prefix
                    (
                        p->name.string[idx],
                        path_prefix_remove.string[k]
                    );
                if (s)
                {
                    trace(("\"%s\" -> \"%s\"\n", p->name.string[idx]->str_text,
                        s->str_text));
                    str_free(p->name.string[idx]);
                    p->name.string[idx] = s;
                }
            }
            if (path_prefix_add)
            {
                s = os_path_cat(path_prefix_add, p->name.string[idx]);
                trace(("\"%s\" -> \"%s\"\n", p->name.string[idx]->str_text,
                    s->str_text));
                str_free(p->name.string[idx]);
                p->name.string[idx] = s;
            }
        }
    }

    //
    // Look for the name with the fewest removed leading path
    // compenents that produces a file name which exists in the
    // project.
    //
    dev_null = str_from_c("/dev/null");
    best.npaths = 32767;
    best.idx = 32767;
    for (j = 0; j < plp->length; ++j)
    {
        p = plp->item[j];
        for (idx = 0; idx < p->name.nstrings; ++idx)
        {
            npaths = 0;
            if (str_equal(dev_null, p->name.string[idx]))
                continue;
            cp = p->name.string[idx]->str_text;
            for (;;)
            {
                s = str_from_c(cp);
                if (pp->file_find(s, view_path_extreme))
                {
                    if
                    (
                        npaths < best.npaths
                    ||
                        (npaths == best.npaths && (int)idx < best.idx)
                    )
                    {
                        best.npaths = npaths;
                        best.idx = idx;
                        break;
                    }
                }
                cp = strchr(cp, '/');
                if (!cp)
                    break;
                ++cp;
                if (!*cp)
                    break;
                ++npaths;
            }
        }
    }
    str_free(dev_null);
    if (best.npaths == 32767)
    {
        best.npaths = 0;
        best.idx = 0;
    }
    trace(("best.npaths = %d\n", best.npaths));
    trace(("best.idx = %d\n", best.idx));

    //
    // Now adjust the file names, using the path information.
    //
    for (j = 0; j < plp->length; ++j)
    {
        //
        // Rip the right number of path elements from the
        // "best" name.  Note that we have to cope with the
        // number of names in the patch being inconsistent.
        //
        trace(("%zd of %zd\n", j, plp->length));
        p = plp->item[j];
        trace(("nstrings = %d\n", (int)p->name.nstrings));
        assert(p->name.nstrings);
        if (p->name.nstrings == 0)
            continue;
        s = p->name.string[0];
        if (best.idx < (int)p->name.nstrings)
            s = p->name.string[best.idx];
        trace(("\"%s\"\n", s->str_text));
        cp = s->str_text;
        for (npaths = best.npaths; npaths > 0; --npaths)
        {
            char *cp2 = strchr(cp, '/');
            if (!cp2)
                break;
            cp = cp2 + 1;
        }
        trace(("%s -> %s\n", p->name.string[0]->str_text, cp));

        //
        // Insert the reconstructed name into the front of the
        // list of names.
        //
        s = str_from_c(cp);
        p->name.push_front(s);
        str_free(s);
    }
    trace(("}\n"));
}


static long
number_of_files(string_ty *project_name, long change_number)
{
    project      *pp;
    change::pointer cp;
    long            result;

    pp = project_alloc(project_name);
    pp->bind_existing();
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    result = change_file_count(cp);
    change_free(cp);
    project_free(pp);
    return result;
}


static cstate_ty *
extract_meta_data(string_ty **spp)
{
    cstate_ty       *change_set;
    const char      *begin;
    const char      *end;
    string_ty       *s;

    //
    // See if the necessary text markers are present.
    //
    begin = strstr((*spp)->str_text, "Aegis-Change-Set-Begin");
    if (!begin)
        return 0;
    while (*begin)
    {
        ++begin;
        if (begin[-1] == '\n')
            break;
    }
    end = strstr(begin, "Aegis-Change-Set-End");
    if (!end)
        return 0;
    while (end > begin)
    {
        --end;
        if (*end == '\n')
            break;
    }

    //
    // Build an input source out of the marked text.
    //
    input ip = new input_string(nstring(begin, end - begin));

    //
    // Crop the description to emit the meta-data
    // (and everything following it).
    //
    end = begin;
    while (end > (*spp)->str_text && end[-1] != '\n')
        --end;
    s = str_n_from_c((*spp)->str_text, end - (*spp)->str_text);
    str_free(*spp);
    *spp = s;

    //
    // Decipher the meta data.
    //
    ip = new input_base64(ip);
    ip = input_gunzip_open(ip);
    ip = input_bunzip2_open(ip);
    change_set = (cstate_ty *)parse_input(ip, &cstate_type);
    ip.close();
    return change_set;
}


static cstate_src_ty *
cstate_src_find(cstate_ty *cstate_data, string_ty *file_name)
{
    size_t          j;

    if (!cstate_data)
        return 0;
    if (!cstate_data->src)
        return 0;
    for (j = 0; j < cstate_data->src->length; ++j)
    {
        cstate_src_ty   *src;

        src = cstate_data->src->list[j];
        if (src && src->file_name && str_equal(src->file_name, file_name))
            return src;
    }
    return 0;
}


void
receive(void)
{
    string_ty       *ifn;
    string_ty       *s;
    patch_list_ty   *plp;
    size_t          j;
    string_ty       *project_name;
    long            change_number;
    project      *pp;
    change::pointer cp;
    string_ty       *attribute_file_name;
    cattr_ty        *cattr_data;
    cattr_ty        *dflt;
    string_ty       *dot;
    string_ty       *devdir;
    pconf_ty        *pconf_data;
    string_ty       *dd;
    const char      *delta;
    int             config_seen;
    int             trojan;
    cstate_ty       *change_set;

    trace(("receive()\n{\n"));
    project_name = 0;
    change_number = 0;
    ifn = 0;
    devdir = 0;
    delta = 0;
    trojan = -1;
    nstring output_filename;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_change:
            arglex();
            arglex_parse_change(&project_name, &change_number, usage);
            continue;

        case arglex_token_project:
            arglex();
            arglex_parse_project(&project_name, usage);
            continue;

        case arglex_token_file:
            if (ifn)
                duplicate_option(usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_file, usage);
                // NOTREACHED

            case arglex_token_stdio:
                ifn = str_from_c("");
                break;

            case arglex_token_string:
                ifn = str_from_c(arglex_value.alv_string);
                break;
            }
            break;

        case arglex_token_directory:
            if (devdir)
            {
                duplicate_option(usage);
                // NOTREACHED
            }
            if (arglex() != arglex_token_string)
            {
                option_needs_dir(arglex_token_directory, usage);
                // NOTREACHED
            }
            devdir = str_format(" --directory %s", arglex_value.alv_string);
            break;

        case arglex_token_trojan:
            if (trojan > 0)
                duplicate_option(usage);
            if (trojan >= 0)
            {
                too_many_trojans:
                mutually_exclusive_options
                (
                    arglex_token_trojan,
                    arglex_token_trojan_not,
                    usage
                );
            }
            trojan = 1;
            break;

        case arglex_token_trojan_not:
            if (trojan == 0)
                duplicate_option(usage);
            if (trojan >= 0)
                goto too_many_trojans;
            trojan = 0;
            break;

        case arglex_token_delta:
            if (delta)
                duplicate_option(usage);
            switch (arglex())
            {
            default:
                option_needs_number(arglex_token_delta, usage);
                // NOTREACHED

            case arglex_token_number:
            case arglex_token_string:
                delta = arglex_value.alv_string;
                break;
            }
            break;

        case arglex_token_path_prefix_add:
            if (path_prefix_add)
                duplicate_option(usage);
            if (arglex() != arglex_token_string)
                option_needs_file(arglex_token_path_prefix_add, usage);
            path_prefix_add = str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_path_prefix_remove:
            if (arglex() != arglex_token_string)
                option_needs_file(arglex_token_path_prefix_remove, usage);
            s = str_from_c(arglex_value.alv_string);
            path_prefix_remove.push_back_unique(s);
            str_free(s);
            break;

        case arglex_token_output:
            if (!output_filename.empty())
                duplicate_option(usage);
            if (arglex() != arglex_token_string)
            {
                option_needs_file(arglex_token_output, usage);
                // NOTREACHED
            }
            output_filename = nstring(arglex_value.alv_string);
            break;
        }
        arglex();
    }

    if (change_number && !output_filename.empty())
    {
        mutually_exclusive_options
        (
            arglex_token_change,
            arglex_token_output,
            usage
        );
    }

    //
    // read the input
    //
    plp = patch_slurp(ifn);
    assert(plp);

    if (!project_name)
        project_name = plp->project_name;
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
        project_name = str_copy(n.get_ref());
    }

    //
    // locate project data
    //      (Even of we don't use it, this confirms it is a valid
    //      project name.)
    //
    pp = project_alloc(project_name);
    pp->bind_existing();

    //
    // See if the initial prelude contains the project meta-data.
    //
    change_set = extract_meta_data(&plp->description);

    //
    // Search the names in the patch, trying to figure out how much
    // path prefix to throw away.  When this is done, name.string[0]
    // is the name we will use for the files.
    //
    bool build_files_are_ok = true;
    if (!change_set)
    {
        mangle_file_names(plp, pp);
        build_files_are_ok = true;
    }

    //
    // default the change number
    //
    // Note that the change number in the patch is advisory only, if we
    // can't get it, just use the next available.
    //
    if (!change_number)
    {
        if
        (
            plp->change_number
        &&
            !project_change_number_in_use(pp, plp->change_number)
        )
            change_number = plp->change_number;
        else
            change_number = project_next_change_number(pp, 1);
    }

    //
    // If the user asked for it, write the change number in the output
    // file.
    //
    if (!output_filename.empty())
    {
        os_become_orig();
        output::pointer ofp(output_file::open(output_filename));
        os_become_undo();

        ofp->fprintf("%ld\n", change_number);
    }

    //
    // construct change attributes from the patch
    //
    os_become_orig();
    attribute_file_name = os_edit_filename(0);
    undo_unlink_errok(attribute_file_name);
    cattr_data = (cattr_ty *)cattr_type.alloc();
    if (change_set && change_set->brief_description)
        cattr_data->brief_description = str_copy(change_set->brief_description);
    else if (plp->brief_description)
        cattr_data->brief_description = str_copy(plp->brief_description);
    else
        cattr_data->brief_description = str_from_c("none");
    if (change_set && change_set->description)
            cattr_data->description = str_copy(change_set->description);
    else if (plp->description)
        cattr_data->description = str_copy(plp->description);
    else
        cattr_data->description = str_from_c("none");
    if (change_set && (change_set->mask & cstate_cause_mask))
        cattr_data->cause = change_set->cause;
    else
        cattr_data->cause = change_cause_external_bug;
    if (change_set && change_set->attribute)
        cattr_data->attribute = attributes_list_copy(change_set->attribute);

    dflt = (cattr_ty *)cattr_type.alloc();
    dflt->cause = cattr_data->cause;
    os_become_undo();
    pconf_data = project_pconf_get(pp);
    change_attributes_default(dflt, pp, pconf_data);
    os_become_orig();

    if (change_set && (change_set->mask & cstate_test_exempt_mask))
        cattr_data->test_exempt = change_set->test_exempt;
    else
        cattr_data->test_exempt = dflt->test_exempt;
    if (change_set && (change_set->mask & cstate_test_baseline_exempt_mask))
        cattr_data->test_baseline_exempt = change_set->test_baseline_exempt;
    else
        cattr_data->test_baseline_exempt = dflt->test_baseline_exempt;
    if (change_set && (change_set->mask & cstate_regression_test_exempt_mask))
        cattr_data->regression_test_exempt = change_set->regression_test_exempt;
    else
        cattr_data->regression_test_exempt = dflt->regression_test_exempt;
    cattr_type.free(dflt);
    cattr_write_file(attribute_file_name, cattr_data, 0);
    cattr_type.free(cattr_data);
    project_free(pp);
    pp = 0;

    nstring reason;
    if (plp->comment)
    {
        reason =
            (
                " --reason="
            +
                ("Downloaded from " + nstring(plp->comment)).quote_shell()
            );
    }

    //
    // create the new change
    //
    nstring trace_options(trace_args());
    dot = os_curdir();
    s =
        str_format
        (
            "aegis --new-change %ld --project=%s --file=%s --verbose%s%s",
            change_number,
            project_name->str_text,
            attribute_file_name->str_text,
            reason.c_str(),
            trace_options.c_str()
        );
    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
    str_free(s);
    os_unlink_errok(attribute_file_name);
    str_free(attribute_file_name);

    //
    // Begin development of the new change.
    //
    s =
        str_format
        (
            "aegis --develop-begin %ld --project %s --verbose%s%s",
            change_number,
            project_name->str_text,
            (devdir ? devdir->str_text : ""),
            trace_options.c_str()
        );
    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
    str_free(s);
    os_become_undo();

    //
    // Change to the development directory, so that we can use
    // relative filenames.  It makes things easier to read.
    //
    pp = project_alloc(project_name);
    pp->bind_existing();
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    dd = change_development_directory_get(cp, 0);
    dd = str_copy(dd); // will vanish when change_free();

    os_chdir(dd);

    //
    // Adjust the file actions to reflect the current state of
    // the project.
    //
    bool need_to_test = false;
    bool could_have_a_trojan = false;
    for (j = 0; j < plp->length; ++j)
    {
        patch_ty        *p;
        fstate_src_ty   *p_src_data;
        string_ty       *file_name;

        p = plp->item[j];
        assert(p->name.nstrings >= 1);
        file_name = p->name.string[0];
        p_src_data = pp->file_find(file_name, view_path_extreme);
        if (!p_src_data)
        {
            switch (p->action)
            {
            case file_action_remove:
                //
                // Removing a removed file would be an
                // error.  Get rid of it.
                //
                plp->item[j] = plp->item[plp->length - 1];
                plp->length--;
                --j;
                patch_delete(p);
                continue;

            case file_action_insulate:
            case file_action_transparent:
                assert(0);

            case file_action_create:
                break;

            case file_action_modify:
#ifndef DEBUG
            default:
#endif
                p->action = file_action_create;
                break;
            }
        }
        else
        {
            switch (p->action)
            {
            case file_action_remove:
                break;

            case file_action_modify:
                break;

            case file_action_create:
            case file_action_insulate:
            case file_action_transparent:
#ifndef DEBUG
            default:
#endif
                p->action = file_action_modify;
                break;
            }
        }
        if (project_file_trojan_suspect(pp, file_name))
            could_have_a_trojan = true;
    }

    //
    // add the modified files to the change
    //
    string_list_ty files_source;
    string_list_ty files_test_auto;
    string_list_ty files_test_manual;
    for (j = 0; j < plp->length; ++j)
    {
        patch_ty        *p;
        string_ty       *file_name;
        cstate_src_ty   *csrc;

        //
        // For now, we are only copying files.
        //
        p = plp->item[j];
        assert(p->name.nstrings >= 1);
        file_name = p->name.string[0];
        csrc = cstate_src_find(change_set, file_name);
        if (csrc)
            p->usage = csrc->usage;
        switch (p->action)
        {
        case file_action_modify:
            break;

        case file_action_create:
        case file_action_remove:
        case file_action_insulate:
        case file_action_transparent:
#ifndef DEBUG
        default:
#endif
            continue;
        }
        switch (p->usage)
        {
        case file_usage_build:
            continue;

        case file_usage_source:
        case file_usage_config:
            break;

        case file_usage_test:
        case file_usage_manual_test:
            need_to_test = true;
            break;
        }

        //
        // add it to the list
        //
        files_source.push_back_unique(file_name);
    }
    bool uncopy = false;
    if (files_source.nstrings)
    {
        string_ty       *delopt;

        delopt = 0;
        if (delta)
        {
            delopt = str_from_c(delta);
            s = str_quote_shell(delopt);
            str_free(delopt);
            delopt = str_format(" --delta=%s", s->str_text);
            str_free(s);
        }
        uncopy = true;
        s =
            str_format
            (
                "aegis --copy-file --project=%s --change=%ld%s --verbose%s%s",
                project_name->str_text,
                change_number,
                trace_options.c_str(),
                (build_files_are_ok ? " -build" : ""),
                (delopt ? delopt->str_text : "")
            );
        if (delopt)
            str_free(delopt);
        os_xargs(s, &files_source, dd);
        str_free(s);

        // change state invalid
        change_lock_sync_forced(cp);
    }

    //
    // add the removed files to the change
    //
    files_source.clear();
    for (j = 0; j < plp->length; ++j)
    {
        patch_ty        *p;

        //
        // For now, we are only removing files.
        //
        p = plp->item[j];
        assert(p->name.nstrings >= 1);
        switch (p->action)
        {
        case file_action_remove:
            break;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
        case file_action_transparent:
#ifndef DEBUG
        default:
#endif
            continue;
        }

        //
        // add it to the list
        //
        files_source.push_back_unique(p->name.string[0]);
    }
    if (files_source.nstrings)
    {
        s =
            str_format
            (
                "aegis --remove-file --project=%s --change=%ld%s --verbose",
                project_name->str_text,
                change_number,
                trace_options.c_str()
            );
        os_xargs(s, &files_source, dd);
        str_free(s);

        // change state invalid
        change_lock_sync_forced(cp);
    }

    //
    // add the new files to the change
    //
    files_source.clear();
    string_list_ty files_config;
    string_list_ty files_build;
    for (j = 0; j < plp->length; ++j)
    {
        string_ty       *fn;
        patch_ty        *p;

        //
        // for now, we are only dealing with create
        //
        p = plp->item[j];
        assert(p->name.nstrings >= 1);
        switch (p->action)
        {
        case file_action_create:
            break;

        case file_action_modify:
        case file_action_remove:
        case file_action_insulate:
        case file_action_transparent:
#ifndef DEBUG
        default:
#endif
            continue;
        }

        //
        // add it to the list
        //
        fn = p->name.string[0];
        switch (p->usage)
        {
        case file_usage_source:
            if (cp->file_is_config(fn))
                files_config.push_back_unique(fn);
            else
                files_source.push_back_unique(fn);
            break;

        case file_usage_config:
            files_config.push_back_unique(fn);
            break;

        case file_usage_build:
            files_build.push_back_unique(fn);
            break;

        case file_usage_test:
            files_test_auto.push_back_unique(fn);
            need_to_test = true;
            break;

        case file_usage_manual_test:
            files_test_manual.push_back_unique(fn);
            need_to_test = true;
            break;
        }
    }

    if (files_test_auto.nstrings)
    {
        s =
            str_format
            (
                "aegis --new-test --automatic --project=%s --change=%ld%s "
                    "--verbose --no-template",
                project_name->str_text,
                change_number,
                trace_options.c_str()
            );
        os_xargs(s, &files_test_auto, dd);
        str_free(s);

        // change state invalid
        change_lock_sync_forced(cp);
    }
    if (files_test_manual.nstrings)
    {
        s =
            str_format
            (
                "aegis --new-test --manual --project=%s --change=%ld%s "
                    "--verbose --no-template",
                project_name->str_text,
                change_number,
                trace_options.c_str()
            );
        os_xargs(s, &files_test_manual, dd);
        str_free(s);

        // change state invalid
        change_lock_sync_forced(cp);
    }

    //
    // NOTE: do this one last, in case it includes the first instance
    // of the project config file.
    //
    if (files_source.nstrings)
    {
        s =
            str_format
            (
                "aegis --new-file --project=%s --change=%ld%s --verbose "
                    "--no-template --no-configured",
                project_name->str_text,
                change_number,
                trace_options.c_str()
            );
        os_xargs(s, &files_source, dd);
        str_free(s);

        // change state invalid
        change_lock_sync_forced(cp);
    }
    if (files_build.nstrings)
    {
        s =
            str_format
            (
                "aegis --new-file --build --project=%s --change=%ld%s "
                    "--verbose --no-template",
                project_name->str_text,
                change_number,
                trace_options.c_str()
            );
        os_xargs(s, &files_build, dd);
        str_free(s);

        // change state invalid
        change_lock_sync_forced(cp);
    }
    if (files_config.nstrings)
    {
        s =
            str_format
            (
                "aegis --new-file --config --project=%s --change=%ld%s "
                    "--verbose --no-template",
                project_name->str_text,
                change_number,
                trace_options.c_str()
            );
        os_xargs(s, &files_config, dd);
        str_free(s);

        // change state invalid
        change_lock_sync_forced(cp);
    }

    //
    // now extract each file from the input
    //
    config_seen = 0;
    for (j = 0; j < plp->length; ++j)
    {
        patch_ty        *p;
        string_ty       *orig;

        // verbose progress message here?
        p = plp->item[j];
        switch (p->action)
        {
        case file_action_create:
        case file_action_modify:
            break;

        case file_action_remove:
        case file_action_insulate:
        case file_action_transparent:
            continue;
        }
        switch (p->usage)
        {
        case file_usage_build:
            continue;

        case file_usage_source:
            if (cp->file_is_config(p->name.string[0]))
            {
                could_have_a_trojan = true;
                config_seen = 1;
            }
            break;

        case file_usage_config:
            could_have_a_trojan = true;
            config_seen = 1;
            break;

        case file_usage_test:
        case file_usage_manual_test:
            could_have_a_trojan = true;
            break;
        }
        assert(p->name.nstrings >= 1);
        trace(("%s\n", p->name.string[0]->str_text));

        //
        // Recall that, somewhere above, we may have messed
        // with the `action' field, so we have to look at the
        // patch itself, and reconstruct whether it is creating
        // new content.
        //
        if
        (
            p->actions.length == 1
        &&
            p->actions.item[0]->before.length == 0
        &&
            p->actions.item[0]->before.start_line_number == 0
        )
            p->action = file_action_create;

        //
        // Apply the patch.
        //
        switch (p->action)
        {
        case file_action_create:
            os_become_orig();
            patch_apply(p, (string_ty *)0, p->name.string[0]);
            os_become_undo();
            break;

        case file_action_modify:
            //
            // This is the normal case: modify an existing file.
            //
            // The input file (to which the patch is applied) may
            // be found in the baseline.
            //
            orig = project_file_path(pp, p->name.string[0]);
            os_become_orig();
            patch_apply(p, orig, p->name.string[0]);
            os_become_undo();
            str_free(orig);
            break;

        case file_action_remove:
            break;

        case file_action_insulate:
        case file_action_transparent:
            assert(0);
            break;
        }
    }
    change_free(cp);
    cp = 0;

    if (change_set)
    {
        //
        // FIXME: update file attributes, using file attribute data from
        // the change_set
        //
    }

    //
    // Un-copy any files which did not change.
    //
    // The idea is, if there are no files left, there is nothing
    // for this change to do, so cancel it.
    //
    if (uncopy)
    {
        s =
            str_format
            (
                "aegis --copy-file-undo --unchanged --change=%ld "
                    "--project=%s%s --verbose",
                change_number,
                project_name->str_text,
                trace_options.c_str()
            );
        os_become_orig();
        os_execute(s, OS_EXEC_FLAG_INPUT, dd);
        os_become_undo();
        str_free(s);

        //
        // If there are no files left, we already have this change.
        //
        if (number_of_files(project_name, change_number) == 0)
        {
            //
            // get out of there
            //
            os_chdir(dot);

            //
            // stop developing the change
            //
            s =
                str_format
                (
                    "aegis --develop-begin-undo --change=%ld --project=%s%s "
                        "--verbose",
                    change_number,
                    project_name->str_text,
                    trace_options.c_str()
                );
            os_become_orig();
            os_execute(s, OS_EXEC_FLAG_INPUT, dot);
            str_free(s);

            //
            // cancel the change
            //
            s =
                str_format
                (
                    "aegis --new-change-undo --change=%ld --project=%s%s "
                        "--verbose",
                    change_number,
                    project_name->str_text,
                    trace_options.c_str()
                );
            os_execute(s, OS_EXEC_FLAG_INPUT, dot);
            os_become_undo();
            str_free(s);

            //
            // run away, run away!
            //
            error_intl(0, i18n("change already present"));
            return;
        }
    }

    //
    // If the change could have a trojan horse in it, stop here with
    // a warning.  The user needs to look at it and check.
    //
    if (trojan > 0)
        could_have_a_trojan = true;
    else if (trojan == 0)
    {
        error_intl(0, i18n("warning: potential trojan, proceeding anyway"));
        could_have_a_trojan = false;
        config_seen = 0;
    }

    //
    // If the change could have a trojan horse in the project config
    // file, stop here with a warning.  Don't even difference the
    // change, because the trojan could be embedded in the diff
    // command.  The user needs to look at it and check.
    //
    // FIX ME: what if the aecpu got rid of it?
    //
    if (config_seen)
    {
        error_intl
        (
            0,
         i18n("warning: potential trojan, review before completing development")
        );
        return;
    }

    //
    // now diff the change
    //
    s =
        str_format
        (
            "aegis --diff --no-merge --change=%ld --project=%s%s --verbose",
            change_number,
            project_name->str_text,
            trace_options.c_str()
        );
    os_become_orig();
    os_execute(s, OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();
    str_free(s);

    //
    // Now that all the files have been unpacked,
    // set the change's UUID.
    //
    // It is vaguely possible you have already downloaded this change
    // before, so we don't complain (OS_EXEC_FLAG_ERROK) if the command
    // fails.
    //
    if (change_set && change_set->uuid)
    {
        string_ty       *quoted_uuid;

        quoted_uuid = str_quote_shell(change_set->uuid);
        s =
            str_format
            (
                "aegis --change-attr --uuid %s -change=%ld --project=%s%s",
                quoted_uuid->str_text,
                change_number,
                project_name->str_text,
                trace_options.c_str()
            );
        str_free(quoted_uuid);
        os_become_orig();
        os_execute(s, OS_EXEC_FLAG_INPUT | OS_EXEC_FLAG_ERROK, dd);
        os_become_undo();
        str_free(s);
    }

    //
    // If the change could have a trojan horse in it, stop here with
    // a warning.  The user needs to look at it and check.
    //
    if (could_have_a_trojan)
    {
        error_intl
        (
            0,
         i18n("warning: potential trojan, review before completing development")
        );
        return;
    }

    //
    // Sleep for a second to make sure the derived files will have
    // mod-times strictly later than the source files, and that the aeb
    // timestamp will also be strictly later then the mod times for the
    // source files.
    //
    sleep(1);

    //
    // now build the change
    //
    s =
        str_format
        (
            "aegis --build --change=%ld --project=%s%s --verbose",
            change_number,
            project_name->str_text,
            trace_options.c_str()
        );
    os_become_orig();
    os_execute(s, OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();
    str_free(s);

    //
    // Sleep for a second to make sure the aet timestamps will be
    // strictly later then the aeb timestamp.
    //
    sleep(1);

    //
    // re-read the change state data.
    //
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    cstate_ty *cstate_data = cp->cstate_get();

    //
    // now test the change
    //
    if (need_to_test && !cstate_data->test_exempt)
    {
        s =
            str_format
            (
                "aegis --test --change=%ld --project=%s%s --verbose",
                change_number,
                project_name->str_text,
                trace_options.c_str()
            );
        os_become_orig();
        os_execute(s, OS_EXEC_FLAG_INPUT, dd);
        os_become_undo();
        str_free(s);
    }
    if (need_to_test && !cstate_data->test_baseline_exempt)
    {
        s =
            str_format
            (
                "aegis --test --baseline --change=%ld --project=%s%s --verbose",
                change_number,
                project_name->str_text,
                trace_options.c_str()
            );
        os_become_orig();
        os_execute(s, OS_EXEC_FLAG_INPUT, dd);
        os_become_undo();
        str_free(s);
    }

    // always do a regession test?
    if (!cstate_data->regression_test_exempt)
    {
        s =
            str_format
            (
                "aegis --test --regression --change=%ld --project=%s%s "
                "--verbose",
                change_number,
                project_name->str_text,
                trace_options.c_str()
            );
        os_become_orig();
        os_execute(s, OS_EXEC_FLAG_INPUT, dd);
        os_become_undo();
        str_free(s);
    }

    change_free(cp);
    cp = 0;
    project_free(pp);
    pp = 0;

    //
    // end development (if we got this far!)
    //
    s =
        str_format
        (
            "aegis --develop-end --change=%ld --project=%s%s --verbose",
            change_number,
            project_name->str_text,
            trace_options.c_str()
        );
    os_become_orig();
    os_execute(s, OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();
    str_free(s);

    if (change_set)
        cstate_type.free(change_set);

    // verbose success message here?
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
