//
//	aegis - project change supervisor
//	Copyright (C) 2004 Walter Franzini;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate UUIDs
//
#include <ac/dce/rpc.h>
#include <ac/fcntl.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/unistd.h>
#include <ac/uuid.h>

#include <error.h>
#include <r250.h>
#include <str.h>
#include <uuidentifier.h>


static string_ty *
make_uuid_random(void)
{
    static const char charset[] = "0123456789abcdef";
    char            buffer[36];
    size_t          i;

    //
    // Generate a random UUID as per
    // http://www.opengroup.org/dce/info/draft-leach-uuids-guids-01.txt
    //
    for (i = 0; i < 36; ++i)
    {
	switch (i)
	{
	case 8:
	case 13:
	case 18:
	case 23:
	    buffer[i] = '-';
	    break;

	case 14:
	case 19:
	    // see below
	    break;

	default:
	    buffer[i] = charset[r250() & 15];
	    break;
	}
    }

    // version
    buffer[14] = '4';
    // top 2 bits are variant
    buffer[19] = charset[8 | (r250() & 3)];

    return str_n_from_c(buffer, 36);
}


#if defined(UUID_IS_OSSP)

static string_ty *
make_uuid(void)
{
    uuid_rc_t       u_rc;
    uuid_t          *uuid;
    void            *vp;
    string_ty       *s;

    u_rc = uuid_create(&uuid);
    if (u_rc != UUID_RC_OK)
    {
	failed:
	return make_uuid_random();
    }
    u_rc = uuid_make(uuid, UUID_MAKE_V4);
    if (u_rc != UUID_RC_OK)
	goto failed;
    vp = NULL;
    u_rc = uuid_export(uuid, UUID_FMT_STR, &vp, NULL);
    if (u_rc != UUID_RC_OK)
	goto failed;
    assert(NULL != vp);
    s = str_n_from_c((char *)vp, UUID_LEN_STR);
    free(vp);
    return s;
}

#elif defined(UUID_IS_E2FS)

static string_ty *
make_uuid(void)
{
    uuid_t          uuid;
    char            uu[37];

    uu[0] = '\0';
    uuid_generate(uuid);
    uuid_unparse(uuid, uu);
    assert(*uu);
    if (!*uu)
	return make_uuid_random();
    return str_n_from_c(uu, 36);
}

#elif defined(UUID_IS_DCE)

static string_ty *
make_uuid(void)
{
    uuid_t	uu_identifier;
    uint32_t	status;
    char	*uu;
    string_ty	*ret;

    uu = 0;
    uuid_create(&uu_identifier, &status);
    switch (status)
    {
    case uuid_s_ok:
	uuid_to_string(&uu_identifier, &uu, &status);
	if (uuid_s_ok != status)
	    uu = 0;
	break;

    case uuid_s_bad_version:
    case uuid_s_invalid_string_uuid:
    case uuid_s_no_memory:
	break;
    }

    if (!uu && !*uu)
	ret = make_uuid_random();
    else
    {
	ret = str_n_from_c(uu, 36);
	//
	// From
	// http://www.opengroup.org/onlinepubs/9629399/uuid_to_string.htm
	//
	// Note: The RPC run-time system allocates memory for the
	// string returned in string_uuid. To deallocate the
	// memory, the application calls the rpc_string_free()
	// routine.
	//
	rpc_string_free(&uu, &status);
    }

    return ret;
}

#elif defined(UUID_IS_LINUX)

static string_ty *
make_uuid(void)
{
    int             fd;
    char            buffer[36];

    fd = open(LINUX_UUID_FILENAME, O_RDONLY);
    if (fd < 0)
    {
	failed:
	return make_uuid_random();
    }
    if (read(fd, buffer, 36) != 36)
    {
	close(fd);
	goto failed;
    }
    close(fd);
    return str_n_from_c(buffer, 36);
}

#else

static string_ty *
make_uuid(void)
{
    return make_uuid_random();
}

#endif


string_ty *
universal_unique_identifier(void)
{
    string_ty	    *ret;

    ret = make_uuid();
    assert(ret);
    return ret;
}


int
universal_unique_identifier_valid(string_ty *uuid)
{
    static const char uuid_charset[] = "0123456789abcdefABCDEF";
    int		    i;

    if (!uuid || uuid->str_length != 36)
	return 0;

    //
    // Syntax check
    //
    for (i = 0; i < 36; ++i)
    {
	switch (i)
	{
	case 8:
	case 13:
	case 18:
	case 23:
	    if (uuid->str_text[i] != '-')
		return 0;
	    break;

	default:
	    if (0 == strchr(uuid_charset, uuid->str_text[i]))
		return 0;
	    break;
	}
    }
    return 1;
}
