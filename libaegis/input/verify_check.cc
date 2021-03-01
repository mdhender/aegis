//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#include <common/ac/zlib.h>

#include <libaegis/input/verify_check.h>
#include <libaegis/sub.h>


input_verify_checksum::~input_verify_checksum()
{
}


input_verify_checksum::input_verify_checksum(input &arg1, unsigned long arg2) :
    deeper(arg1),
    checksum_given(arg2),
    checksum_calculated(adler32(0L, Z_NULL, 0)),
    pos(0)
{
}


long
input_verify_checksum::read_inner(void *data, size_t nbytes)
{
    long result = deeper->read(data, nbytes);
    if (!result)
    {
	if (checksum_calculated != checksum_given)
	{
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", deeper->name());
	    sc.var_set_string
    	    (
		"Number1",
		nstring::format("%08lX", checksum_given)
	    );
	    sc.var_set_string
    	    (
		"Number2",
		nstring::format("%08lX", checksum_calculated)
	    );
	    sc.fatal_intl
	    (
		i18n("$filename: checksum mismatch ($number1 != $number2)")
	    );
	    // NOTREACHED
	}
	return 0;
    }
    checksum_calculated = adler32(checksum_calculated, (Bytef *)data, result);
    pos += result;
    return result;
}


nstring
input_verify_checksum::name()
{
    return deeper->name();
}


long
input_verify_checksum::length()
{
    return deeper->length();
}


long
input_verify_checksum::ftell_inner()
{
    return pos;
}


bool
input_verify_checksum::is_remote()
    const
{
    return deeper->is_remote();
}
