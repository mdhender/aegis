//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to receive change sets
//

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <arglex3.h>
#include <arglex/change.h>
#include <arglex/project.h>
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
#include <str_list.h>
#include <sub.h>
#include <undo.h>
#include <user.h>


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
mangle(string_ty **filename_p)
{
    size_t          k;
    string_ty       *s;

    for (k = 0; k < path_prefix_remove.nstrings; ++k)
    {
	s = is_a_path_prefix(*filename_p, path_prefix_remove.string[k]);
	if (s)
	{
	    str_free(*filename_p);
	    *filename_p = s;
	}
    }
    if (path_prefix_add)
    {
	s = os_path_cat(path_prefix_add, *filename_p);
	str_free(*filename_p);
	*filename_p = s;
    }
}


void
receive(void)
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

	    case arglex_token_string:
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
		// NOTREACHED

	    case arglex_token_number:
	    case arglex_token_string:
		delta = arglex_value.alv_string;
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

	case arglex_token_path_prefix_add:
	    if (path_prefix_add)
		duplicate_option(usage);
	    if (arglex() != arglex_token_string)
		option_needs_file(arglex_token_path_prefix_add, usage);
	    path_prefix_add = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_path_prefix_remove:
	    if (arglex() != arglex_token_string)
		option_needs_file(arglex_token_path_prefix_add, usage);
	    s = str_from_c(arglex_value.alv_string);
	    string_list_append_unique(&path_prefix_remove, s);
	    str_free(s);
	    break;
	}
	arglex();
    }

    //
    // Open the tape archive file.
    //
    os_become_orig();
    tar_p = input_file_open(ifn);
    tar_p = input_gunzip(tar_p);
    tar_p = input_tar(tar_p);
    os_become_undo();

    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    project_bind_existing(pp);

    //
    // default the change number
    //
    if (!change_number)
	change_number = project_next_change_number(pp, 1);

    //
    // create the new change
    //
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
	    "aegis --new-change %ld --project=%s --file=%s --verbose",
	    change_number,
	    project_name->str_text,
	    attribute_file_name->str_text
	);
    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
    str_free(s);
    str_free(attribute_file_name);
    os_unlink_errok(attribute_file_name);

    //
    // Begin development of the new change.
    //
    s =
	str_format
	(
	    "aegis --develop-begin %ld --project %s --verbose%s",
	    change_number,
	    project_name->str_text,
	    (devdir ? devdir->str_text : "")
	);
    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
    str_free(s);
    os_become_undo();

    //
    // Change to the development directory, so that we can use
    // relative filenames.  It makes things easier to read.
    //
    pp = project_alloc(project_name);
    project_bind_existing(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    dd = change_development_directory_get(cp, 0);
    dd = str_copy(dd);	// will vanish when change_free();
    change_free(cp);
    cp = 0;

    os_chdir(dd);

    //
    // Now extract each file from the tar archive.
    //
    // We don't know in advance what the file names will be, because
    // that would require two passes.
    //
    for (;;)
    {
	input_ty        *ip;
	string_ty       *filename;
	fstate_src_ty   *src_data;
	string_ty       *cmd;

	//
	// Find the next file in the archive.
	//
	os_become_orig();
	ip = input_tar_child(tar_p, &filename);
	os_become_undo();
	if (!ip)
	    break;

	//
	// Mangle the file name if necessary.
	//
	mangle(&filename);

	//
	// Work out if the file sxists in the project.
	// This is how we decide to copy or create.
	//
	src_data = project_file_find(pp, filename, view_path_extreme);

	//
	// Build the command to be run.
	//
	if (src_data)
	{
	    cmd =
		str_format
		(
		    "aegis --copy-file %s --project=%s --change=%ld --verbose",
		    filename->str_text,
		    project_name->str_text,
		    change_number
		);
	}
	else
	{
	    cmd =
		str_format
		(
	"aegis --new-file %s --no-template --project=%s --change=%ld --verbose",
		    filename->str_text,
		    project_name->str_text,
		    change_number
		);
	}
	os_become_orig();
	os_execute(cmd, OS_EXEC_FLAG_INPUT, dd);
	str_free(cmd);

	//
	// Now copy the file into the project.
	//
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
