//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef AECOMPLETE_COMPLETE_USER_REVIEWER_H
#define AECOMPLETE_COMPLETE_USER_REVIEWER_H

#include <aecomplete/complete.h>

struct project_ty; // forward

complete_ty *complete_user_reviewer(struct project_ty *);
complete_ty *complete_user_reviewer_not(struct project_ty *);

#endif // AECOMPLETE_COMPLETE_USER_REVIEWER_H
