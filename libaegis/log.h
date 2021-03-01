//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 1996, 2002, 2004-2008 Peter Miller
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

#ifndef AEGIS_LOG_H
#define AEGIS_LOG_H

#include <libaegis/quit/action/log.h>
#include <libaegis/user.h>

enum log_style_ty
{
	log_style_append,
	log_style_append_default,
	log_style_create,
	log_style_create_default,
	log_style_none,
	log_style_none_default,
	log_style_snuggle,
	log_style_snuggle_default
};

class string_ty; // forward

void log_open(string_ty *, user_ty::pointer, log_style_ty);
void log_close(void);
extern quit_action_log log_quitter;

#endif // AEGIS_LOG_H
