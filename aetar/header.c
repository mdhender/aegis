/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate headers
 */

#include <ac/errno.h>
#include <ac/string.h>

#include <header.h>


static long octal _((char *buf, size_t len));

static long
octal(buf, len)
    char	    *buf;
    size_t	    len;
{
    long	    value;

    errno = EINVAL;
    value = 0;
    while (len > 0 && *buf == ' ')
    {
       	++buf;
       	--len;
    }
    if (len <= 0)
       	return -1;
    while (len > 0)
    {
       	if (!*buf || *buf == ' ')
	    break;
       	if (*buf < '0' || *buf > '7')
	    return -1;
       	/*
	 * Limit the range to 0..2^31-1.
	 * Must test for overflow *before* the shift.
	 */
       	if (value & 0xF0000000)
       	{
	    errno = ERANGE;
	    return -1;
       	}

       	value = (value << 3) + (*buf++ & 7);
       	--len;
    }
    errno = 0;
    return value;
}


static void to_octal _((char *buf, size_t len, long n));


static void
to_octal(buf, len, n)
    char	    *buf;
    size_t	    len;
    long	    n;
{
    buf[--len] = 0;
    memset(buf, ' ', len);
    while (len > 0)
    {
       	buf[--len] = '0' + (n & 7);
       	n >>= 3;
       	if (!n)
	    break;
    }
}


static size_t careful_strlen _((char *s, size_t n));

static size_t
careful_strlen(s, n)
    char	    *s;
    size_t	    n;
{
    char	    *ss;

    ss = s;
    while (n > 0 && *s)
    {
       	++s;
       	--n;
    }
    return (s - ss);
}


static void string_field_set _((char *to_buf, size_t to_len, string_ty *from));

static void
string_field_set(to_buf, to_len, from)
    char	    *to_buf;
    size_t	    to_len;
    string_ty	    *from;
{
    int from_len = from->str_length;
    const char *from_buf = from->str_text;
    if (from_len > to_len - 1)
	from_len = to_len - 1;
    if (from_len)
	memcpy(to_buf, from_buf, from_len);
    if (from_len < to_len)
	memset(to_buf + from_len, 0, to_len - from_len);
}


string_ty *
header_name_get(hp)
    header_ty	    *hp;
{
    return str_n_from_c(hp->name, careful_strlen(hp->name, sizeof(hp->name)));
}


void
header_name_set(hp, arg)
    header_ty	    *hp;
    string_ty	    *arg;
{
    string_field_set(hp->name, sizeof(hp->name), arg);
}


long
header_mode_get(hp)
    header_ty	    *hp;
{
    return (07777 & octal(hp->mode, sizeof(hp->mode)));
}


void
header_mode_set(hp, n)
    header_ty	    *hp;
    long	    n;
{
    to_octal(hp->mode, sizeof(hp->mode), n);
}


long
header_uid_get(hp)
    header_ty	    *hp;
{
    return octal(hp->uid, sizeof(hp->uid));
}


void
header_uid_set(hp, n)
    header_ty	    *hp;
    long	    n;
{
    to_octal(hp->uid, sizeof(hp->uid), n);
}


long
header_gid_get(hp)
    header_ty	    *hp;
{
    return octal(hp->gid, sizeof(hp->gid));
}


void
header_gid_set(hp, n)
    header_ty	    *hp;
    long	    n;
{
    to_octal(hp->gid, sizeof(hp->gid), n);
}


long
header_size_get(hp)
    header_ty	    *hp;
{
    return octal(hp->size, sizeof(hp->size));
}


void
header_size_set(hp, n)
    header_ty	    *hp;
    long	    n;
{
    to_octal(hp->size, sizeof(hp->size), n);
}


long
header_mtime_get(hp)
    header_ty	    *hp;
{
    return octal(hp->mtime, sizeof(hp->mtime));
}


void
header_mtime_set(hp, n)
    header_ty	    *hp;
    long	    n;
{
    to_octal(hp->mtime, sizeof(hp->mtime), n);
}


long
header_checksum_get(hp)
    header_ty	    *hp;
{
    return octal(hp->checksum, sizeof(hp->checksum));
}


void
header_checksum_set(hp, n)
    header_ty	    *hp;
    long	    n;
{
    to_octal(hp->checksum, sizeof(hp->checksum), n);
}


int
header_linkflag_get(hp)
    header_ty	    *hp;
{
    return (unsigned char)hp->linkflag;
}


void
header_linkflag_set(hp, n)
    header_ty	    *hp;
    int		    n;
{
    hp->linkflag = n;
}


string_ty *
header_linkname_get(hp)
    header_ty	    *hp;
{
    return
	str_n_from_c
	(
	    hp->linkname,
	    careful_strlen(hp->linkname, sizeof(hp->linkname))
	);
}


void
header_linkname_set(hp, arg)
    header_ty	    *hp;
    string_ty	    *arg;
{
    string_field_set(hp->linkname, sizeof(hp->linkname), arg);
}


string_ty *
header_uname_get(hp)
    header_ty	    *hp;
{
    if (0 != strncmp(hp->magic, TMAGIC, sizeof(hp->magic)))
	return 0;
    return
	str_n_from_c(hp->uname, careful_strlen(hp->uname, sizeof(hp->uname)));
}


void
header_uname_set(hp, arg)
    header_ty	    *hp;
    string_ty	    *arg;
{
    string_field_set(hp->uname, sizeof(hp->uname), arg);
    strcpy(hp->magic, TMAGIC);
}


string_ty *
header_gname_get(hp)
    header_ty	    *hp;
{
    if (0 != strncmp(hp->magic, TMAGIC, sizeof(hp->magic)))
	return 0;
    return
	str_n_from_c(hp->gname, careful_strlen(hp->gname, sizeof(hp->gname)));
}


void
header_gname_set(hp, arg)
    header_ty	    *hp;
    string_ty	    *arg;
{
    string_field_set(hp->gname, sizeof(hp->gname), arg);
    strcpy(hp->magic, TMAGIC);
}


long
header_devmajor_get(hp)
    header_ty	    *hp;
{
    return octal(hp->devmajor, sizeof(hp->devmajor));
}


void
header_devmajor_set(hp, n)
    header_ty	    *hp;
    long	    n;
{
    to_octal(hp->devmajor, sizeof(hp->devmajor), n);
}


long
header_devminor_get(hp)
    header_ty	    *hp;
{
    return octal(hp->devminor, sizeof(hp->devminor));
}


void
header_devminor_set(hp, n)
    header_ty	    *hp;
    long	    n;
{
    to_octal(hp->devminor, sizeof(hp->devminor), n);
}


long
header_checksum_calculate(hp)
    header_ty	    *hp;
{
    unsigned char *cp = (unsigned char *)hp;
    unsigned char *ep = (unsigned char *)hp->checksum;
    long sum = ((unsigned char)' ') * sizeof(hp->checksum);
    while (cp < ep)
       	sum += *cp++;
    cp = (unsigned char *)(hp->checksum + sizeof(hp->checksum));
    ep = (unsigned char *)hp + TBLOCK;
    while (cp < ep)
       	sum += *cp++;
    return sum;
}


#include <ac/stdio.h>


void
header_dump(hp)
    header_ty	    *hp;
{
    int		    j;
    int		    k;
    unsigned char   *cp;

    cp = (unsigned char *)hp;
    for (j = 0; j < TBLOCK; j += 16)
    {
       	fprintf(stderr, "%03X:", j);
       	for (k = 0; k < 16; ++k)
	    fprintf(stderr, " %02X", cp[j + k]);
       	fprintf(stderr, "  ");
       	for (k = 0; k < 16; ++k)
       	{
	    int c = cp[j + k] & 0x7F;
	    if (c < ' ' || c > '~')
	     	c = '.';
	    fputc(c, stderr);
	}
	fputc('\n', stderr);
    }
}
