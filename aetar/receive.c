/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to receive change sets
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex3.h>
#include <change.h>
#include <help.h>
#include <input/file.h>
#include <input/gunzip.h>
#include <input/tar.h>
#include <os.h>
#include <output/file.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <project/history.h>
#include <receive.h>
#include <str.h>
#include <sub.h>
#include <undo.h>
#include <user.h>


static void usage _((void));

static void
usage()
{
    char	    *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s --receive [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


void
receive()
{
    string_ty       *project_name;
    long            change_number;
    string_ty       *ifn;
    string_ty       *s;
    project_ty      *pp;
    change_ty       *cp;
    string_ty       *dd;
    string_ty       *attribute_file_name;
    string_ty       *dot;
    const char      *delta;
    string_ty       *devdir;
    input_ty	    *tar_p;
    output_ty	    *ofp;
    int		    trojan;

    project_name = 0;
    change_number = 0;
    ifn = 0;
    trojan = -1;
    delta = 0;
    devdir = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

	case arglex_token_change:
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_change, usage);
	    if (change_number)
		duplicate_option_by_name(arglex_token_change, usage);
	    change_number = arglex_value.alv_number;
	    if (!change_number)
		change_number = MAGIC_ZERO;
	    else if (change_number < 0)
	    {
		sub_context_ty *scp;

		scp = sub_context_new();
		sub_var_set_long(scp, "Number", change_number);
		fatal_intl(scp, i18n("change $number out of range"));
		/* NOTREACHED */
	    }
	    break;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, usage);
	    if (project_name)
		duplicate_option_by_name(arglex_token_project, usage);
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_file:
	    if (ifn)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, usage);
	     /*NOTREACHED*/ case arglex_token_string:
		ifn = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_stdio:
		ifn = str_from_c("");
		break;
	    }
	    break;

	case arglex_token_trojan:
	    if (trojan > 0)
		duplicate_option(usage);
	    if (trojan >= 0)
	    {
	      too_many_trojans:
		mutually_exclusive_options
		    (arglex_token_trojan, arglex_token_trojan_not, usage);
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
	     /*NOTREACHED*/ case arglex_token_number:
	    case arglex_token_string:
		delta = arglex_value.alv_string;
		break;
	    }
	    break;

	case arglex_token_directory:
	    if (devdir)
	    {
		duplicate_option(usage);
		/* NOTREACHED */
	    }
	    if (arglex() != arglex_token_string)
	    {
		option_needs_dir(arglex_token_directory, usage);
		/* NOTREACHED */
	    }
	    devdir = str_format(" --directory %s", arglex_value.alv_string);
	    break;
	}
	arglex();
    }

    /*
     * Open the tape archive file.
     */
    os_become_orig();
    tar_p = input_file_open(ifn);
    tar_p = input_gunzip(tar_p);
    tar_p = input_tar(tar_p);
    os_become_undo();

    /*
     * locate project data
     */
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    project_bind_existing(pp);

    /*
     * default the change number
     */
    if (!change_number)
	change_number = project_next_change_number(pp, 1);

    /*
     * create the new change
     */
    os_become_orig();
    attribute_file_name = os_edit_filename(0);
    undo_unlink_errok(attribute_file_name);
    ofp = output_file_text_open(attribute_file_name);
    output_fputs
    (
	ofp, "\
brief_description = \"none\";\n\
description = \"This change was extracted from a tarball.\";\n\
cause = external_bug;\n"
    );
    output_delete(ofp);
    ofp = 0;

    dot = os_curdir();
    s =
	str_format
	(
	    "aegis --new-change %ld --project=%S --file=%S --verbose",
	    change_number,
	    project_name,
	    attribute_file_name
	);
    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
    str_free(s);
    str_free(attribute_file_name);
    os_unlink_errok(attribute_file_name);

    /*
     * Begin development of the new change.
     */
    s = str_format
	("aegis --develop-begin %ld --project %S --verbose%s",
	change_number, project_name, (devdir ? devdir->str_text : ""));
    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
    str_free(s);
    os_become_undo();

    /*
     * Change to the development directory, so that we can use
     * relative filenames.  It makes things easier to read.
     */
    pp = project_alloc(project_name);
    project_bind_existing(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    dd = change_development_directory_get(cp, 0);
    dd = str_copy(dd);	/* will vanish when change_free(); */
    change_free(cp);
    cp = 0;

    os_chdir(dd);

    /*
     * Now extract each file from the tar archive.
     *
     * We don't know in advance what the file names will be, because
     * that would require two passes.
     */
    for (;;)
    {
	input_ty        *ip;
	string_ty       *filename;
	fstate_src      src_data;
	string_ty       *cmd;

	/*
	 * Find the next file in the archive.
	 */
	os_become_orig();
	ip = input_tar_child(tar_p, &filename);
	os_become_undo();
	if (!ip)
	    break;

	/*
	 * Work out if the file sxists in the project.
	 * This is how we decide to copy or create.
	 */
	src_data = project_file_find(pp, filename);
	if
	(
	    src_data
	&&
	    (src_data->deleted_by || src_data->about_to_be_created_by)
	)
	    src_data = 0;

	/*
	 * Build the command to be run.
	 */
	if (src_data)
	{
	    cmd =
		str_format
		(
		    "aegis --copy-file %S --project=%S --change=%ld --verbose",
		    filename,
		    project_name,
		    change_number
		);
	}
	else
	{
	    cmd =
		str_format
		(
	"aegis --new-file %S --no-template --project=%S --change=%ld --verbose",
		    filename,
		    project_name,
		    change_number
		);
	}
	os_become_orig();
	os_execute(cmd, OS_EXEC_FLAG_INPUT, dd);
	str_free(cmd);

	/*
	 * Now copy the file into the project.
	 */
	ofp = output_file_binary_open(filename);
	input_to_output(ip, ofp);
	output_delete(ofp);
	input_delete(ip);
	os_become_undo();
	str_free(filename);
    }
    input_delete(tar_p);

    project_free(pp);
    pp = 0;
}
