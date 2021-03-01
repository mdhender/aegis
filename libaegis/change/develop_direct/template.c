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
 * MANIFEST: functions to manipulate templates
 */

#include <change.h>
#include <error.h> /* for assert */
#include <os.h>
#include <sub.h>
#include <user.h>


string_ty *
change_development_directory_template(cp, up)
	change_ty	*cp;
	user_ty		*up;
{
	pconf		pconf_data;
	unsigned long	k;
	string_ty	*ddd;
	string_ty	*devdir;

	/*
	 * Get the project config file.  Don't insist that it exists,
	 * because it will not for the first change.
	 */
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	assert(pconf_data->new_test_filename);

	/*
	 * If the user did not give the directory to use,
	 * we must construct one.
	 * The length is limited by the available filename
	 * length limit, trim the project name if necessary.
	 */
	ddd = user_default_development_directory(up);
	assert(ddd);

	devdir = 0;
	for (k = 0;; ++k)
	{
		sub_context_ty	*scp;
		char		suffix[30];
		char		*tp;
		unsigned long	n;
		int		exists;

		/*
		 * Construct the magic string.  This is used to
		 * construct unique file names, should there be
		 * a conflict for some reason (usually to do
		 * with truncating file names, or re-using file
		 * names).
		 */
		scp = sub_context_new();
		tp = suffix;
		n = k;
		for (;;)
		{
			*tp++ = (n & 15) + 'C';
			n >>= 4;
			if (!n)
				break;
		}
		*tp = 0;
		sub_var_set(scp, "Magic", "%s", suffix);

		/*
		 * The default development directory is
		 * directory within which the new development
		 * directory is to be placed.  It is needed by
		 * the substitution if it is to crop file names
		 * by the maximum filename length (see the
		 * ${namemax} substitution).
		 */
		sub_var_set(scp, "Default_Development_Directory", "%S", ddd);

		/*
		 * Perform the substitution to construct the
		 * development directory name.
		 */
		devdir =
			substitute
			(
				scp,
				cp,
				pconf_data->development_directory_template
			);
		sub_context_delete(scp);

		/*
		 * See if this path is unique.
		 */
		user_become(up);
		exists = os_exists(devdir);
		user_become_undo();
		if (!exists)
			break;
		str_free(devdir);
	}
	str_free(ddd);
	assert(devdir);
	return devdir;
}
