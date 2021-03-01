//
// aegis - project change supervisor
// Copyright (C) 2005, 2006, 2008, 2012 Peter Miller
// Copyright (C) 2004, 2008 Walter Franzini;
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
#include <common/ac/dce/rpc.h>
#include <common/ac/fcntl.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/unistd.h>
#include <common/ac/uuid.h>

#include <common/r250.h>
#include <common/str.h>
#include <common/uuidentifier.h>


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
    // top 2 bits are invariant
    buffer[19] = charset[8 | (r250() & 3)];

    return str_n_from_c(buffer, 36);
}


#if HAVE_UUID_CREATE && HAVE_UUID_MAKE && HAVE_UUID_EXPORT

static string_ty *
make_uuid(void)
{
    uuid_t *uuid = 0;
    uuid_rc_t u_rc = uuid_create(&uuid);
    if (u_rc != UUID_RC_OK)
    {
        failed:
        return make_uuid_random();
    }
    u_rc = uuid_make(uuid, UUID_MAKE_V4);
    if (u_rc != UUID_RC_OK)
        goto failed;
    void *vp = NULL;
    u_rc = uuid_export(uuid, UUID_FMT_STR, &vp, NULL);
    if (u_rc != UUID_RC_OK)
        goto failed;
    assert(NULL != vp);
    string_ty *s1 = str_n_from_c((char *)vp, UUID_LEN_STR);
    uuid_destroy(uuid);
    free(vp);
    string_ty *s2 = str_downcase(s1);
    str_free(s1);
    return s2;
}

#elif HAVE_UUID_GENERATE && HAVE_UUID_UNPARSE

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
    string_ty *s1 = str_n_from_c(uu, 36);
    string_ty *s2 = str_downcase(s1);
    str_free(s1);
    return s2;
}

#elif HAVE_DCE_UUID_H && HAVE_UUID_CREATE && HAVE_UUID_TO_STRING

static string_ty *
make_uuid(void)
{
    uuid_t      uu_identifier;
    uint32_t    status;
    char        *uu;

    uu = 0;
    uuid_create(&uu_identifier, &status);
    switch (status)
    {
    case uuid_s_ok:
        uuid_to_string(&uu_identifier, &uu, &status);
        if (uuid_s_ok != status)
        {
            do_it_ourselves:
            return make_uuid_random();
        }
        break;

    case uuid_s_bad_version:
    case uuid_s_invalid_string_uuid:
    case uuid_s_no_memory:
        goto do_it_ourselves;
    }

    if (!uu || !*uu)
        goto do_it_ourselves;

    string_ty *s1 = str_n_from_c(uu, 36);

    //
    // From
    // http://www.opengroup.org/onlinepubs/9629399/uuid_to_string.htm
    //
    // Note: The RPC run-time system allocates memory for the
    // string returned in string_uuid. To deallocate the
    // memory, the application calls the rpc_string_free()
    // routine.
    //
    int ignore;
    rpc_string_free(&uu, &ignore);

    string_ty *s2 = str_downcase(s1);
    str_free(s1);
    return s2;
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
    string_ty *s1 = str_n_from_c(buffer, 36);
    string_ty *s2 = str_downcase(s1);
    str_free(s1);
    return s2;
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
    string_ty       *ret;

    ret = make_uuid();
    assert(ret);
    return ret;
}


static const char uuid_charset[] = "0123456789abcdefABCDEF";


bool
universal_unique_identifier_valid(string_ty *uuid)
{
    return universal_unique_identifier_valid(nstring(uuid));
}


bool
universal_unique_identifier_valid(const nstring &uuid)
{
    return
        (
            uuid.size() == 36
        &&
            universal_unique_identifier_valid_partial(uuid)
        );
}


bool
universal_unique_identifier_valid_partial(string_ty *uuid)
{
    return universal_unique_identifier_valid_partial(nstring(uuid));
}


bool
universal_unique_identifier_valid_partial(const nstring &uuid)
{
    if (uuid.size() < 4 || uuid.size() > 36)
        return false;

    //
    // Syntax check
    //
    for (size_t i = 0; i < uuid.size(); ++i)
    {
        switch (i)
        {
        case 8:
        case 13:
        case 18:
        case 23:
            if (uuid[i] != '-')
                return false;
            break;

        default:
            if (0 == strchr(uuid_charset, uuid[i]))
                return false;
            break;
        }
    }
    return true;
}


// vim: set ts=8 sw=4 et :
