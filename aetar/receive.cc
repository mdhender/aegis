//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
//	Copyright (C) 2006, 2007 Walter Franzini;
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <aetar/arglex3.h>
#include <common/error.h>       // for assert
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change.h>
#include <libaegis/help.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <aetar/input/tar.h>
#include <common/nstring.h>
#include <common/nstring/list.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <common/progname.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <aedist/receive.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s --receive [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


static bool
is_a_path_prefix(const nstring &haystack, const nstring &needle,
    nstring &result)
{
    if
    (
	haystack.size() > needle.size()
    &&
	haystack[needle.size()] == '/'
    &&
	0 == memcmp(haystack.c_str(), needle.c_str(), needle.size())
    )
    {
	result =
	    nstring
	    (
		haystack.c_str() + needle.size() + 1,
		haystack.size() - needle.size() - 1
	    );
	return true;
    }
    return false;
}

static nstring
remove_path_prefix(const nstring& haystack, long count)
{
    size_t n;
    long slash = 0;

    trace(("remove_path_prefix(\"%s\", %ld)\n{\n", haystack.c_str(), count));

    if (count == 0)
    {
        trace(("return \"%s\";\n}\n", haystack.c_str()));
        return haystack;
    }

    for (n = 0; n < haystack.size(); ++n)
    {
        if (haystack[n] != '/')
            continue;
        //
        // A sequence of one or more slashes is counted as a single slash.
        //
        if (n < (haystack.size() - 1) && haystack[n] == haystack[ n + 1 ])
            continue;
        ++slash;
        if (slash == count)
            break;
    }
    trace(("return \"%s\";\n}\n", haystack.c_str() + n + 1));
    return nstring(haystack.c_str() + n + 1, haystack.size() - n - 1);
}


static nstring path_prefix_add;
static nstring_list path_prefix_remove;
static long path_prefix_remove_count = -1;


static void
mangle(nstring &filename)
{
    if (path_prefix_remove_count >= 0)
    {
        assert(path_prefix_remove.size() == 0);
        filename = remove_path_prefix(filename, path_prefix_remove_count);
    }

    for (size_t k = 0; k < path_prefix_remove.size(); ++k)
    {
        assert(path_prefix_remove_count == -1);
	nstring s;
	if (is_a_path_prefix(filename, path_prefix_remove[k], s))
	{
	    filename = s;
	}
    }
    if (!path_prefix_add.empty())
    {
	filename = os_path_cat(path_prefix_add, filename);
    }
}


static void
get_exclude_attribute(change::pointer cp, nstring_list &exclude)
{
    nstring value = cp->pconf_attributes_find("aetar:exclude");
    exclude.split(value);
}


static void
read_cvs_ignore(const nstring &filename, nstring_list &patterns)
{
    nstring dir = filename.dirname();
    if (dir == ".")
        dir = "";
    else
        dir += "/";
    FILE *fp = fopen(filename.c_str(), "r");
    if (!fp)
        return;
    for (;;)
    {
        char buffer[300];
        if (!fgets(buffer, sizeof(buffer), fp))
            break;

        char *bp = buffer;
        while (*bp && isspace((unsigned char)*bp))
            ++bp;
        if (!*bp || *bp == '#')
            continue;
        char *ep = bp + strlen(bp);
        while (ep > bp && isspace((unsigned char)ep[-1]))
            --ep;
        assert(ep > bp);

        patterns.push_back(dir + nstring(bp, ep - bp));
    }
    fclose(fp);
}


static void
remove_by_pattern(nstring_list &filenames, const nstring_list &patterns)
{
    nstring_list culled;
    for (size_t j = 0; j < filenames.size(); ++j)
    {
        nstring fn = filenames[j];
        if (fn.gmatch(patterns))
            os_unlink_errok(fn);
        else
            culled.push_back(fn);
    }
    filenames = culled;
}


void
receive(void)
{
    trace(("receive()\n{\n"));
    string_ty       *project_name;
    nstring         ifn;
    string_ty       *s;
    project_ty      *pp;
    change::pointer cp;
    string_ty       *attribute_file_name;
    string_ty       *dot;
    const char      *delta;
    string_ty       *devdir;
    int		    trojan;

    project_name = 0;
    long change_number = 0;
    trojan = -1;
    delta = 0;
    devdir = 0;
    nstring_list exclude_patterns;
    int exclude_auto_tools = -1;
    int exclude_cvs = -1;
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
	    if (!ifn.empty())
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, usage);
		// NOTREACHED

	    case arglex_token_string:
		ifn = arglex_value.alv_string;
		break;

	    case arglex_token_stdio:
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
	    path_prefix_add = arglex_value.alv_string;
	    break;

	case arglex_token_path_prefix_remove:
            switch (arglex())
            {
            case arglex_token_string:
                if (path_prefix_remove_count >= 0)
                    duplicate_option(usage);
                path_prefix_remove.push_back_unique(arglex_value.alv_string);
                break;

            case arglex_token_number:
                if (path_prefix_remove.size() > 0)
                    duplicate_option(usage);
                if (arglex_value.alv_number < 0)
                    option_needs_number(arglex_token_path_prefix_remove, usage);
                path_prefix_remove_count = arglex_value.alv_number;
                break;

            default:
                option_needs_file(arglex_token_path_prefix_remove, usage);
                break;
            }
            break;

	case arglex_token_exclude:
	    if (arglex() != arglex_token_string)
		option_needs_file(arglex_token_exclude, usage);
	    exclude_patterns.push_back_unique(arglex_value.alv_string);
	    break;

	case arglex_token_exclude_auto_tools:
	    if (exclude_auto_tools >= 0)
		duplicate_option(usage);
	    exclude_auto_tools = 1;
	    break;

	case arglex_token_exclude_auto_tools_not:
	    if (exclude_auto_tools >= 0)
		duplicate_option(usage);
	    exclude_auto_tools = 0;
	    break;

	case arglex_token_exclude_cvs:
	    if (exclude_cvs >= 0)
		duplicate_option(usage);
	    exclude_cvs = 1;
	    break;

	case arglex_token_exclude_cvs_not:
	    if (exclude_cvs >= 0)
		duplicate_option(usage);
	    exclude_cvs = 0;
	    break;
	}
	arglex();
    }

    //
    // Open the tape archive file.
    //
    os_become_orig();
    input ifp = input_file_open(ifn);
    ifp = input_gunzip_open(ifp);
    ifp = input_bunzip2_open(ifp);
    input_tar *tar_p = new input_tar(ifp);
    os_become_undo();

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    pp = project_alloc(project_name);
    pp->bind_existing();

    //
    // Look to see if there are any relevant attributes in the
    // aegis.conf file's project_specific field.
    //
    if (exclude_auto_tools < 0)
        pp->attribute_get_boolean("aetar:exclude-auto-tools");
    if (exclude_cvs < 0)
        pp->attribute_get_boolean("aetar:exclude-cvs");

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
    {
        output::pointer ofp = output_file::text_open(attribute_file_name);
        ofp->fprintf
        (
            "brief_description = \"%s\";\n"
            "description = \"This change was extracted from a tarball.\";\n"
            "cause = external_bug;\n",
            ifn.length() > 0 ? ifn.c_str() : "none"
        );
    }

    nstring trace_options(trace_args());
    dot = os_curdir();
    s =
	str_format
	(
	    "aegis --new-change %ld --project=%s --file=%s%s --verbose",
	    magic_zero_decode(change_number),
	    project_name->str_text,
	    attribute_file_name->str_text,
            trace_options.c_str()
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
	    "aegis --develop-begin %ld --project %s --verbose%s%s",
	    magic_zero_decode(change_number),
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
    nstring dd(change_development_directory_get(cp, 0));
    int umask = change_umask(cp);
    get_exclude_attribute(cp, exclude_patterns);
    change_free(cp);
    cp = 0;
    nstring_list files_created;
    nstring_list files_modified;
    nstring_list file_manifest;

    os_chdir(dd);

    //
    // Now extract each file from the tar archive.
    //
    // We don't know in advance what the file names will be, because
    // that would require two passes.
    //
    for (;;)
    {
	//
	// Find the next file in the archive.
	//
	nstring filename;
        bool executable = false;
	os_become_orig();
	input ip = tar_p->child(filename, executable);
	os_become_undo();
	if (!ip.is_open())
	    break;

	//
	// Mangle the file name if necessary.
	//
	mangle(filename);
        trace_string(filename.c_str());
	if (filename.gmatch(exclude_patterns))
	    continue;
	file_manifest.push_back(filename);

	//
	// Work out if the file exists in the project.
	// This is how we decide to copy or create.
	//
	fstate_src_ty *src_data =
	    project_file_find(pp, filename.get_ref(), view_path_extreme);

	//
	// Build the command to be run.
	//
	if (src_data)
	{
            //
            // Skip build files, we cannot copy them into the change
            //
            switch (src_data->usage)
            {
            case file_usage_build:
                os_become_orig();
                os_unlink_errok(src_data->file_name);
                os_become_undo();
                break;

            case file_usage_source:
            case file_usage_config:
            case file_usage_manual_test:
            case file_usage_test:
#ifndef DEBUG
            default:
#endif
                //
                // If the project is configured to copy or (sym)link
                // the sources from the baseline, we need to unlink
                // the source to prevent a permission denied error.
                //
                os_become_orig();
                os_unlink_errok(src_data->file_name);
                os_become_undo();
                files_modified.push_back(filename);
                break;
            }
	}
	else
	{
            files_created.push_back(filename);
	}

        //
	// Now copy the file into the project.
	//
        os_become_orig();
        os_mkdir_between(dd, filename, 02755 & ~umask);
        {
            output::pointer ofp = output_file::binary_open(filename);
            ofp << ip;
        }
	ip.close();
        int mode = 0666;
        if (executable)
            mode |= 0111;
        os_chmod(filename, mode & ~umask);
	os_become_undo();
    }
    delete tar_p;

    if (exclude_cvs)
    {
        //
        // Build the list of patterns of file names to be rejected.
        //
        exclude_patterns.push_back("Attic/*");
        exclude_patterns.push_back("CVS/*");
        exclude_patterns.push_back(".cvsignore");
        exclude_patterns.push_back("*/Attic/*");
        exclude_patterns.push_back("*/CVS/*");
        exclude_patterns.push_back("*/.cvsignore");
        for (size_t n = 0; n < file_manifest.size(); ++n)
        {
            nstring fn = file_manifest[n];
            if (fn == ".cvsignore" || fn.gmatch("*/.cvsignore"))
            {
                read_cvs_ignore(fn, exclude_patterns);
            }
        }
    }

    //
    // Now we have to see if any of the files in the file manifest need
    // to be excluded, because they are derived files from the various
    // GNU auto tools.
    //
    if (exclude_auto_tools > 0)
    {
	if
       	(
	    file_manifest.member("configure.ac")
	||
	    file_manifest.member("configure.in")
	)
	{
	    const char *table[] =
	    {
		"aclocal.m4",
		// "acinclude.m4",
		"autom4te.cache",
		"config.guess",
		"config.h",
		"config.h.in",
		"config.log",
		"config.status",
		"config.sub",
		"configure",
		"install-sh",
                // "ltmain.h",
		"stamp-h",
		"stamp-h.in",
	    };

	    for (const char **tp = table; tp < ENDOF(table); ++tp)
	    {
		nstring fn = *tp;
                exclude_patterns.push_back(fn);
                exclude_patterns.push_back("*/" + fn);
	    }
	}
	if (file_manifest.member("Makefile.am"))
	{
	    const char *table[] =
	    {
		"install-sh",
		"Makefile",
		"Makefile.in",
		"missing",
		"mkinstalldirs",
		"stamp-h",
		"stamp-h.in",
	    };

	    for (const char **tp = table; tp < ENDOF(table); ++tp)
	    {
		nstring fn = *tp;
		exclude_patterns.push_back(fn);
		exclude_patterns.push_back("*/" + fn);
	    }
	}
    }

    //
    // Cull the file name lists
    //
    os_become_orig();
    remove_by_pattern(file_manifest, exclude_patterns);
    remove_by_pattern(files_created, exclude_patterns);
    remove_by_pattern(files_modified, exclude_patterns);
    os_become_undo();

    //
    // See if we need to remove any of the excluded files.
    //
    nstring_list files_removed;
    for (size_t j = 0; ; ++j)
    {
        fstate_src_ty *src = pp->file_nth(j, view_path_extreme);
        if (!src)
            break;
        nstring fn(src->file_name);
        if (fn.gmatch(exclude_patterns))
        {
	    files_removed.push_back(fn);

            os_become_orig();
            os_unlink_errok(fn);
            os_become_undo();
        }
    }
    if (!files_removed.empty())
    {
        nstring cmd =
            nstring::format
            (
                "aegis --remove-file --project=%s --change=%ld%s --verbose",
                project_name->str_text,
		magic_zero_decode(change_number),
                trace_options.c_str()
            );
        os_xargs(cmd, files_removed, dd);
    }

    //
    // Now create the new files.
    //
    if (!files_created.empty())
    {
        nstring cmd =
            nstring::format
            (
                "aegis --new-file --no-template --project=%s --change=%ld%s "
		    "--keep --verbose",
                project_name->str_text,
		magic_zero_decode(change_number),
                trace_options.c_str()
            );
        os_xargs(cmd, files_created, dd);
    }

    //
    // Now copy the modified files.
    //
    if (!files_modified.empty())
    {
        nstring delta_opt;
        if (delta)
            delta_opt = nstring(" --delta=") + delta;

        nstring cmd =
            nstring::format
            (
                "aegis --copy-file --project=%s --change=%ld%s%s "
		    "--keep --verbose",
                project_name->str_text,
		magic_zero_decode(change_number),
                delta_opt.c_str(),
                trace_options.c_str()
            );
        os_xargs(cmd, files_modified, dd);

        cmd =
            nstring::format
            (
                "aegis --copy-file-undo --unchanged --project=%s "
		    "--change=%ld%s --verbose",
                project_name->str_text,
		magic_zero_decode(change_number),
                trace_options.c_str()
            );
	os_become_orig();
	os_execute(cmd, OS_EXEC_FLAG_NO_INPUT, dd);
	os_become_undo();
    }

    project_free(pp);
    pp = 0;
    trace(("}\n"));
}
