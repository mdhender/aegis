/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate opens
 */

#include <header.h>
#include <input/base64.h>
#include <input/cpio.h>
#include <input/file.h>
#include <input/gunzip.h>
#include <open.h>
#include <os.h>
#include <str.h>


input_ty *
aedist_open(ifn, subject_p)
	const char	*ifn;
	string_ty	**subject_p;
{
	input_ty	*ifp;
	input_ty	*cpio_p;
	header_ty	*hp;
	string_ty	*s;

	/*
	 * Open the input file and verify the format.
	 */
	os_become_orig();
	ifp = input_file_open(ifn);
	hp = header_read(ifp);
	s = header_query(hp, "mime-version");
	if (s)
	{
		string_ty	*content_type;

		s = header_query(hp, "content-type");
		content_type = str_from_c("application/aegis-change-set");
		if (!s || !str_equal(s, content_type))
			input_format_error(ifp);
		str_free(content_type);
	}

	/*
	 * Deal with the content encoding.
	 */
	s = header_query(hp, "content-transfer-encoding");
	if (s)
	{
		string_ty	*base64;

		/*
		 * We could cope with other encodings here,
		 * if we ever need to.
		 */
		base64 = str_from_c("base64");
		if (!str_equal(s, base64))
			input_format_error(ifp);
		str_free(base64);

		/*
		 * Some sort of start line could be nice.  This could
		 * allow us to add comments and stuff (the long
		 * description?)  before the actual content.
		 *
		 * Search for such a thing here, if and when we add that.
		 * Use one of the base64-illegal characters to start
		 * the line.
		 */

		/*
		 * The rest of the input is in base64 encoding.
		 */
		ifp = input_base64(ifp);
	}

	/*
	 * The contents we are interested in are 
	 * a gzipped cpio archive.
	 */
	ifp = input_gunzip(ifp);
	cpio_p = input_cpio(ifp);
	os_become_undo();

	/*
	 * Set the subject if they are interested.
	 */
	if (subject_p)
	{
		s = header_query(hp, "subject");
		if (s && s->str_length)
			*subject_p = str_copy(s);
		else
			*subject_p = str_from_c("No Subject");
	}

	/*
	 * clean up and go home
	 */
	header_delete(hp);
	return cpio_p;
}
