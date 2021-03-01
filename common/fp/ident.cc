//
// cook - file construction tool
// Copyright (C) 1995, 1999, 2003-2006, 2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
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
#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/debug.h>
#include <common/fp/ident.h>
#include <common/fp/combined.h>
#include <common/fp/crc32.h>

struct ident_ty
{
    FINGERPRINT_BASE_CLASS
    fingerprint_ty  *combined;
    fingerprint_ty  *crc32;
};


static void
ident_constructor(fingerprint_ty *p)
{
    ident_ty        *f;

    f = (ident_ty *)p;
    f->combined = fingerprint_new(&fp_combined);
    f->crc32 = fingerprint_new(&fp_crc32);
}


static void
ident_destructor(fingerprint_ty *p)
{
    ident_ty        *f;

    f = (ident_ty *)p;
    fingerprint_delete(f->combined);
    fingerprint_delete(f->crc32);
}


static void
ident_addn(fingerprint_ty *p, unsigned char *s, size_t n)
{
    ident_ty    *f;

    f = (ident_ty *)p;
    fingerprint_addn(f->combined, s, n);
}


static int
ident_hash(fingerprint_ty *p, unsigned char *h)
{
    ident_ty        *f;
    size_t          nbytes;
    unsigned char   t[1024];

    f = (ident_ty *)p;
    nbytes = fingerprint_hash(f->combined, t);
    fingerprint_addn(f->crc32, t, nbytes);
    nbytes = fingerprint_hash(f->crc32, h);
    return(nbytes);
}


static void
ident_sum(fingerprint_ty *p, char *obuf, size_t obuf_len)
{
    unsigned char   h[1024];
    unsigned long   x;
    static char digits[] = "0123456789";
    char            *cp;

#ifdef DEBUG
    int             nbytes;

    nbytes = ident_hash(p, h);
    assert(nbytes == 4);
#else
    ident_hash(p, h);
#endif

    x = h[0] | (h[1] << 8) | (h[2] << 16) | (h[3] << 24);
    snprintf(obuf, obuf_len, "%8.8lx", x);

    //
    // some older stdio implementations don't grok the above format
    // string, so hunt down and kill any spaces.
    //
    for (cp = obuf; *cp; ++cp)
        if (*cp == ' ')
            *cp = '0';

    //
    // This forces the first character to be a letter, so
    // the result is a valid identifier in most computer
    // languages. The strchr makes it not ASCII specific.
    //
    cp = strchr(digits, *obuf);
    if (cp)
        *obuf = "ghijklmnop"[cp - digits];
}


fingerprint_methods_ty fp_ident =
{
    sizeof(ident_ty),
    "identifier",
    ident_constructor,
    ident_destructor,
    ident_addn,
    ident_hash,
    ident_sum
};


// vim: set ts=8 sw=4 et :
