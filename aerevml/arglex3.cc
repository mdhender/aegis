//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller
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
// MANIFEST: implementation of the arglex3 class
//

#include <aerevml/arglex3.h>


static arglex_table_ty argtab[] =
{
    { "-Content_Transfer_Encoding", arglex_token_content_transfer_encoding},
    { "-Description_Header", arglex_token_description_header, },
    { "-Not_Description_Header", arglex_token_description_header_not, },
    { "-Entire_Source", arglex_token_entire_source, },
    { "-Not_Entire_Source", arglex_token_entire_source_not, },
    { "-Partial_Source", arglex_token_entire_source_not, },
    { "-Receive", arglex_token_receive, },
    { "-Send", arglex_token_send, },
    { "-Trojan", arglex_token_trojan, },
    { "-Not_Trojan", arglex_token_trojan_not, },
    ARGLEX_END_MARKER
};


void
arglex3_init(int argc, char **argv)
{
    arglex2_init3(argc, argv, argtab);
}
