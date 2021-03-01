//
//	cook - file construction tool
//	Copyright (C) 1994, 1998, 1999, 2003-2006, 2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/fcntl.h>
#include <common/ac/unistd.h>

#include <common/fp.h>
#include <common/mem.h>


fingerprint_ty *
fingerprint_new(fingerprint_methods_ty *mp)
{
    fingerprint_ty  *fp;

    fp = (fingerprint_ty *)mem_alloc(mp->size);
    fp->method = mp;
    mp->constructor(fp);
    return fp;
}


void
fingerprint_delete(fingerprint_ty *fp)
{
    fp->method->destructor(fp);
    mem_free(fp);
}


static int
fingerprint_scan(fingerprint_ty *fp, const char *fn)
{
    int		    fd;
    unsigned char   ibuf[1024];
    long	    nbytes;
    int		    err;

    if (fn)
    {
	fd = open(fn, O_RDONLY | O_BINARY, 0666);
	if (fd < 0)
    	    return -1;
    }
    else
	fd = 0;
    for (;;)
    {
	nbytes = read(fd, ibuf, sizeof(ibuf));
	if (nbytes < 0)
	{
	    if (fn)
	    {
	       	err = errno;
	       	close(fd);
	       	errno = err;
	    }
	    return -1;
	}
	if (nbytes == 0)
	    break;
	fingerprint_addn(fp, ibuf, nbytes);
    }
    if (fn && close(fd) < 0)
	return -1;
    return 0;
}


int
fingerprint_file_hash(fingerprint_ty *fp, const char *fn, unsigned char *obuf)
{
    if (fingerprint_scan(fp, fn))
	return -1;
    return fingerprint_hash(fp, obuf);
}


int
fingerprint_file_sum(fingerprint_ty *fp, const char *fn, char *obuf,
    size_t obuf_len)
{
    if (fingerprint_scan(fp, fn))
	return -1;
    fingerprint_sum(fp, obuf, obuf_len);
    return 0;
}


void
fingerprint_add(fingerprint_ty *p, int c)
{
    unsigned char   buf;

    buf = c;
    fingerprint_addn(p, &buf, 1);
}
