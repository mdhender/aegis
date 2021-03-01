//
//	cook - file construction tool
//	Copyright (C) 1994, 2003-2006, 2008 Peter Miller.
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
// Derived from code marked:
//	Daniel J. Bernstein, brnstnd@nyu.edu.
//	930708: fprintfile 0.95. Public domain.
//	930708: Added fingerprintfile_addn.
//	930622: Split off fprintfmt.c.
//	930601: Baseline, fprintfile 0.8. Public domain.
//	No known patent problems.
//

#include <common/error.h>
#include <common/fp/combined.h>
#include <common/fp/crc32.h>
#include <common/fp/len.h>
#include <common/fp/md5.h>
#include <common/fp/snefru.h>

struct combined_ty
{
    FINGERPRINT_BASE_CLASS
    fingerprint_ty  *snefru;
    fingerprint_ty  *md5;
    fingerprint_ty  *crc32;
    fingerprint_ty  *len;
};


static void
combined_constructor(fingerprint_ty *p)
{
    combined_ty     *f;

    f = (combined_ty *)p;
    f->snefru = fingerprint_new(&fp_snefru);
    f->md5 = fingerprint_new(&fp_md5);
    f->crc32 = fingerprint_new(&fp_crc32);
    f->len = fingerprint_new(&fp_len);
}


static void
combined_destructor(fingerprint_ty *p)
{
    combined_ty     *f;

    f = (combined_ty *)p;
    fingerprint_delete(f->snefru);
    fingerprint_delete(f->md5);
    fingerprint_delete(f->crc32);
    fingerprint_delete(f->len);
}


static void
combined_addn(fingerprint_ty *p, unsigned char *s, size_t n)
{
    combined_ty     *f;

    f = (combined_ty *)p;
    fingerprint_addn(f->snefru, s, n);
    fingerprint_addn(f->md5, s, n);
    fingerprint_addn(f->crc32, s, n);
    fingerprint_addn(f->len, s, n);
}


static int
combined_hash(fingerprint_ty *p, unsigned char *h)
{
    combined_ty     *f;
    int		    nbytes;
    unsigned char   *obuf;

    f = (combined_ty *)p;
    obuf = h;
    nbytes = fingerprint_hash(f->snefru, h);
    h += nbytes;
    nbytes = fingerprint_hash(f->md5, h);
    h += nbytes;
    nbytes = fingerprint_hash(f->crc32, h);
    h += nbytes;
    nbytes = fingerprint_hash(f->len, h);
    h += nbytes;
    return (h - obuf);
}


static char base64sane[] =
	"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ:.";


static void
combined_sum(fingerprint_ty *p, char *s, size_t data_len)
{
    unsigned char   h[1024];
    int		    i;
    unsigned long   x;
    int		    nbytes;

    nbytes = combined_hash(p, h);
    assert(nbytes == 57);

    for (i = 0; i < 19; ++i)
    {
	unsigned        idx;

	x = (h[3 * i] + 256L * (h[3 * i + 1] + 256L * h[3 * i + 2]));
	idx = (12 * i) % 76;
	if (idx < data_len)
	    s[idx] = base64sane[x & 63];
	x /= 64;
	idx = (12 * i + 41) % 76;
	if (idx < data_len)
	    s[idx] = base64sane[x & 63];
	x /= 64;
	idx = (12 * i + 6) % 76;
	if (idx < data_len)
	    s[idx] = base64sane[x & 63];
	x /= 64;
	idx = (12 * i + 47) % 76;
	if (idx < data_len)
	    s[idx] = base64sane[x & 63];
    }
    if (data_len < 77)
	s[data_len - 1] = 0;
    else
	s[76] = 0;
}


fingerprint_methods_ty fp_combined =
{
    sizeof(combined_ty),
    "fingerprint",
    combined_constructor,
    combined_destructor,
    combined_addn,
    combined_hash,
    combined_sum
};
