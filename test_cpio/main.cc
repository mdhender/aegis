//
//	aegis - project change supervisor
//	Copyright (C) 2005 Walter Franzini;
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
// MANIFEST: implementation of the main class
//

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex.h>
#include <arglex2.h>
#include <error.h> // for assert
#include <help.h>
#include <input.h>
#include <input/base64.h>
#include <input/cpio.h>
#include <input/file.h>
#include <input/gunzip.h>
#include <input/uudecode.h>
#include <nstring.h>
#include <nstring/list.h>
#include <os.h>
#include <output.h>
#include <output/cpio.h>
#include <output/file.h>
#include <progname.h>
#include <r250.h>
#include <rfc822header.h>
#include <sub.h>
#include <trace.h>
#include <uuidentifier.h>

enum
{
    arglex_token_extract,
    arglex_token_create,
};

static arglex_table_ty argtab[] =
{
    { "-EXTract", arglex_token_extract },
    { "-CREate", arglex_token_create },
    { "-File", arglex_token_file },
    { "-List", arglex_token_list },
    { "-Change_Directory", arglex_token_change_directory},
    ARGLEX_END_MARKER
};


static input_cpio *
input_cpio_mime(input_ty *ifp)
{
    rfc822_header_ty *hp = rfc822_header_read(ifp);

    nstring s(rfc822_header_query(hp, "mime-version"));
    if (!s.empty())
    {
        nstring content_type("application/aegis-change-set");
        s = nstring(rfc822_header_query(hp, "content-type"));
        if (s.empty() || s != content_type)
            ifp->fatal_error("wrong content type");
    }

    //
    // Deal with the content encoding.
    //
    s = nstring(rfc822_header_query(hp, "content-transfer-encoding"));
    if (!s.empty())
    {
        static nstring base64("base64");
        static nstring uuencode("uuencode");

        //
        // We could cope with other encodings here,
        // if we ever need to.
        //
        if (s == base64)
        {
            //
            // The rest of the input is in base64 encoding.
            //
            ifp = new input_base64(ifp, true);
        }
        else if (s == uuencode)
        {
            //
            // The rest of the input is uuencoded.
            //
            ifp = new input_uudecode(ifp, true);
        }
        else
        {
            sub_context_ty sc;
            sc.var_set_string("Name", s);
            string_ty *tmp =
                sc.subst_intl(i18n("content transfer encoding $name unknown"));
            ifp->fatal_error(tmp->str_text);
            str_free(tmp);
        }
    }

    //
    // The contents we are interested in are
    // a gzipped cpio archive.
    //
    ifp = input_gunzip_open(ifp);
    input_cpio *cpio_p = new input_cpio(ifp);

    rfc822_header_delete(hp);
    return cpio_p;
}


static int
cpio_create(const nstring &cwd, const nstring &archive_name,
    const nstring_list &file_list)
{
    if (archive_name.empty())
        fatal_raw("empty archive name");

    if (file_list.empty())
        fatal_raw("empty list");

    for (size_t n = 0; n < file_list.size(); ++n)
    {
        nstring ifn = file_list.get(n);
        nstring abs_path = os_path_join(cwd, ifn);
        if (ifn.empty())
            fatal_raw("empty file name");
        os_become_orig();
        if (!os_exists(abs_path) || os_isa_special_file(abs_path.get_ref()))
            fatal_raw("bad file: %s", ifn.c_str());
        os_become_undo();
    }

    os_become_orig();
    int archive_exists = os_exists(archive_name);
    os_become_undo();

    if (archive_exists)
        fatal_raw("target already exists: %s", archive_name.c_str());

    output_ty   *ofp;
    os_become_orig();
    ofp = output_file_binary_open(archive_name.get_ref());
    output_cpio_ty *cpio_p = new output_cpio_ty(ofp);
    input_ty *ifp;
    for (size_t n = 0; n < file_list.size(); ++n)
    {
        nstring abs_path = os_path_join(cwd, file_list[n]);

        trace_nstring(file_list[n]);
        ifp = input_file_open(abs_path);
        int len = ifp->length();
        ofp = cpio_p->child(file_list[n], len);
        input_to_output(ifp, ofp);
        delete ifp;
        delete ofp;
    }
    delete cpio_p;
    os_become_undo();
    return 0;
}


static int
cpio_list(const nstring &, const nstring &archive, const nstring_list &)
{
    os_become_orig();
    input_ty *ifp = input_file_open(archive);
    input_cpio *cpio_p = input_cpio_mime(ifp);
    for (;;)
    {
	nstring ofn;
	ifp = cpio_p->child(ofn);
	if (!ifp)
	    break;
        printf("%s\n", ofn.c_str());
	char buff[4096];
        while (ifp->read(buff, sizeof(buff)))
	    ;
        delete ifp;
    }
    delete cpio_p;
    os_become_undo();
    return 0;
}


static int
cpio_extract(const nstring& root, const nstring& archive, const nstring_list&)
{
    os_become_orig();
    input_ty *ifp = input_file_open(archive);
    input_cpio *cpio_p = input_cpio_mime(ifp);
    for (;;)
    {
	nstring ofn;
	ifp = cpio_p->child(ofn);
	if (!ifp)
	    break;
        nstring abs_path = os_path_join(root, ofn);
        os_mkdir_between(root, ofn, 0755);
        output_ty *ofp = output_file_open(abs_path);
        input_to_output(ifp, ofp);
        delete ofp;
        delete ifp;
    }
    delete cpio_p;
    os_become_undo();
    return 0;
}


static void
usage(void)
{
    exit(EXIT_FAILURE);
}


//
// test_cpio [ -extract | -create infile1 infile2 ... ]
//
// Extract operation:
//
//      * the archive is read from stdin
//      * the file are extracted in the current directory
//      * if needed directory will be created
//
//      % test_cpio -extract -file archive.cpio
//
// Create operation:
//
//      * the archive will be written to stdout;
//      * the file to add to the archive are taken from the command file;
//
//      % test_cpio -create -file archive.cpio file1 file2 ....
//
int
main(int argc, char **argv)
{
    nstring      directory;
    nstring      archive;
    nstring_list ifn;
    nstring      cd;
    int          (*func) (const nstring&, const nstring&, const nstring_list&);

    arglex_init(argc, argv, argtab);
    r250_init();
    arglex();
    os_become_init_mortal();
    func = 0;

    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_file:
            arglex();
            if (arglex_token != arglex_token_string)
                option_needs_string(arglex_token_file, usage);
            archive = arglex_value.alv_string;
            trace_nstring(archive);
            break;

        case arglex_token_change_directory:
            arglex();
            if (arglex_token != arglex_token_string)
                option_needs_string(arglex_token_file, usage);
            cd = arglex_value.alv_string;
            break;

        case arglex_token_string:
            ifn.push_back(arglex_value.alv_string);
            break;

        case arglex_token_extract:
            func = cpio_extract;
            break;

        case arglex_token_create:
            func = cpio_create;
            break;

        case arglex_token_list:
            func = cpio_list;
            break;
        }
        arglex();
    }

    trace_nstring(archive);

    int ret = 1;

    if (cd.empty())
        cd = ".";
    if (func)
        ret = func(cd, archive, ifn);
    else
        usage();

    exit(ret ? EXIT_FAILURE : EXIT_SUCCESS);
}
