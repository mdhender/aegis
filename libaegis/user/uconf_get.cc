//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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

#include <common/libdir.h>
#include <common/trace.h>
#include <libaegis/attribute.h>
#include <libaegis/os.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>


/**
  * The merge_uconf function is used internally by
  * user_ty::uconf_get to merge two sets of user
  * configuration parameters.
  *
  * @param curdata
  *     data set that is operated on
  * @param defdata
  *     default data set. used if some fields in curdata is not filled.
  */

static void
merge_uconf(uconf_ty *data, uconf_ty *tmp)
{
    if (!data->default_project_name && tmp->default_project_name)
    {
	data->default_project_name = str_copy(tmp->default_project_name);
    }
    if (!data->default_change_number && tmp->default_change_number)
    {
	data->default_change_number = tmp->default_change_number;
    }
    if
    (
	!data->default_development_directory
    &&
	tmp->default_development_directory
    )
    {
	data->default_development_directory =
	    str_copy(tmp->default_development_directory);
    }
    if (!data->default_project_directory && tmp->default_project_directory)
    {
	data->default_project_directory =
	    str_copy(tmp->default_project_directory);
    }
    if
    (
	!(data->mask & uconf_delete_file_preference_mask)
    &&
	(tmp->mask & uconf_delete_file_preference_mask)
    )
    {
	data->delete_file_preference = tmp->delete_file_preference;
	data->mask |= uconf_delete_file_preference_mask;
    }
    if
    (
	!(data->mask & uconf_pager_preference_mask)
    &&
	(tmp->mask & uconf_pager_preference_mask)
    )
    {
	data->pager_preference = tmp->pager_preference;
	data->mask |= uconf_pager_preference_mask;
    }
    if
    (
	!(data->mask & uconf_persevere_preference_mask)
    &&
	(tmp->mask & uconf_persevere_preference_mask)
    )
    {
	data->persevere_preference = tmp->persevere_preference;
	data->mask |= uconf_persevere_preference_mask;
    }
    if
    (
	!(data->mask & uconf_log_file_preference_mask)
    &&
	(tmp->mask & uconf_log_file_preference_mask)
    )
    {
	data->log_file_preference = tmp->log_file_preference;
	data->mask |= uconf_log_file_preference_mask;
    }
    if
    (
	!(data->mask & uconf_lock_wait_preference_mask)
    &&
	(tmp->mask & uconf_lock_wait_preference_mask)
    )
    {
	data->lock_wait_preference = tmp->lock_wait_preference;
	data->mask |= uconf_lock_wait_preference_mask;
    }
    if (!data->email_address && tmp->email_address)
    {
	data->email_address = str_copy(tmp->email_address);
    }
    if
    (
	!(data->mask & uconf_whiteout_preference_mask)
    &&
	(tmp->mask & uconf_whiteout_preference_mask)
    )
    {
	data->whiteout_preference = tmp->whiteout_preference;
	data->mask |= uconf_whiteout_preference_mask;
    }
    if (!data->editor_command && tmp->editor_command)
    {
	data->editor_command = str_copy(tmp->editor_command);
    }
    if (!data->visual_command && tmp->visual_command)
    {
	data->visual_command = str_copy(tmp->visual_command);
    }
    if (!data->pager_command && tmp->pager_command)
    {
	data->pager_command = str_copy(tmp->pager_command);
    }

    //
    // Merge the attribute lists.
    //
    if (tmp->attribute && tmp->attribute->length)
    {
	if (!data->attribute)
	{
	    data->attribute =
		    (attributes_list_ty *)attributes_list_type.alloc();
	}
	for (size_t j = 0; j < tmp->attribute->length; ++j)
	{
	    attributes_ty *ap = tmp->attribute->list[j];
	    if
	    (
		ap->name
	    &&
		ap->value
	    &&
		!attributes_list_find(data->attribute, ap->name->str_text)
	    )
	    {
		attributes_list_append
	       	(
		    data->attribute,
		    ap->name->str_text,
		    ap->value->str_text
		);
	    }
	}
    }
}


static void
read_and_merge(uconf_ty *data, const nstring &filename)
{
    uconf_ty	    *uconf_new;

    //
    // Read the file.
    //
    if (os_readable(filename) == 0)
	uconf_new = uconf_read_file(filename);
    else
	uconf_new = (uconf_ty *)uconf_type.alloc();

    //
    // Merge the two.
    //
    merge_uconf(data, uconf_new);
    uconf_type.free(uconf_new);
}


static void
fix_default_change(uconf_ty *uconf_data)
{
    if
    (
	(uconf_data->mask & uconf_default_change_number_mask)
    &&
	uconf_data->default_change_number == 0
    )
	uconf_data->default_change_number = MAGIC_ZERO;
}


/**
  * The user_uconf_get function is used to
  * fetch the "uconf" file for this user,
  * caching for future reference.
  *
  * @param up
  *	pointer to user structure
  * @returns
  *	pointer to uconf structure in dynamic memory
  */
uconf_ty *
user_uconf_get(user_ty::pointer up)
{
    trace(("user_uconf_get(up = %08lX)\n", (long)up.get()));
    return up->uconf_get();
}


uconf_ty *
user_ty::uconf_get()
{
    trace(("user_ty::uconf_get(this = %08lX)\n{\n", (long)this));
    lock_sync();
    if (!uconf_path)
	uconf_path = home + "/.aegisrc";

    //
    // Read in the user preferences.  There are several sources of
    // preferences, in the following priority order:
    //
    // $AEGIS_FLAGS
    // $HOME/.aegisrc
    // $(datadir)/aegisrc
    // $(libdir)/aegisrc
    //
    if (!uconf_data)
    {
	//
	// read the environment variable
	//
	uconf_data = (uconf_ty *)parse_env("AEGIS_FLAGS", &uconf_type);

	//
	// Read the user's $HOME/.aegisrc file.
	//
	become_begin();
	read_and_merge(uconf_data, uconf_path);

	//
	// read system architecture-neutral file
	//
	nstring datadir_aegisrc =
            nstring::format("%s/aegisrc", configured_datadir());
	read_and_merge(uconf_data, datadir_aegisrc);

	//
	// read system architecture-specific file
	//
        nstring libdir_aegisrc =
            nstring::format("%s/aegisrc", configured_libdir());
	read_and_merge(uconf_data, libdir_aegisrc);
	become_end();

	fix_default_change(uconf_data);
    }
    trace(("return %08lX;\n", (long)uconf_data));
    trace(("}\n"));
    return uconf_data;
}
