//
//      aegis - project change supervisor
//      Copyright (C) 2007-2009 Peter Miller
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License as
//      published by the Free Software Foundation; either version 3 of
//      the License, or (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//
// Derived from a file marked
//
//      Copyright (C) 1991, 1992, 1993, 1994, 1996, 1997, 1998, 2000,
//      2004, 2007, 2008 Free Software Foundation, Inc.
//      This file is part of the GNU C Library.

#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/main.h>


//
// Knuth-Morris-Pratt algorithm.
// See http://en.wikipedia.org/wiki/Knuth-Morris-Pratt_algorithm
// Return a boolean indicating success: false means the malloc failed,
// true means that the comparison has been completed, and *resultp will
// the appropriate value to return from memmem.
//

static bool
knuth_morris_pratt(const unsigned char *haystack,
    const unsigned char *last_haystack, const unsigned char *needle, size_t m,
    const unsigned char **resultp)
{
    // Allocate the table.
    size_t *table = (size_t *)malloc(m * sizeof(size_t));
    if (!table)
        return false;
    // Fill the table.
    // For 0 < i < m:
    //   0 < table[i] <= i is defined such that
    //   forall 0 < x < table[i]: needle[x..i-1] != needle[0..i-1-x],
    //   and table[i] is as large as possible with this property.
    // This implies:
    // 1) For 0 < i < m:
    //      If table[i] < i,
    //      needle[table[i]..i-1] = needle[0..i-1-table[i]].
    // 2) For 0 < i < m:
    //      rhaystack[0..i-1] == needle[0..i-1]
    //      and exists h, i <= h < m: rhaystack[h] != needle[h]
    //      implies
    //      forall 0 <= x < table[i]: rhaystack[x..x+m-1] != needle[0..m-1].
    // table[0] remains uninitialized.
    {
        // i = 1: Nothing to verify for x = 0.
        table[1] = 1;
        size_t j = 0;

        for (size_t i = 2; i < m; i++)
        {
            // Here: j = i-1 - table[i-1].
            // The inequality needle[x..i-1] != needle[0..i-1-x] is
            // known to hold for x < table[i-1], by induction.
            // Furthermore, if j>0: needle[i-1-j..i-2] = needle[0..j-1].
            unsigned char b = needle[i - 1];

            for (;;)
            {
                // Invariants: The inequality needle[x..i-1] != needle[0..i-1-x]
                // is known to hold for x < i-1-j.
                // Furthermore, if j>0: needle[i-1-j..i-2] = needle[0..j-1].
                if (b == needle[j])
                {
                    // Set table[i] := i-1-j.
                    table[i] = i - ++j;
                    break;
                }
                // The inequality needle[x..i-1] != needle[0..i-1-x] also holds
                // for x = i-1-j, because
                //   needle[i-1] != needle[j] = needle[i-1-x].
                if (j == 0)
                {
                    // The inequality holds for all possible x.
                    table[i] = i;
                    break;
                }
                // The inequality needle[x..i-1] != needle[0..i-1-x] also holds
                // for i-1-j < x < i-1-j+table[j], because for these x:
                //   needle[x..i-2]
                //   = needle[x-(i-1-j)..j-1]
                //   != needle[0..j-1-(x-(i-1-j))]  (by definition of table[j])
                //      = needle[0..i-2-x],
                // hence needle[x..i-1] != needle[0..i-1-x].
                // Furthermore
                //   needle[i-1-j+table[j]..i-2]
                //   = needle[table[j]..j-1]
                //   = needle[0..j-1-table[j]]  (by definition of table[j]).
                j = j - table[j];
            }
            // Here: j = i - table[i].
        }
    }

    // Search, using the table to accelerate the processing.
    {
        *resultp = 0;
        size_t j = 0;
        const unsigned char *rhaystack = haystack;
        const unsigned char *phaystack = haystack;
        // Invariant: phaystack = rhaystack + j.
        while (phaystack != last_haystack)
        {
            if (needle[j] == *phaystack)
            {
                j++;
                phaystack++;
                if (j == m)
                {
                    // The entire needle has been found.
                    *resultp = rhaystack;
                    break;
                }
            }
            else if (j > 0)
            {
                // Found a match of needle[0..j-1], mismatch at needle[j].
                rhaystack += table[j];
                j -= table[j];
            }
            else
            {
                // Found a mismatch at needle[0] already.
                rhaystack++;
                phaystack++;
            }
        }
    }

    free(table);
    // The comparison has been completed, and *resultp now contains the
    // correct value to be returned from memmem.
    return true;
}


//
// Return the first occurrence of NEEDLE in HAYSTACK.  Return HAYSTACK
// if NEEDLE_LEN is 0, otherwise NULL if NEEDLE is not found in
// HAYSTACK.
//
//
extern "C" const void *
memmem_replacement(const void *haystack_start, size_t haystack_len,
    const void *needle_start, size_t needle_len)
{
    // Abstract memory is considered to be an array of 'unsigned char' values,
    // not an array of 'char' values.  See ISO C 99 section 6.2.6.1.
    const unsigned char *haystack = (const unsigned char *)haystack_start;
    const unsigned char *needle = (const unsigned char *)needle_start;
    const unsigned char *last_haystack = haystack + haystack_len;
    const unsigned char *last_needle = needle + needle_len;

    if (needle_len == 0)
    {
        // The first occurrence of the empty string is deemed to occur at
        // the beginning of the string.
        return (void *)haystack;
    }

    // Sanity check, otherwise the loop might search through the whole
    // memory.
    if (haystack_len < needle_len)
        return 0;

    // Use optimizations in memchr when possible.
    if (needle_len == 1)
        return memchr(haystack, *needle, haystack_len);

    // Minimizing the worst-case complexity:
    // Let n = haystack_len, m = needle_len.
    // The naïve algorithm is O(n*m) worst-case.
    // The Knuth-Morris-Pratt algorithm is O(n) worst-case but it needs a
    // memory allocation.
    // To achieve linear complexity and yet amortize the cost of the
    // memory allocation, we activate the Knuth-Morris-Pratt algorithm
    // only once the naïve algorithm has already run for some time; more
    // precisely, when
    //   - the outer loop count is >= 10,
    //   - the average number of comparisons per outer loop is >= 5,
    //   - the total number of comparisons is >= m.
    // But we try it only once.  If the memory allocation attempt failed,
    // we don't retry it.
    {
        bool try_kmp = true;
        size_t outer_loop_count = 0;
        size_t comparison_count = 0;

        // Speed up the following searches of needle by caching its first
        // byte.
        unsigned char b = *needle++;

        for (;; haystack++)
        {
            if (haystack == last_haystack)
            {
                // No match.
                return 0;
            }

            // See whether it's advisable to use an asymptotically faster
            // algorithm.
            if
            (
                try_kmp
            &&
                outer_loop_count >= 10
            &&
                comparison_count >= 5 * outer_loop_count
            )
            {
                // See if needle + comparison_count now reaches the end of
                // needle.
                if (comparison_count >= needle_len)
                {
                    // Try the Knuth-Morris-Pratt algorithm.  Note that
                    // returning false means the malloc failed, and we
                    // will not try KMP again.  Returning true means that
                    // "result" contains the value to be returned my memmem.
                    const unsigned char *result;
                    if
                    (
                        knuth_morris_pratt
                        (
                            haystack,
                            last_haystack,
                            needle - 1,
                            needle_len,
                            &result
                        )
                    )
                        return (void *)result;
                    try_kmp = false;
                }
            }

            outer_loop_count++;
            comparison_count++;
            if (*haystack == b)
            {
                // The first byte matches.
                const unsigned char *rhaystack = haystack + 1;
                const unsigned char *rneedle = needle;

                for (;; rhaystack++, rneedle++)
                {
                    if (rneedle == last_needle)
                    {
                        // Found a match.
                        return (void *)haystack;
                    }
                    if (rhaystack == last_haystack)
                    {
                        // No match.
                        return 0;
                    }
                    comparison_count++;
                    if (*rhaystack != *rneedle)
                    {
                        // Nothing in this round.
                        break;
                    }
                }
            }
        }
    }

    return 0;
}
