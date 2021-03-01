/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate mains
 */

#include <ac/stdlib.h>

#include <arglex3.h>
#include <env.h>
#include <error.h>
#include <import.h>
#include <language.h>
#include <log.h>
#include <os.h>
#include <r250.h>
#include <str.h>
#include <undo.h>


/*
 * NAME
 *	import
 *
 * SYNOPSIS
 *	void import(void);
 *
 * DESCRIPTION
 *	The import function is used to
 *	dispatch the `aeimport' command to the relevant functionality.
 *	Where it goes depends on the command line.
 */

int
main(int argc, char **argv)
{
	r250_init();
	os_become_init();
	arglex3_init(argc, argv);
	str_initialize();
	env_initialize();
	language_init();
	quit_register(log_quitter);
	quit_register(undo_quitter);
	os_interrupt_register();
	import();
	exit(0);
	return 0;
}
