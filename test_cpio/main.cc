//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
//	Copyright (C) 2005, 2007 Walter Franzini
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/arglex.h>
#include <common/error.h> // for assert
#include <common/nstring.h>
#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <libaegis/arglex2.h>
#include <libaegis/help.h>
#include <libaegis/input/base64.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/cpio.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/input.h>
#include <libaegis/input/uudecode.h>
#include <libaegis/os.h>
#include <libaegis/output/bit_bucket.h>
#include <libaegis/output/cpio.h>
#include <libaegis/output/file.h>
#include <libaegis/output.h>
#include <libaegis/rfc822.h>
#include <libaegis/sub.h>

enum
{
    arglex_token_extract,
    arglex_token_create
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
input_cpio_mime(input &ifp)
{
    rfc822 hdr;
    hdr.load(ifp, true);

    nstring s = hdr.get("mime-version");
    if (!s.empty())
    {
	s = hdr.get("content-type");
	if (s != "application/aegis-change-set")
            ifp->fatal_error("wrong content type");
    }

    //
    // Deal with the content encoding.
    //
    s = hdr.get("content-transfer-encoding");
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
            ifp = new input_base64(ifp);
        }
        else if (s == uuencode)
        {
            //
            // The rest of the input is uuencoded.
            //
            ifp = new input_uudecode(ifp);
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
    ifp = input_bunzip2_open(ifp);
    input_cpio *cpio_p = new input_cpio(ifp);
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

    os_become_orig();
    output::pointer ofp = output_file::binary_open(archive_name);
    output_cpio *cpio_p = new output_cpio(ofp, (time_t)0);
    for (size_t n = 0; n < file_list.size(); ++n)
    {
        nstring abs_path = os_path_join(cwd, file_list[n]);

        trace_nstring(file_list[n]);
        input ifp = input_file_open(abs_path);
        int len = ifp->length();
        output::pointer os = cpio_p->child(file_list[n], len);
        os << ifp;
    }
    delete cpio_p;
    os_become_undo();
    return 0;
}


static int
cpio_list(const nstring &, const nstring &archive, const nstring_list &)
{
    os_become_orig();
    input ifp = input_file_open(archive);
    input_cpio *cpio_p = input_cpio_mime(ifp);
    for (;;)
    {
	nstring ofn;
	input ifp2 = cpio_p->child(ofn);
	if (!ifp2.is_open())
	    break;
        printf("%s\n", ofn.c_str());
	output::pointer nowhere = output_bit_bucket::create();
	nowhere << ifp2;
    }
    delete cpio_p;
    os_become_undo();
    return 0;
}


static int
cpio_extract(const nstring& root, const nstring& archive, const nstring_list&)
{
    os_become_orig();
    input ifp = input_file_open(archive);
    input_cpio *cpio_p = input_cpio_mime(ifp);
    for (;;)
    {
	nstring ofn;
	input ifp2 = cpio_p->child(ofn);
	if (!ifp2.is_open())
	    break;
        nstring abs_path = os_path_join(root, ofn);
        os_mkdir_between(root, ofn, 0755);
        output::pointer ofp = output_file::open(abs_path);
        ofp << ifp2;
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
