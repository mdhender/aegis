//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 1998, 1999, 2001-2006, 2008 Peter Miller
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
//
// A literal pool is maintained.  Each string has a reference count.  The
// string stays in the literal pool for as long as it has a positive
// reference count.  To determine if a string is already in the literal pool,
// linear dynamic hashing is used to guarantee an O(1) search.  Making all equal
// strings the same item in the literal pool means that string equality is
// a pointer test, and thus very fast.
//

#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/mem.h>
#include <common/str.h>


//
// maximum conversion width for numbers
//
#define MAX_WIDTH 509

static string_ty **hash_table;
static str_hash_ty hash_modulus;
static str_hash_ty hash_mask;
static str_hash_ty hash_load;

#define MAX_HASH_LEN 20


//
// NAME
//	hash_generate - hash string to number
//
// SYNOPSIS
//	str_hash_ty hash_generate(char *s, size_t n);
//
// DESCRIPTION
//	The hash_generate function is used to make a number from a string.
//
// RETURNS
//	str_hash_ty - the magic number
//
// CAVEAT
//	Only the last MAX_HASH_LEN characters are used.
//	It is important that str_hash_ty be unsigned (int or long).
//

static str_hash_ty
hash_generate(const char *s, size_t n)
{
    str_hash_ty     retval;

    if (n > MAX_HASH_LEN)
    {
	s += n - MAX_HASH_LEN;
	n = MAX_HASH_LEN;
    }

    retval = 0;
    while (n > 0)
    {
	retval = (retval + (retval << 1)) ^ *s++;
	--n;
    }
    return retval;
}


static void
str_initialize(void)
{
    str_hash_ty     j;

    hash_modulus = 1 << 8;	// MUST be a power of 2
    hash_mask = hash_modulus - 1;
    hash_load = 0;
    hash_table = (string_ty **)mem_alloc(hash_modulus * sizeof(string_ty *));
    for (j = 0; j < hash_modulus; ++j)
	hash_table[j] = 0;
}

void
str_release(void)
{
    mem_free(hash_table);
    hash_table = 0;
}

//
// NAME
//	split - reduce table loading
//
// SYNOPSIS
//	void split(void);
//
// DESCRIPTION
//	The split function is used to reduce the load factor on the hash table.
//
// RETURNS
//	void
//
// CAVEAT
//	A load factor of about 80% is suggested.
//

static void
split(void)
{
    string_ty       **new_hash_table;
    str_hash_ty     new_hash_modulus;
    str_hash_ty     new_hash_mask;
    str_hash_ty     idx;

    //
    // double the modulus
    //
    // This is subtle.  If we only increase the modulus by one, the
    // load always hovers around 80%, so we have to do a split for
    // every insert.  I.e. thr malloc burden os O(n) for the lifetime of
    // the program.  BUT if we double the modulus, the length of time
    // until the next split also doubles, making the probablity of a
    // split halve, and sigma(2**-n)=1, so the malloc burden becomes O(1)
    // for the lifetime of the program.
    //
    new_hash_modulus = hash_modulus * 2;
    new_hash_table =
        (string_ty **)mem_alloc(new_hash_modulus * sizeof(string_ty *));
    new_hash_mask = new_hash_modulus - 1;

    //
    // now redistribute the list elements
    //
    for (idx = 0; idx < hash_modulus; ++idx)
    {
	string_ty       *p;

	new_hash_table[idx] = 0;
	new_hash_table[idx + hash_modulus] = 0;
	p = hash_table[idx];
	while (p)
	{
	    string_ty       *p2;
	    str_hash_ty     new_idx;

	    p2 = p;
	    p = p->str_next;

	    assert((p2->str_hash & hash_mask) == idx);
	    new_idx = p2->str_hash & new_hash_mask;
	    p2->str_next = new_hash_table[new_idx];
	    new_hash_table[new_idx] = p2;
	}
    }
    mem_free(hash_table);
    hash_table = new_hash_table;
    hash_modulus = new_hash_modulus;
    hash_mask = new_hash_mask;
}


string_ty *
str_from_c(const char *s)
{
    return str_n_from_c(s, strlen(s));
}


string_ty *
str_n_from_c(const char *s, size_t length)
{
    str_hash_ty     hash;
    str_hash_ty     idx;
    string_ty       *p;

    hash = hash_generate(s, length);

    if (!hash_table)
	str_initialize();
    idx = hash & hash_mask;

    for (p = hash_table[idx]; p; p = p->str_next)
    {
	if
	(
	    p->str_hash == hash
	&&
	    p->str_length == length
	&&
	    !memcmp(p->str_text, s, length)
	)
	{
	    p->str_references++;
	    return p;
	}
    }

    p = (string_ty *)mem_alloc(sizeof(string_ty) + length);
    p->str_hash = hash;
    p->str_length = length;
    p->str_references = 1;
    p->str_next = hash_table[idx];
    hash_table[idx] = p;
    memcpy(p->str_text, s, length);
    p->str_text[length] = 0;

    hash_load++;
    if (hash_load * 10 > hash_modulus * 8)
	split();
    return p;
}


string_ty *
str_copy(string_ty *s)
{
    s->str_references++;
    return s;
}


void
str_free(string_ty *s)
{
    str_hash_ty     idx;
    string_ty       **spp;

    if (!s)
	return;
    if (s->str_references > 1)
    {
	s->str_references--;
	return;
    }

    //
    // find the hash bucket it was in,
    // and remove it
    //
    idx = s->str_hash & hash_mask;
    for (spp = &hash_table[idx]; *spp; spp = &(*spp)->str_next)
    {
	if (*spp == s)
	{
	    *spp = s->str_next;
	    mem_free(s);
	    --hash_load;
	    return;
	}
    }

    //
    // should never reach here!
    //
#ifdef DEBUG
    error_raw("attempted to free non-existent string (bug)");
    abort();
#else
    fatal_raw("attempted to free non-existent string (bug)");
#endif
}


int
str_validate(const string_ty *s)
{
    str_hash_ty     idx;
    string_ty       **spp;

    if (!s)
	return 0;
    if (s->str_references == 0)
	return 0;
    idx = s->str_hash & hash_mask;
    for (spp = &hash_table[idx]; *spp; spp = &(*spp)->str_next)
	if (*spp == s)
	    return 1;
    return 0;
}


void
slow_to_fast(const char *const *in, string_ty **out, size_t length)
{
    size_t          j;

    if (out[0])
	return;
    for (j = 0; j < length; ++j)
	out[j] = str_from_c(in[j]);
}
