//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate annotates
//

#include <ac/string.h>

#include <annotate.h>
#include <arglex3.h>
#include <arglex/project.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <col.h>
#include <error.h> // for assert
#include <fstate.h>
#include <help.h>
#include <input/file_text.h>
#include <line_list.h>
#include <mem.h>
#include <now.h>
#include <os.h>
#include <output.h>
#include <patch.h>
#include <patch/list.h>
#include <project.h>
#include <project/file.h>
#include <project/file/roll_forward.h>
#include <str.h>
#include <sub.h>
#include <symtab.h>
#include <trace.h>
#include <undo.h>
#include <usage.h>
#include <user.h>


struct column_t
{
    string_ty	    *formula;
    string_ty	    *heading;
    int		    width;
    output_ty	    *fp;
    symtab_ty	    *stp;
    long	    maximum;
    string_ty       *previous;
    string_ty       *newval;
};


struct column_list_t
{
    size_t	    length;
    size_t	    maximum;
    column_t	    *item;
};


static column_list_t columns;
static symtab_ty *file_stp;
static int      filestat = -1;
static const char *diff_option;


static void
column_list_append(column_list_t *clp, string_ty *formula, string_ty *heading,
    int width)
{
    column_t	    *cp;

    if (clp->length >= clp->maximum)
    {
	size_t		nbytes;

	clp->maximum = clp->maximum * 2 + 4;
	nbytes = clp->maximum * sizeof(clp->item[0]);
	clp->item = (column_t *)mem_change_size(clp->item, nbytes);
    }
    cp = clp->item + clp->length++;
    cp->formula = formula;
    cp->heading = heading;
    cp->width = width;
    cp->fp = 0;
    cp->stp = new symtab_ty(5);
    cp->maximum = 1;
    cp->previous = 0;
    cp->newval = 0;
}


static void
process(project_ty *pp, string_ty *filename, line_list_t *buffer)
{
    time_t	    when;
    size_t	    j;
    file_event_list_ty *felp;
    string_ty	    *prev_ifn;
    int		    prev_ifn_unlink;
    size_t	    linum;
    string_ty	    *output_file_name;
    patch_list_ty   *plp;
    patch_ty	    *pap;

    //
    // Get the time to extract the files at.
    //
    // FIXME: add --delta options, so that we can select a time based
    // on a delta.
    //
    when = now();

    //
    // Reconstruct the file history.
    //
    project_file_roll_forward historian(pp, when, 1);

    felp = historian.get(filename);
    if (!felp)
    {
	sub_context_ty  *scp;

	// FIXME: add fuzzy file name matching
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	project_fatal(pp, scp, i18n("no $filename"));
	// project_fatal(pp, scp, i18n("no $filename, closest is $guess"));
	// NOTREACHED
	sub_context_delete(scp);
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
    for (j = 0; j < felp->length; ++j)
    {
	file_event_ty	*fep;
	string_ty	*ifn;
	int		ifn_unlink;
	input_ty	*ifp;
	size_t		m;

	//
	// find the file within the change
	//
	fep = felp->item + j;
	assert(fep->src);

	//
	// What we do next depends on what the change did to the file.
	//
	ifn = 0;
	ifn_unlink = 0;
	switch (fep->src->action)
	{
	case file_action_create:
	    //
	    // read whole file into buffer
	    //
	    trace(("create %s\n", change_version_get(fep->cp)->str_text));
	    line_list_clear(buffer);
	    ifn = project_file_version_path(pp, fep->src, &ifn_unlink);
	    os_become_orig();
	    ifp = input_file_text_open(ifn);
	    for (linum = 0;; ++linum)
	    {
		string_ty	*s;

		s = input_one_line(ifp);
		if (!s)
		    break;
		line_list_insert(buffer, linum, fep->cp, s);
		assert(buffer->item[linum].cp == fep->cp);
		assert(str_equal(buffer->item[linum].text, s));
		str_free(s);
	    }
	    input_delete(ifp);
	    os_become_undo();
	    break;

	case file_action_insulate:
	case file_action_transparent:
	    assert(0);
	    // fall through...

	case file_action_modify:
	    trace(("modify %s\n", change_version_get(fep->cp)->str_text));

	    //
	    // generate the difference between the last edit and this edit.
	    //
	    ifn = project_file_version_path(pp, fep->src, &ifn_unlink);
	    change_run_annotate_diff_command
	    (
		fep->cp,
		user_executing(fep->cp->pp),
		prev_ifn,
		ifn,
		output_file_name,
		filename,
		diff_option
	    );

	    //
	    // read the diff in as a patch
	    //
	    os_become_orig();
	    ifp = input_file_text_open(output_file_name);
	    plp = patch_read(ifp, 0);
	    input_delete(ifp);
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
		patch_hunk_ty	*php;
		size_t		first_line;
		size_t		k;

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
			    fep->cp,
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
		string_ty	*s;
		line_t		*lp;

		lp = buffer->item + buffer->start1 + m;
		s = input_one_line(ifp);
		if (!s)
		{
		    trace(("line %d: file too short\n", linum));
		    assert(0);
		    break;
		}
		if (!str_equal(lp->text, s))
		{
		    trace(("line %d: lp->text %08lX != s %08lX\n", linum,
			(long)lp->text, (long)s));
		    assert(0);
		}
		str_free(s);
	    }
	    for (m = 0 ; m < buffer->length2; ++m, ++linum)
	    {
		string_ty	*s;
		line_t		*lp;

		lp = buffer->item + buffer->start2 + m;
		s = input_one_line(ifp);
		if (!s)
		{
		    trace(("line %d: file too short\n", linum));
		    assert(0);
		    break;
		}
		if (!str_equal(lp->text, s))
		{
		    trace(("line %d: lp->text %08lX != s %08lX\n", linum,
			(long)lp->text, (long)s));
		    assert(0);
		}
		str_free(s);
	    }
	    input_delete(ifp);
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

	//
	// check that our reconstruction matches the file contents
	//
    }
}


static void
incr(symtab_ty *stp, string_ty *key, long *maximum_p)
{
    long	    *data;

    data = (long int *)stp->query(key);
    if (!data)
    {
	static size_t   templen;
	static long	*temp;

	if (templen == 0)
	{
	    templen = 100;
	    temp = (long int *)mem_alloc(templen * sizeof(long));
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
emit_range(output_ty *line_col, output_ty *source_col, line_t *line_array,
    size_t line_len, long *linum_p, col_ty *ofp)
{
    size_t	    j;

    for (j = 0; j < line_len; ++j, ++*linum_p)
    {
	size_t		k;
	line_t		*lp;
	int		changed;

	lp = line_array + j;
	changed = 0;
	for (k = 0; k < columns.length; ++k)
	{
	    column_t	    *cp;

	    cp = columns.item + k;
	    cp->newval = substitute(0, lp->cp, cp->formula);
	    incr(cp->stp, cp->newval, &cp->maximum);

	    if (!cp->previous || !str_equal(cp->previous, cp->newval))
		++changed;
	}
	for (k = 0; k < columns.length; ++k)
	{
	    column_t	    *cp;

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
	col_eoln(ofp);

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
    project_ty *pp)
{
    col_ty	    *ofp;
    output_ty	    *line_col;
    output_ty	    *source_col;
    size_t	    j;
    long	    linum;
    int		    left;

    trace(("buf line = %ld\n", (long)(buffer->length1 + buffer->length2)));
    ofp = col_open(outfilename);
    col_title(ofp, "Annotated File Listing", filename->str_text);

    //
    // Create the columns.
    //
    left = 0;
    for (j = 0; j < columns.length; ++j)
    {
	column_t	*cp;

	cp = columns.item + j;
	cp->fp = col_create(ofp, left, left + cp->width, cp->heading->str_text);
	left += cp->width + 1;
    }
    line_col = col_create(ofp, left, left + 6, "Line\n------");
    source_col = col_create(ofp, left + 7, 0, "Source\n---------");
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
	col_eject(ofp);
	col_title(ofp, "Statistics", filename->str_text);
	--linum;
    }
    for (j = 0; j < columns.length; ++j)
    {
	column_t	*cp;
	size_t		k;

	cp = columns.item + j;
	string_list_ty keys;
	cp->stp->keys(&keys);
	col_need(ofp, keys.nstrings > 10 ? 10 : (int)keys.nstrings);
	keys.sort_version();
	for (k = 0; k < keys.nstrings; ++k)
	{
	    string_ty	    *key;
	    long	    *data;

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
	    col_eoln(ofp);
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
	col_need(ofp, keys.nstrings > 10 ? 10 : (int)keys.nstrings);
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
	    src = project_file_find(pp, key, view_path_extreme);
	    if (!src)
		continue;
	    line_col->fprintf("%5ld", *data);
	    source_col->fputs(key);
	    col_eoln(ofp);
	}
    }
    col_close(ofp);
}


void
annotate(void)
{
    string_ty	    *project_name;
    string_ty	    *filename;
    string_ty	    *outfile;
    project_ty	    *pp;
    line_list_t	    buffer;

    trace(("annotate()\n{\n"));
    project_name = 0;
    filename = 0;
    outfile = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, usage);
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
		string_ty	*formula;
		string_ty	*heading;
		int		width;
		char		*minus;
		string_ty	*s;

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

		minus = (char *)mem_alloc(width + 1);
		memset(minus, '-', width);
		minus[width] = 0;
		s = str_format("%.*s\n%s", width, heading->str_text, minus);
		mem_free(minus);
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
    if (!filename)
	fatal_intl(0, i18n("no file names"));

    //
    // Insert the default columsn if the user does not specify any.
    //
    if (columns.length == 0)
    {
	string_ty	*formula;
	string_ty	*heading;
	int		width;

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

    //
    // locate project data
    //	    (Even of we don't use it, this confirms it is a valid
    //	    project name.)
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    project_bind_existing(pp);

    process(pp, filename, &buffer);
    trace(("buf lines = %ld\n", (long)(buffer.length1 + buffer.length2)));
    emit(&buffer, outfile, filename, pp);

    trace(("}\n"));
}
