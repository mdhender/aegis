//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License as
//      published by the Free Software Foundation; either version 2 of
//      the License, or (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public
//      License along with this program; if not, write to the Free
//      Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//      Boston, MA 02110-1301 USA.
//

#include <common/ac/string.h>
#include <common/main.h>

//
// This function implements the Horspool algorithm.
// http://en.wikipedia.org/wiki/Boyer%E2%80%93Moore%E2%80%93Horspool_algorithm
// It is assumed that chars have eight bits.
//
extern "C" void *
memmem_replacement(const void *const haystack_vp, const size_t haystack_len,
    const void *const needle_vp, const size_t needle_len)
{
    if (needle_len == 0)
        return (void *)haystack_vp;
    const unsigned char *haystack = (const unsigned char *)haystack_vp;
    const unsigned char *needle = (const unsigned char *)needle_vp;
    const unsigned char *const haystack_endptr = haystack + haystack_len;
    const unsigned char *const needle_endptr = needle + needle_len;
    const unsigned char *haystack_shifting_ptr;
    size_t occurrence[256];

    //
    // Compute occurrence function.
    //
    for (size_t j = 0; j < SIZEOF(occurrence); ++j)
        occurrence[j] = 0;
    const unsigned char *needle_ptr = needle;
    for (size_t i = 1; i < needle_len; ++i)
        occurrence[*needle_ptr++] = i;

    //
    // Matching algorithm
    //
    haystack_shifting_ptr = haystack;
    while ((size_t)(haystack_endptr - haystack_shifting_ptr) >= needle_len)
    {
        // FIXME: wouldn't memcmp be faster?
#if 1
        const unsigned char *haystack_ptr = haystack_shifting_ptr + needle_len;
        needle_ptr = needle_endptr;
        size_t len = needle_len;
        while (len > 0 && *--haystack_ptr == *--needle_ptr)
            --len;
        if (len == 0)
            return (void *)haystack_ptr;
#else
        if (0 == memcmp(haystack_shifting_ptr, needle, needle_len))
            return (void *)haystack_shifting_ptr;
#endif

        haystack_shifting_ptr +=
            needle_len - occurrence[haystack_shifting_ptr[needle_len - 1]];
    }

    return 0;
}
