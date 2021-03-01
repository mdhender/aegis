//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <common/ac/ctype.h>

#include <common/error.h>
#include <libaegis/project/history/uuid_trans.h>
#include <common/uuidentifier.h>


static int
safe_tolower(int c)
{
    unsigned char uc;

    //
    // The ANSI C standard only guarantees that tolower works for
    // characters for which isupper is true.  All other characters
    // result in undefined behaviour (some, but by NO means all, lib C
    // implementations pass all other characters through unscathed).
    //
    // This is performed in the C locale, no need for
    // language_human/language_C bracketing.
    //
    // We even though the characters are supposed to be lower case
    // already, we can't take any chances with the names of the history
    // files, or things will rapidly become unglued.
    //
    uc = c;
    if (isupper(uc))
	return tolower(uc);
    return uc;
}


string_ty *
project_history_uuid_translate(fstate_src_ty *src)
{
    char            buffer[46];

    assert(src);
    assert(src->file_name);
    if (!src->uuid)
	return str_copy(src->file_name);
    assert(src->uuid);
    assert(universal_unique_identifier_valid(src->uuid));

    //
    // The UUID
    //     aabbcccc-cccc-ddde-eeee-eeefffgggggg
    // becomes
    //      aa.uui/bb.uui/cccccccc.ddd/eeeeeeee.fff/gggggg
    //
    // The idea is that we have two sparsely populated directories at
    // the top that are quick to search (the names are random, it gets
    // exponentially less likely the deeper you go) followed by as few
    // densely populated directories as possible.  The really ugly
    // names are to avoid backwards compatibility issues.
    //
    // The file names have been designed so that we will work even on
    // the most brain-dead 8.3 file system.
    //
    assert(src->uuid->str_length == 36);
    buffer[ 0] = safe_tolower(src->uuid->str_text[ 0]);
    buffer[ 1] = safe_tolower(src->uuid->str_text[ 1]);
    buffer[ 2] = '.';
    buffer[ 3] = 'u';
    buffer[ 4] = 'u';
    buffer[ 5] = 'i';
    buffer[ 6] = '/';
    buffer[ 7] = safe_tolower(src->uuid->str_text[ 2]);
    buffer[ 8] = safe_tolower(src->uuid->str_text[ 3]);
    buffer[ 9] = '.';
    buffer[10] = 'u';
    buffer[11] = 'u';
    buffer[12] = 'i';
    buffer[13] = '/';
    buffer[14] = safe_tolower(src->uuid->str_text[ 4]);
    buffer[15] = safe_tolower(src->uuid->str_text[ 5]);
    buffer[16] = safe_tolower(src->uuid->str_text[ 6]);
    buffer[17] = safe_tolower(src->uuid->str_text[ 7]);
    assert(src->uuid->str_text[8] == '-');
    buffer[18] = safe_tolower(src->uuid->str_text[ 9]);
    buffer[19] = safe_tolower(src->uuid->str_text[10]);
    buffer[20] = safe_tolower(src->uuid->str_text[11]);
    buffer[21] = safe_tolower(src->uuid->str_text[12]);
    assert(src->uuid->str_text[13] == '-');
    buffer[22] = '.';
    buffer[23] = safe_tolower(src->uuid->str_text[14]);
    buffer[24] = safe_tolower(src->uuid->str_text[15]);
    buffer[25] = safe_tolower(src->uuid->str_text[16]);
    buffer[26] = '/';
    buffer[27] = safe_tolower(src->uuid->str_text[17]);
    assert(src->uuid->str_text[18] == '-');
    buffer[28] = safe_tolower(src->uuid->str_text[19]);
    buffer[29] = safe_tolower(src->uuid->str_text[20]);
    buffer[30] = safe_tolower(src->uuid->str_text[21]);
    buffer[31] = safe_tolower(src->uuid->str_text[22]);
    assert(src->uuid->str_text[23] == '-');
    buffer[32] = safe_tolower(src->uuid->str_text[24]);
    buffer[33] = safe_tolower(src->uuid->str_text[25]);
    buffer[34] = safe_tolower(src->uuid->str_text[26]);
    buffer[35] = '.';
    buffer[36] = safe_tolower(src->uuid->str_text[27]);
    buffer[37] = safe_tolower(src->uuid->str_text[28]);
    buffer[38] = safe_tolower(src->uuid->str_text[29]);
    buffer[39] = '/';
    buffer[40] = safe_tolower(src->uuid->str_text[30]);
    buffer[41] = safe_tolower(src->uuid->str_text[31]);
    buffer[42] = safe_tolower(src->uuid->str_text[32]);
    buffer[43] = safe_tolower(src->uuid->str_text[33]);
    buffer[44] = safe_tolower(src->uuid->str_text[34]);
    buffer[45] = safe_tolower(src->uuid->str_text[35]);

    //
    // Make sure the last filename component doesn't look like a number.
    // FHist can't cope with module names which look like numbers.
    //
    if
    (
	isdigit(buffer[40])
    &&
	isdigit(buffer[41])
    &&
	isdigit(buffer[42])
    &&
	isdigit(buffer[43])
    &&
	isdigit(buffer[44])
    &&
	isdigit(buffer[45])
    )
    {
	buffer[40] += 'z' - '0' - 9;
    }

    //
    // Form it into a string.
    //
    return str_n_from_c(buffer, 46);
}
