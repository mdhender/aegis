//
// aegis - project change supervisor
// Copyright (C) 2002-2009, 2011, 2012 Peter Miller
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
#include <common/ac/string.h>

#include <common/now.h>
#include <common/str.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/change.h>
#include <libaegis/col.h>
#include <libaegis/file/event.h>
#include <libaegis/file/event/list.h>
#include <libaegis/fstate.fmtgen.h>
#include <libaegis/help.h>
#include <libaegis/input/file_text.h>
#include <libaegis/os.h>
#include <libaegis/output.h>
#include <libaegis/patch.h>
#include <libaegis/patch/list.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aeannotate/annotate.h>
#include <aeannotate/arglex3.h>
#include <aeannotate/line_list.h>
#include <aeannotate/usage.h>


struct column_t
{
    ~column_t() { }

    column_t() :
        formula(0),
        heading(0),
        width(0),
        stp(0),
        maximum(0),
        previous(0),
        newval(0)
    {
    }

    string_ty       *formula;
    string_ty       *heading;
    int             width;
    output::pointer fp;
    symtab_ty       *stp;
    long            maximum;
    string_ty       *previous;
    string_ty       *newval;
};


struct column_list_t
{
    ~column_list_t()
    {
        delete [] item;
    }

    column_list_t() :
        length(0),
        maximum(0),
        item(0)
    {
    }

    size_t          length;
    size_t          maximum;
    column_t        *item;
};


static column_list_t columns;
static symtab_ty *file_stp;
static int      filestat = -1;
static const char *diff_option;


static void
column_list_append(column_list_t *clp, string_ty *formula, string_ty *heading,
    int width)
{
    if (clp->length >= clp->maximum)
    {
        clp->maximum = clp->maximum * 2 + 4;
        column_t *new_item = new column_t [clp->maximum];
        for (size_t j = 0; j < clp->length; ++j)
            new_item[j] = clp->item[j];
        delete [] clp->item;
        clp->item = new_item;
    }
    column_t *cp = clp->item + clp->length++;
    cp->formula = formula;
    cp->heading = heading;
    cp->width = width;
    cp->fp.reset();
    cp->stp = new symtab_ty(5);
    cp->maximum = 1;
    cp->previous = 0;
    cp->newval = 0;
}


static string_ty *
change_number_get(change::pointer cp)
{
    return str_format("%ld", magic_zero_decode(cp->number));
}


static void
process(change_identifier &cid, string_ty *filename, line_list_t *buffer)
{
    size_t          j;
    file_event_list::pointer felp;
    string_ty       *prev_ifn;
    int             prev_ifn_unlink;
    size_t          linum;
    string_ty       *output_file_name;
    patch_list_ty   *plp;
    patch_ty        *pap;

    //
    // We can't cope with change history that isn't in history yet.
    //
    change::pointer cp = cid.get_cp();
    project *pp = cid.get_pp();
    if (!cp->was_a_branch() && !cp->is_completed())
    {
        sub_context_ty sc;
        sc.var_set_string("Number", change_number_get(cp));
        project_fatal(pp, &sc, i18n("change $number not completed"));
        // NOTREACHED
    }

    //
    // Get the time to extract the files at.
    //
    time_t when = cp->completion_timestamp();

    //
    // Reconstruct the file history.
    //
    project_file_roll_forward historian(pp, when, 1);

    felp = historian.get(filename);
    if (!felp)
    {
        // FIXME: add fuzzy file name matching
        // project_fatal(pp, &sc, i18n("no $filename, closest is $guess"));
        sub_context_ty sc;
        sc.var_set_string("File_Name", filename);
        project_fatal(pp, &sc, i18n("no $filename"));
        // NOTREACHED
    }

    //
    // We need a temporary file to park patches in.
    //
    output_file_name = os_edit_filename(0);
    os_become_orig();
    undo_unlink_errok(output_file_name);
    os_become_undo();

    //
    // Process each event in the file's history.
    //
    line_list_constructor(buffer);
    prev_ifn = 0;
    prev_ifn_unlink = 0;
    for (j = 0; j < felp->size(); ++j)
    {
        trace(("j = %d of %d\n", (int)j, (int)felp->size()));
        file_event      *fep;
        string_ty       *ifn;
        int             ifn_unlink;
        input ifp;
        size_t          m;

        //
        // find the file within the change
        //
        fep = felp->get(j);
        assert(fep->get_src());

        //
        // What we do next depends on what the change did to the file.
        //
        ifn = 0;
        ifn_unlink = 0;
        switch (fep->get_src()->action)
        {
        case file_action_create:
            //
            // read whole file into buffer
            //
            trace(("create %s\n",
                fep->get_change()->version_get().c_str()));
            treat_as_create:
            line_list_clear(buffer);
            ifn = project_file_version_path(pp, fep->get_src(), &ifn_unlink);
            os_become_orig();
            ifp = input_file_text_open(ifn);
            for (linum = 0;; ++linum)
            {
                nstring s;
                if (!ifp->one_line(s))
                    break;
                line_list_insert(buffer, linum, fep->get_change(), s.get_ref());
                assert(buffer->item[linum].cp == fep->get_change());
                assert(str_equal(buffer->item[linum].text, s.get_ref()));
            }
            ifp.close();
            os_become_undo();
            break;

        case file_action_insulate:
        case file_action_transparent:
            assert(0);
            // fall through...

        case file_action_modify:
            trace(("modify %s\n",
                fep->get_change()->version_get().c_str()));
            if (!prev_ifn)
            {
                trace(("treating this modify as a create\n"));
                goto treat_as_create;
            }

            //
            // generate the difference between the last edit and this edit.
            //
            ifn = project_file_version_path(pp, fep->get_src(), &ifn_unlink);
            trace(("prev_ifn = \"%s\"\n", prev_ifn ? prev_ifn->str_text :
                "NULL"));
            trace(("ifn = \"%s\"\n", ifn->str_text));
            trace(("output_file_name = \"%s\"\n", output_file_name->str_text));
            trace(("filename = \"%s\"\n", filename->str_text));
            trace(("diff_option = \"%s\"\n", diff_option));
            change_run_annotate_diff_command
            (
                cid.get_cp(),
                user_ty::create(),
                prev_ifn,
                ifn,
                output_file_name,
                filename,
                (diff_option ? diff_option : "")
            );

            //
            // read the diff in as a patch
            //
            os_become_orig();
            ifp = input_file_text_open(output_file_name);
            plp = patch_read(ifp, 0);
            ifp.close();
            os_become_undo();
            assert(plp);

            //
            // There should be either 0 or 1 files in the patch list we
            // just read.
            //
            assert(plp->length < 2);
            if (plp->length == 0)
            {
                patch_list_delete(plp);
                break;
            }
            pap = plp->item[0];

            //
            // Work through the hunks, applying them one at a time.
            //
            // By working backwards, the "before" line numbers are always
            // valid, even as the buffer grows and strinks as we apply
            // the patch hunks.
            //
            for (m = 0; m < pap->actions.length; ++m)
            {
                patch_hunk_ty   *php;
                size_t          first_line;
                size_t          k;

                php = pap->actions.item[pap->actions.length - 1 - m];
                first_line = php->before.start_line_number;
                if (first_line > 0)
                    --first_line;
                line_list_delete
                (
                    buffer,
                    first_line,
                    php->before.length
                );
                for (k = 0; k < php->after.length; ++k)
                {
                    patch_line_ty   *plip;

                    plip = php->after.item + k;
                    assert(plip->type != patch_line_type_deleted);
                    if (plip->type != patch_line_type_deleted)
                    {
                        line_list_insert
                        (
                            buffer,
                            first_line++,
                            fep->get_change(),
                            plip->value
                        );
                    }
                }
            }
            patch_list_delete(plp);

#ifdef DEBUG
            //
            // Check that our reconstruction matches the file contents.
            //
            // Note that this assumes they didn't use any of
            // the more interesting white space options in the
            // annotate_diff_command.
            //
            os_become_orig();
            ifp = input_file_text_open(ifn);
            linum = 1;
            for (m = 0 ; m < buffer->length1; ++m, ++linum)
            {
                line_t *lp = buffer->item + buffer->start1 + m;
                nstring s;
                if (!ifp->one_line(s))
                {
                    trace(("line %zd: file too short\n", linum));
                    assert(0);
                    break;
                }
                if (nstring(lp->text) != s)
                {
                    trace(("line %zd: lp->text %08lX != s %08lX\n", linum,
                        (long)lp->text, (long)s.c_str()));
                    assert(0);
                }
            }
            for (m = 0 ; m < buffer->length2; ++m, ++linum)
            {
                line_t *lp = buffer->item + buffer->start2 + m;
                nstring s;
                if (!ifp->one_line(s))
                {
                    trace(("line %zd: file too short\n", linum));
                    assert(0);
                    break;
                }
                if (nstring(lp->text) != s)
                {
                    trace(("line %zd: lp->text %08lX != s %08lX\n", linum,
                        (long)lp->text, (long)s));
                    assert(0);
                }
            }
            ifp.close();
            os_become_undo();
#endif
            break;

        case file_action_remove:
            trace(("remove\n"));
            line_list_clear(buffer);
            break;
        }
        if (prev_ifn)
        {
            if (prev_ifn_unlink)
            {
                os_become_orig();
                os_unlink(prev_ifn);
                os_become_undo();
            }
            str_free(prev_ifn);
        }
        prev_ifn = ifn;
        prev_ifn_unlink = ifn_unlink;

        trace(("buf line = %ld\n", (long)(buffer->length1 + buffer->length2)));
    }
}


static void
incr(symtab_ty *stp, string_ty *key, long *maximum_p)
{
    long            *data;

    data = (long int *)stp->query(key);
    if (!data)
    {
        static size_t   templen;
        static long     *temp;

        if (templen == 0)
        {
            templen = 100;
            temp = new long [templen];
        }
        data = temp++;
        --templen;
        *data = 0;
        stp->assign(key, data);
    }
    ++*data;
    if (maximum_p && *data > *maximum_p)
        *maximum_p = *data;
}


static void
emit_range(output::pointer line_col, output::pointer source_col,
    line_t *line_array, size_t line_len, long *linum_p, col::pointer ofp)
{
    size_t          j;

    for (j = 0; j < line_len; ++j, ++*linum_p)
    {
        size_t          k;
        line_t          *lp;
        int             changed;

        lp = line_array + j;
        changed = 0;
        for (k = 0; k < columns.length; ++k)
        {
            column_t        *cp;

            cp = columns.item + k;
            cp->newval = substitute(0, lp->cp, cp->formula);
            incr(cp->stp, cp->newval, &cp->maximum);

            if (!cp->previous || !str_equal(cp->previous, cp->newval))
                ++changed;
        }
        for (k = 0; k < columns.length; ++k)
        {
            column_t        *cp;

            cp = columns.item + k;
            if (changed)
            {
                //
                // The choices are to print only the columns changed, or
                // all the columns when one column changes.  CVS annotate
                // prints all of the columns.
                //
                cp->fp->fputs(cp->newval);
            }
            if (cp->previous)
                str_free(cp->previous);
            cp->previous = cp->newval;
            cp->newval = 0;
        }
        line_col->fprintf("%5ld", *linum_p);
        source_col->fputs(lp->text);
        ofp->eoln();

        //
        // Collect the file statistics.
        //
        for (k = 0; ; ++k)
        {
            fstate_src_ty   *src;

            src = change_file_nth(lp->cp, k, view_path_first);
            if (!src)
                break;
            incr(file_stp, src->file_name, 0);
        }
    }
}


static void
emit(line_list_t *buffer, string_ty *outfilename, string_ty *filename,
    project *pp)
{
    col::pointer ofp;
    output::pointer line_col;
    output::pointer source_col;
    size_t          j;
    long            linum;
    int             left;

    trace(("buf line = %ld\n", (long)(buffer->length1 + buffer->length2)));
    ofp = col::open(outfilename);
    ofp->title("Annotated File Listing", filename->str_text);

    //
    // Create the columns.
    //
    left = 0;
    for (j = 0; j < columns.length; ++j)
    {
        column_t        *cp;

        cp = columns.item + j;
        cp->fp = ofp->create(left, left + cp->width, cp->heading->str_text);
        left += cp->width + 1;
    }
    line_col = ofp->create(left, left + 6, "Line\n------");
    source_col = ofp->create(left + 7, 0, "Source\n---------");
    file_stp = new symtab_ty(5);

    //
    // Emit the lines.
    //
    linum = 1;
    trace(("buffer->length1 = %ld\n", (long)buffer->length1));
    emit_range
    (
        line_col,
        source_col,
        buffer->item + buffer->start1,
        buffer->length1,
        &linum,
        ofp
    );
    trace(("buffer->length2 = %ld\n", (long)buffer->length2));
    emit_range
    (
        line_col,
        source_col,
        buffer->item + buffer->start2,
        buffer->length2,
        &linum,
        ofp
    );

    if (linum > 1)
    {
        ofp->eject();
        ofp->title("Statistics", filename->str_text);
        --linum;
    }
    for (j = 0; j < columns.length; ++j)
    {
        column_t        *cp;
        size_t          k;

        cp = columns.item + j;
        string_list_ty keys;
        cp->stp->keys(&keys);
        ofp->need(keys.nstrings > 10 ? 10 : (int)keys.nstrings);
        keys.sort_version();
        for (k = 0; k < keys.nstrings; ++k)
        {
            string_ty       *key;
            long            *data;

            key = keys.string[k];
            data = (long int *)cp->stp->query(key);
            assert(key);
            if (!data)
                continue;
            cp->fp->fputs(key);
            line_col->fprintf("%5ld", *data);
            source_col->fprintf("%6.2f%%", 100. * *data / linum);

            //
            // Histogram in trhe rest of the line.
            //
            left = source_col->page_width() - 8;
            if (left > 0)
            {
                if (left > 50)
                    left = 50;
                left = (left * *data + cp->maximum / 2) / cp->maximum;
                if (left > 0)
                {
                    source_col->fputc(' ');
                    for (;;)
                    {
                        source_col->fputc('*');
                        --left;
                        if (left <= 0)
                            break;
                    }
                }
            }
            ofp->eoln();
        }
    }

    if (filestat > 0)
    {
        //
        // Emit the file statistics.
        //
        string_list_ty keys;
        file_stp->keys(&keys);
        keys.sort();
        ofp->need(keys.nstrings > 10 ? 10 : (int)keys.nstrings);
        for (j = 0; j < keys.nstrings; ++j)
        {
            string_ty       *key;
            long            *data;
            fstate_src_ty   *src;

            key = keys.string[j];
            if (str_equal(key, filename))
                continue;
            data = (long int *)file_stp->query(key);
            assert(key);
            if (!data)
                continue;
            src = pp->file_find(key, view_path_extreme);
            if (!src)
                continue;
            line_col->fprintf("%5ld", *data);
            source_col->fputs(key);
            ofp->eoln();
        }
    }
}


void
annotate(void)
{
    string_ty       *filename;
    string_ty       *outfile;
    line_list_t     buffer;

    trace(("annotate()\n{\n"));
    filename = 0;
    outfile = 0;
    change_identifier cid;
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

        case arglex_token_string:
            if (filename)
                fatal_intl(0, i18n("too many files"));
            filename = str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_output:
            if (outfile)
                duplicate_option(usage);
            if (arglex() != arglex_token_string)
                option_needs_file(arglex_token_output, usage);
            outfile = str_from_c(arglex_value.alv_string);
            break;

        case arglex_token_column:
            {
                string_ty       *formula;
                string_ty       *heading;
                int             width;
                string_ty       *s;

                if (arglex() != arglex_token_string)
                    option_needs_string(arglex_token_column, usage);
                formula = str_from_c(arglex_value.alv_string);

                if (arglex() == arglex_token_string)
                {
                    heading = str_from_c(arglex_value.alv_string);
                    arglex();
                }
                else
                    heading = str_copy(formula);

                if (arglex_token == arglex_token_number)
                {
                    width = arglex_value.alv_number;
                    if (width < 1)
                        width = 7;
                    arglex();
                }
                else
                    width = 7;

                char *minus_ch = new char [width + 1];
                memset(minus_ch, '-', width);
                minus_ch[width] = 0;
                s = str_format("%.*s\n%s", width, heading->str_text, minus_ch);
                delete [] minus_ch;
                str_free(heading);
                heading = s;

                column_list_append(&columns, formula, heading, width);
            }
            continue;

        case arglex_token_filestat:
            filestat = 1;
            break;

        case arglex_token_filestat_not:
            filestat = 0;
            break;

        case arglex_token_diff_option:
            if (diff_option)
                duplicate_option(usage);
            if (arglex_get_string() != arglex_token_string)
                option_needs_file(arglex_token_diff_option, usage);
            diff_option = arglex_value.alv_string;
            break;
        }
        arglex();
    }
    if (!cid.set())
        cid.set_baseline();
    cid.command_line_check(usage);
    if (!filename)
        fatal_intl(0, i18n("no file names"));

    //
    // Insert the default columns if the user does not specify any.
    //
    if (columns.length == 0)
    {
        string_ty       *formula;
        string_ty       *heading;
        int             width;

        formula = str_from_c("${ch date %Y-%m}");
        heading = str_from_c("Date\n-------");
        width = 7;
        column_list_append(&columns, formula, heading, width);

        formula = str_from_c("$version");
        heading = str_from_c("Version\n---------");
        width = 9;
        column_list_append(&columns, formula, heading, width);

        formula = str_from_c("${ch developer}");
        heading = str_from_c("Who\n--------");
        width = 8;
        column_list_append(&columns, formula, heading, width);
    }

    process(cid, filename, &buffer);
    trace(("buf lines = %ld\n", (long)(buffer.length1 + buffer.length2)));
    emit(&buffer, outfile, filename, cid.get_pp());

    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
