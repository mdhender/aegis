/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate is_configs
 */

#include <change/file.h>
#include <os.h>
#include <pconf.h>


int
change_file_is_config(cp, file_name)
    change_ty	    *cp;
    string_ty	    *file_name;
{
    static string_ty *the_config_file;
    pconf	    pconf_data;
    string_ty	    *s;

    /*
     * If the file name is the top level config file, it is a config file.
     */
    if (!the_config_file)
	the_config_file = str_from_c(THE_CONFIG_FILE);
    if (str_equal(the_config_file, file_name))
	return 1;

    /*
     * If the project does not have a config directory, then file is
     * not a confiog file.
     */
    pconf_data = change_pconf_get(cp, 0);
    if
    (
	!pconf_data
    ||
	!pconf_data->configuration_directory
    ||
	!pconf_data->configuration_directory->str_text
    )
	return 0;

    /*
     * If the file is within the config dirfectory it is a confif gile.
     */
    s = os_below_dir(pconf_data->configuration_directory, file_name);
    if (!s)
	return 0;
    str_free(s);
    return 1;
}
