//
//	aegis - project change supervisor
//	Copyright (C) 2001-2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//
// MANIFEST: functions to manipulate encodes
//

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/change/history/encode.h>
#include <libaegis/commit.h>
#include <common/error.h> // for assert
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/output/base64.h>
#include <libaegis/output/file.h>
#include <libaegis/output/quoted_print.h>
#include <libaegis/output/tee.h>
#include <libaegis/project/history/uuid_trans.h>
#include <common/trace.h>
#include <libaegis/undo.h>


static string_ty *
dir_and_base(string_ty *dir, string_ty *other)
{
    const char      *base;

    base = strrchr(other->str_text, '/');
    if (base)
	++base;
    else
	base = other->str_text;
    return str_format("%s/%s", dir->str_text, base);
}


string_ty *
change_history_encode(change::pointer cp, fstate_src_ty *src, int *unlink_p)
{
    pconf_ty        *pconf_data;
    int             min_qp_enc;
    string_ty       *ofn1;
    output_ty       *op1;
    output_ty       *op1x;
    string_ty       *ofn2;
    output_ty       *op2;
    output_ty       *op2x;
    output_ty       *op;
    string_ty       *filename;
    int             last_was_newline;
    int             ascii_yuck;
    int             intl_yuck;
    int             encoding_required;
    long            size1;
    long            size2;
    static string_ty *b64_dir;
    static string_ty *qp_dir;
    static string_ty *nenc_dir;
    string_ty       *ofn3;

    trace(("change_history_encode(cp = %08lX, src->file_name = \"%s\")\n{\n",
	(long)cp, src->file_name->str_text));
    *unlink_p = 1;
    if (!src->edit)
    {
	src->edit = (history_version_ty *)history_version_type.alloc();
    }
    src->edit->encoding = history_version_encoding_none;
    pconf_data = change_pconf_get(cp, 1);
    min_qp_enc = 0;
    switch (pconf_data->history_content_limitation)
    {
    case pconf_history_content_limitation_binary_capable:
	if (src->uuid)
	{
	    string_ty       *relname;

	    //
            // See comment below.  Some history tools need basename of
            // the two files to be the same.  If there is a UUID they
            // will definitely not match.
	    //
	    if (!nenc_dir)
	    {
		nenc_dir = os_edit_filename(0);
		change_become(cp);
		os_mkdir(nenc_dir, 0755);
		undo_rmdir_bg(nenc_dir);
		commit_rmdir_tree_errok(nenc_dir);
		change_become_undo(cp);
	    }
	    relname = project_history_uuid_translate(src);
	    ofn3 = dir_and_base(nenc_dir, relname);
	    str_free(relname);

	    filename = change_file_path(cp, src->file_name);
	    change_become(cp);
	    os_symlink_or_copy(filename, ofn3);
	    change_become_undo(cp);
	    str_free(filename);

	    trace(("return \"%s\"\n", ofn3->str_text));
	    trace(("}\n"));
	    return ofn3;
	}
	*unlink_p = 0;
	trace(("}\n"));
	return change_file_path(cp, src->file_name);

    case pconf_history_content_limitation_international_text:
	min_qp_enc = 1;
	break;

    case pconf_history_content_limitation_ascii_text:
	break;
    }

    //
    // Some history tools are rather dumb, and require the last
    // pathname portion of the input file and the history file
    // to match.  E.g. blah/fred and blahblah/fred,v for RCS.
    // And, no, the GNU versions aren't any smarter.
    //
    // To accomplish this for encoded files, we have to make a
    // directory to plonk them into, one for each encoding we want
    // to compare.  Sigh.
    //
    change_become(cp);
    if (!b64_dir)
    {
	b64_dir = os_edit_filename(0);
	os_mkdir(b64_dir, 0755);
	undo_rmdir_bg(b64_dir);
	commit_rmdir_tree_errok(b64_dir);
    }
    if (!qp_dir)
    {
	qp_dir = os_edit_filename(0);
	os_mkdir(qp_dir, 0755);
	undo_rmdir_bg(qp_dir);
	commit_rmdir_tree_errok(qp_dir);
    }
    if (!nenc_dir)
    {
	nenc_dir = os_edit_filename(0);
	os_mkdir(nenc_dir, 0755);
	undo_rmdir_bg(nenc_dir);
	commit_rmdir_tree_errok(nenc_dir);
    }
    change_become_undo(cp);

    filename = change_file_path(cp, src->file_name);

    if (src->uuid)
    {
	string_ty       *relname;

	relname = project_history_uuid_translate(src);
	ofn1 = dir_and_base(qp_dir, relname);
	ofn2 = dir_and_base(b64_dir, relname);
	ofn3 = dir_and_base(nenc_dir, relname);
	str_free(relname);
    }
    else
    {
	ofn1 = dir_and_base(qp_dir, filename);
	ofn2 = dir_and_base(b64_dir, filename);
	ofn3 = str_copy(filename);
    }

    change_become(cp);
    if (src->uuid)
    {
	assert(!str_equal(filename, ofn3));
	os_symlink_or_copy(filename, ofn3);
    }

    input ip = input_file_open(filename);
    str_free(filename);

    op1 = output_file_text_open(ofn1);
    op1x = new output_quoted_printable_ty(op1, true, min_qp_enc);

    op2 = output_file_text_open(ofn2);
    op2x = new output_base64_ty(op2, true);
    op = new output_tee_ty(op1x, true, op2x, true);

    last_was_newline = 1;
    ascii_yuck = 0;
    intl_yuck = 0;
    for (;;)
    {
	char            buffer[1 << 14];
	int             nbytes;
	char            *pos;
	char            *end;

	nbytes = ip->read(buffer, sizeof(buffer));
	if (nbytes == 0)
	    break;
	op->write(buffer, nbytes);

	pos = buffer;
	end = buffer + nbytes;
	while (pos < end)
	{
	    unsigned char   c;

	    c = *pos++;
	    // C locale
	    switch (c)
	    {
	    case 0:
		intl_yuck = 1;
		ascii_yuck = 1;
		break;

	    case ' ':
	    case '\f':
	    case '\n':
	    case '\r':
	    case '\t':
		break;

	    default:
		if (!isprint(c))
	    	    ascii_yuck = 1;
		break;
	    }
	}
	last_was_newline = (buffer[nbytes - 1] == '\n');
    }
    if (!last_was_newline)
    {
	ascii_yuck = 1;
	intl_yuck = 1;
    }
    ip.close();
    delete op;
    op = 0;
    change_become_undo(cp);

    encoding_required = 1;
    switch (pconf_data->history_content_limitation)
    {
    case pconf_history_content_limitation_binary_capable:
	// unreachable
	encoding_required = 0;
	break;

    case pconf_history_content_limitation_ascii_text:
	encoding_required = ascii_yuck;
	break;

    case pconf_history_content_limitation_international_text:
	encoding_required = intl_yuck;
	break;
    }
    if (!encoding_required)
    {
	change_become(cp);
	os_unlink(ofn1);
	str_free(ofn1);
	os_unlink(ofn2);
	str_free(ofn2);
	change_become_undo(cp);
	*unlink_p = !str_equal(ofn3, filename);
	trace(("}\n"));
	return ofn3;
    }

    //
    // Encoding is required.
    // Use the smallest encoding of the file.
    //
    change_become(cp);
    if (src->uuid)
	os_unlink(ofn3);
    str_free(ofn3);
    size1 = os_file_size(ofn1);
    size2 = os_file_size(ofn2);
    if (size1 <= size2)
    {
	src->edit->encoding = history_version_encoding_quoted_printable;
	filename = ofn1;
	os_unlink(ofn2);
	str_free(ofn2);
    }
    else
    {
	src->edit->encoding = history_version_encoding_base64;
	os_unlink(ofn1);
	str_free(ofn1);
	filename = ofn2;
    }
    change_become_undo(cp);
    trace(("}\n"));
    return filename;
}
