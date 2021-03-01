//
// aegis - project change supervisor
// Copyright (C) 2007 Walter Franzini
// Copyright (C) 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>
#include <common/ac/ctype.h>

#include <common/nstring.h>
#include <common/uuidentifier.h>
#include <common/uuidentifier/translate.h>



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

nstring
uuid_translate(const nstring &uuid)
{
    assert(universal_unique_identifier_valid(uuid));

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
    assert(uuid.length() == 36);
    char buffer[46];

    buffer[ 0] = safe_tolower(uuid[ 0]);
    buffer[ 1] = safe_tolower(uuid[ 1]);
    buffer[ 2] = '.';
    buffer[ 3] = 'u';
    buffer[ 4] = 'u';
    buffer[ 5] = 'i';
    buffer[ 6] = '/';
    buffer[ 7] = safe_tolower(uuid[ 2]);
    buffer[ 8] = safe_tolower(uuid[ 3]);
    buffer[ 9] = '.';
    buffer[10] = 'u';
    buffer[11] = 'u';
    buffer[12] = 'i';
    buffer[13] = '/';
    buffer[14] = safe_tolower(uuid[ 4]);
    buffer[15] = safe_tolower(uuid[ 5]);
    buffer[16] = safe_tolower(uuid[ 6]);
    buffer[17] = safe_tolower(uuid[ 7]);
    assert(uuid[8] == '-');
    buffer[18] = safe_tolower(uuid[ 9]);
    buffer[19] = safe_tolower(uuid[10]);
    buffer[20] = safe_tolower(uuid[11]);
    buffer[21] = safe_tolower(uuid[12]);
    assert(uuid[13] == '-');
    buffer[22] = '.';
    buffer[23] = safe_tolower(uuid[14]);
    buffer[24] = safe_tolower(uuid[15]);
    buffer[25] = safe_tolower(uuid[16]);
    buffer[26] = '/';
    buffer[27] = safe_tolower(uuid[17]);
    assert(uuid[18] == '-');
    buffer[28] = safe_tolower(uuid[19]);
    buffer[29] = safe_tolower(uuid[20]);
    buffer[30] = safe_tolower(uuid[21]);
    buffer[31] = safe_tolower(uuid[22]);
    assert(uuid[23] == '-');
    buffer[32] = safe_tolower(uuid[24]);
    buffer[33] = safe_tolower(uuid[25]);
    buffer[34] = safe_tolower(uuid[26]);
    buffer[35] = '.';
    buffer[36] = safe_tolower(uuid[27]);
    buffer[37] = safe_tolower(uuid[28]);
    buffer[38] = safe_tolower(uuid[29]);
    buffer[39] = '/';
    buffer[40] = safe_tolower(uuid[30]);
    buffer[41] = safe_tolower(uuid[31]);
    buffer[42] = safe_tolower(uuid[32]);
    buffer[43] = safe_tolower(uuid[33]);
    buffer[44] = safe_tolower(uuid[34]);
    buffer[45] = safe_tolower(uuid[35]);

    //
    // Form it into a string.
    //
    return nstring(buffer, 46);
}


// vim: set ts=8 sw=4 et :
