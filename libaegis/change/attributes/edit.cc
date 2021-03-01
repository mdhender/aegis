//
//	aegis - project change supervisor
//	Copyright (C) 2000-2005 Peter Miller;
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
// MANIFEST: functions to manipulate edits
//

#include <common/error.h>
#include <common/language.h>
#include <libaegis/cattr.h>
#include <libaegis/change/attributes.h>
#include <libaegis/commit.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>


void
change_attributes_edit(cattr_ty **dp, int et)
{
	sub_context_ty	*scp;
	cattr_ty	*d;
	string_ty	*filename;
	string_ty	*msg;

	//
	// write attributes to temporary file
	//
	d = *dp;
	assert(d);
	filename = os_edit_filename(1);
	os_become_orig();
	language_human();
	cattr_write_file(filename, d, 0);
	language_C();
	cattr_type.free(d);

	//
	// an error message to issue if anything goes wrong
	//
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	msg = subst_intl(scp, i18n("attributes in $filename"));
	sub_context_delete(scp);
	undo_message(msg);
	str_free(msg);
	os_become_undo();

	//
	// edit the file
	//
	os_edit(filename, (edit_ty)et);

	//
	// read it in again
	//
	os_become_orig();
	d = cattr_read_file(filename);
	commit_unlink_errok(filename);
	os_become_undo();
	change_attributes_fixup(d);
	change_attributes_verify(filename, d);
	str_free(filename);
	*dp = d;
}
