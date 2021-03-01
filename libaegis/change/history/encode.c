/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate encodes
 */

#include <ac/ctype.h>

#include <change.h>
#include <change/file.h>
#include <change/history/encode.h>
#include <commit.h>
#include <input/file.h>
#include <os.h>
#include <output/base64.h>
#include <output/file.h>
#include <output/quoted_print.h>
#include <output/tee.h>
#include <trace.h>
#include <undo.h>


static string_ty *dir_and_base _((string_ty *, string_ty *));

static string_ty *
dir_and_base(dir, other)
	string_ty	*dir;
	string_ty	*other;
{
	string_ty	*base;
	string_ty	*result;

	base = os_entryname(other);
	result = os_path_cat(dir, base);
	str_free(base);
	return result;
}


string_ty *
change_history_encode(cp, src)
	change_ty	*cp;
	fstate_src	src;
{
	pconf		pconf_data;
	int		min_qp_enc;
	string_ty	*ofn1;
	input_ty	*ip;
	output_ty	*op1;
	output_ty	*op1x;
	string_ty	*ofn2;
	output_ty	*op2;
	output_ty	*op2x;
	output_ty	*op;
	string_ty	*filename;
	int		last_was_newline;
	int		ascii_yuck;
	int		intl_yuck;
	int		encoding_required;
	long		size1;
	long		size2;
	static string_ty *b64_dir;
	static string_ty *qp_dir;

	trace(("change_history_encode(cp = %08lX, \
src->file_name = \"%s\")\n{\n", (long)cp, src->file_name->str_text));
	if (!src->edit)
		src->edit = history_version_type.alloc();
	src->edit->encoding = history_version_encoding_none;
	pconf_data = change_pconf_get(cp, 1);
	min_qp_enc = 0;
	switch (pconf_data->history_content_limitation)
	{
	case pconf_history_content_limitation_binary_capable:
		trace(("}\n"));
		return change_file_path(cp, src->file_name);

	case pconf_history_content_limitation_international_text:
		min_qp_enc = 1;
		break;

	case pconf_history_content_limitation_ascii_text:
		break;
	}

	/*
	 * Some history tools are rather dumb, and require the last
	 * pathname portion of the input file and the history file
	 * to match.  E.g. blah/fred and blahblah/fred,v for RCS.
	 * And, no, the GNU versions aren't any smarter.
	 *
	 * To accomplish this for encoded files, we have to make a
	 * directory to plonk them into, one for each encoding we want
	 * to compare.  Sigh.
	 */
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
	change_become_undo();


	filename = change_file_path(cp, src->file_name);

	change_become(cp);
	ofn1 = dir_and_base(qp_dir, filename);
	ofn2 = dir_and_base(b64_dir, filename);

	ip = input_file_open(filename);

	op1 = output_file_text_open(ofn1);
	op1x = output_quoted_printable(op1, 1, min_qp_enc);

	op2 = output_file_text_open(ofn2);
	op2x = output_base64(op2, 1);
	op = output_tee(op1x, 1, op2x, 1);

	last_was_newline = 1;
	ascii_yuck = 0;
	intl_yuck = 0;
	for (;;)
	{
		char	buffer[1 << 14];
		int	nbytes;
		char	*pos;
		char	*end;

		nbytes = input_read(ip, buffer, sizeof(buffer));
		if (nbytes == 0)
			break;
		output_write(op, buffer, nbytes);

		pos = buffer;
		end = buffer + nbytes;
		while (pos < end)
		{
			unsigned char c = *pos++;
			/* C locale */
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
	input_delete(ip);
	output_delete(op);
	change_become_undo();

	encoding_required = 1;
	switch (pconf_data->history_content_limitation)
	{
	case pconf_history_content_limitation_binary_capable:
		/* unreachable */
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
		change_become_undo();
		trace(("}\n"));
		return filename;
	}
	str_free(filename);

	/*
	 * Encoding is required.
	 * Use the smallest encoding of the file.
	 */
	change_become(cp);
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
	change_become_undo();
	trace(("}\n"));
	return filename;
}
