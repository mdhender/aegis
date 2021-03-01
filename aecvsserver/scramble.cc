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
// Taken from cvs-1.12.5/src/scramble.c...
//

#include <common/nstring/accumulator.h>
#include <aecvsserver/scramble.h>

//
// Trivially encode strings to protect them from innocent eyes (i.e.,
// inadvertent password compromises, like a network administrator
// who's watching packets for legitimate reasons and accidentally sees
// the password protocol go by).
//
// This is NOT secure encryption.
//
// It would be tempting to encode the password according to username
// and repository, so that the same password would encode to a
// different string when used with different usernames and/or
// repositories.  However, then users would not be able to cut and
// paste passwords around.  They're not supposed to anyway, but we all
// know they will, and there's no reason to make it harder for them if
// we're not trying to provide real security anyway.
//

//
// Map characters to each other randomly and symmetrically, A <--> B.
//
// We divide the ASCII character set into 3 domains: control chars (0
// thru 31), printing chars (32 through 126), and "meta"-chars (127
// through 255).  The control chars map _to_ themselves, the printing
// chars map _among_ themselves, and the meta chars map _among_
// themselves.  Why is this thus?
//
// No character in any of these domains maps to a character in another
// domain, because I'm not sure what characters are valid in
// passwords, or what tools people are likely to use to cut and paste
// them.  It seems prudent not to introduce control or meta chars,
// unless the user introduced them first.  And having the control
// chars all map to themselves insures that newline and
// carriage-return are safely handled.
//

static const unsigned char shifts[] =
{
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  114,120, 53, 79, 96,109, 72,108, 70, 64, 76, 67,116, 74, 68, 87,
  111, 52, 75,119, 49, 34, 82, 81, 95, 65,112, 86,118,110,122,105,
   41, 57, 83, 43, 46,102, 40, 89, 38,103, 45, 50, 42,123, 91, 35,
  125, 55, 54, 66,124,126, 59, 47, 92, 71,115, 78, 88,107,106, 56,
   36,121,117,104,101,100, 69, 73, 99, 63, 94, 93, 39, 37, 61, 48,
   58,113, 32, 90, 44, 98, 60, 51, 33, 97, 62, 77, 84, 80, 85,223,
  225,216,187,166,229,189,222,188,141,249,148,200,184,136,248,190,
  199,170,181,204,138,232,218,183,255,234,220,247,213,203,226,193,
  174,172,228,252,217,201,131,230,197,211,145,238,161,179,160,212,
  207,221,254,173,202,146,224,151,140,196,205,130,135,133,143,246,
  192,159,244,239,185,168,215,144,139,165,180,157,147,186,214,176,
  227,231,219,169,175,156,206,198,129,164,150,210,154,177,134,127,
  182,128,158,208,162,132,167,209,149,241,153,251,237,236,171,195,
  243,233,253,240,194,250,191,155,142,137,245,235,163,242,178,152
};


//
// SCRAMBLE and DESCRAMBLE work like this:
//
// scramble(STR) returns SCRM, a scrambled copy of STR.  SCRM[0] is a
// single letter indicating the scrambling method.  As of this
// writing, the only valid method is 'A', but check the code for more
// up-to-date information.  The copy will have been allocated with
// xmalloc().
//
// descramble(SCRM) returns STR, again in its own xmalloc'd space.
// descramble() uses SCRM[0] to determine which method of unscrambling
// to use.  If it does not recognize the method, it dies with error.
//

nstring
scramble(const nstring &str)
{
    //
    // The 'A' prefix that indicates which version of scrambling this is
    // (the first, obviously, since we only do one kind of scrambling so
    // far).
    //
    nstring_accumulator buffer;
    buffer.push_back('A');
    for (size_t j = 0; j < str.size(); j++)
	buffer.push_back(shifts[(unsigned char)(str[j])]);
    return buffer.mkstr();
}


nstring
descramble(const nstring &str)
{
    //
    // For now we can only handle one kind of scrambling.  In the future
    // there may be other kinds, and this `if' will become a `switch'.
    //
    if (str[0] != 'A')
	return str;

    //
    // Skip the 'A' prefix that indicates which version of scrambling
    // this is (the first, obviously, since we only do one kind of
    // scrambling so far).
    //
    nstring_accumulator buffer;
    for (size_t j = 1; j < str.size(); j++)
	buffer.push_back(shifts[(unsigned char)(str[j])]);
    return buffer.mkstr();
}
