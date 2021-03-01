//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
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
// MANIFEST: functions to manipulate useunchangeds
//
//
// UseUnchanged
//
// To specify the version of the protocol described in [cvsclient.text
// 1.12.5], servers must support this request (although it need not do
// anything) and clients must issue it.
//
// Response expected: no.
// Root required: no.
//

#include <aecvsserver/request/useunchanged.h>
#include <aecvsserver/server.h>


request_useunchanged::~request_useunchanged()
{
}


request_useunchanged::request_useunchanged()
{
}


void
request_useunchanged::run_inner(server_ty *sp, string_ty *arg)
    const
{
    // Do nothing.
}


const char *
request_useunchanged::name()
    const
{
    return "UseUnchanged";
}


bool
request_useunchanged::reset()
    const
{
    return false;
}
