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
 * MANIFEST: functions to manipulate slurps
 */

#include <ac/stdlib.h>

#include <error.h>
#include <input.h>
#include <input/file.h>
#include <input/base64.h>
#include <input/gunzip.h>
#include <input/uudecode.h>
#include <os.h>
#include <rfc822header.h>
#include <slurp.h>
#include <sub.h>
#include <zero.h>


patch_list_ty *
patch_slurp(ifn)
	string_ty	*ifn;
{
	input_ty	*ifp;
	rfc822_header_ty *hp;
	string_ty	*s;
	patch_list_ty	*plp;

	/*
	 * open the input
	 */
	os_become_orig();
	ifp = input_file_open(ifn);

	/*
	 * Look for an RFC822 header.
	 *
	 * It could tell us useful things, like the content transfer
	 * encoding, and the project name.
	 */
	hp = rfc822_header_read(ifp);

	/*
	 * Deal with the content encoding.
	 */
	s = rfc822_header_query(hp, "content-transfer-encoding");
	if (s)
	{
		static string_ty *base64;
		static string_ty *uuencode;

		/*
		 * We could cope with other encodings here,
		 * if we ever need to.
		 */
		if (!base64)
			base64 = str_from_c("base64");
		if (!uuencode)
			uuencode = str_from_c("uuencode");
		if (str_equal(s, base64))
		{
			/*
			 * The rest of the input is in base64 encoding.
			 */
			ifp = input_base64(ifp, 1);
		}
		else if (str_equal(s, uuencode))
		{
			/*
			 * The rest of the input is uuencoded.
			 */
			ifp = input_uudecode(ifp, 1);
		}
		else
		{
			sub_context_ty	*scp;
			string_ty	*tmp;

			scp = sub_context_new();
			sub_var_set_string(scp, "Name", s);
			tmp =
				subst_intl
				(
					scp,
				 i18n("content transfer encoding $name unknown")
				);
			input_fatal_error(ifp, tmp->str_text);
			str_free(tmp);
			sub_context_delete(scp);
		}

	}

	/*
	 * The contents could be gzipped.
	 */
	ifp = input_gunzip(ifp);

	/*
	 * Read the patch body.
	 */
	plp = patch_read(ifp);
	os_become_undo();
	assert(plp);

	/*
	 * Pull useful information out of the patch header.
	 */
	s = rfc822_header_query(hp, "x-aegis-project-name");
	if (s && s->str_length)
		plp->project_name = str_copy(s);

	s = rfc822_header_query(hp, "x-aegis-change-number");
	if (s && s->str_length)
	{
		char		*endptr;
		long		n;

		n = strtol(s->str_text, &endptr, 0);
		if (!*endptr && n >= 0)
			plp->change_number = magic_zero_encode(n);
	}

	s = rfc822_header_query(hp, "subject");
	if (s && s->str_length)
		plp->brief_description = str_copy(s);

	s = rfc822_header_query(hp, "date");
	if (s && s->str_length)
	{
		if (!plp->description)
			plp->description = str_from_c("No description.");
		s = str_format("Date: %S\n%S", s, plp->description);
		str_free(plp->description);
		plp->description = s;
	}

	s = rfc822_header_query(hp, "from");
	if (s && s->str_length)
	{
		if (!plp->description)
			plp->description = str_from_c("No description.");
		s = str_format("From: %S\n%S", s, plp->description);
		str_free(plp->description);
		plp->description = s;
	}

	/*
	 * finished with the header
	 */
	rfc822_header_delete(hp);
	hp = 0;

	/*
	 * all done
	 */
	return plp;
}
