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

#include <common/ac/crypt.h>
#include <common/ac/errno.h>
#include <common/ac/grp.h>
#include <common/ac/pwd.h>
#include <common/ac/shadow.h>
#include <common/ac/string.h>
#include <common/ac/unistd.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h> // for umask

#include <common/env.h>
#include <libaegis/os.h>
#include <aecvsserver/response/error.h>
#include <aecvsserver/response/hate.h>
#include <aecvsserver/response/love.h>
#include <aecvsserver/server/password.h>
#include <aecvsserver/server/private.h>
#include <aecvsserver/server/simple.h>
#include <aecvsserver/scramble.h>


struct server_password_ty
{
    server_ty       inherited;
    server_ty       *simple;
};


static void
destructor(server_ty *sp)
{
    server_password_ty *spp;

    spp = (server_password_ty *)sp;
    if (spp->simple)
    {
	server_delete(spp->simple);
	spp->simple = 0;
    }
}


static struct passwd *
check_system_password(const char *username, const char *password)
{
    struct passwd   *pw;
    char            *cp;

    //
    // Make sure the user exists.
    //
    pw = getpwnam(username);
    if (!pw)
	return 0;

    //
    // Don't even think about allowing root.
    //
    if (pw->pw_uid == 0)
	return 0;

    //
    // If this systejm has shadow passwords, grab the shadow password.
    //
#ifdef HAVE_GETSPNAM
    {
	struct spwd     *spw;

	spw = getspnam(username);
	if (spw)
	    pw->pw_passwd = spw->sp_pwdp;
    }
#endif

    //
    // Allow for dain bramaged HPUX passwd aging
    //  - Basically, HPUX adds a comma and some data
    //    about whether the passwd has expired or not
    //    on the end of the passwd field.
    //  - This code replaces the ',' with '\0'.
    //
    // I'm guessing that HPUX WANTED other systems to think the password
    // was wrong so logins would fail if the system didn't handle expired
    // passwds and the passwd might be expired.
    //
    cp = pw->pw_passwd;
    while (*cp)
    {
	if (*cp == ',')
	{
	    *cp = 0;
	    break;
	}
	++cp;
    }

    //
    // If the user has no password, we are done.
    //
    if (pw->pw_passwd[0] == 0)
    {
	if (password[0])
	    return 0;
	return pw;
    }

    //
    // Check the password.
    //
#if HAVE_CRYPT
    if (0 != strcmp(pw->pw_passwd, crypt(password, pw->pw_passwd)))
	return 0;

    //
    // Report success by return the user data.
    //
    return pw;
#else
    return 0;
#endif
}


static void
run(server_ty *sp)
{
    server_password_ty *spp = (server_password_ty *)sp;

    //
    // cvsclient.texi:
    // The client connects, and sends the following:
    //
    //  + the string BEGIN AUTH REQUEST, a linefeed,
    //  + the cvs root, a linefeed,
    //  + the username, a linefeed,
    //  + the password trivially encoded (see Password scrambling), a
    //    linefeed,
    //  + the string END AUTH REQUEST, and a linefeed.
    //
    bool verify = false;
    bool ok = false;
    nstring s;
    if (!server_getline(sp, s))
    {
	protocol_failure:
	server_e(sp, "authentication protocol error");
	auth_failure:
	server_response_queue(sp, new response_hate());
	server_response_flush(sp);
	return;
    }
    if (!strcmp(s.c_str(), "BEGIN GSSAPI REQUEST"))
    {
	server_error(sp, "GSSAPI authentication not supported by this server");
	goto auth_failure;
    }
    if (!strcmp(s.c_str(), "BEGIN VERIFICATION REQUEST"))
    {
	ok = true;
	verify = true;
    }
    else
	ok = !strcmp(s.c_str(), "BEGIN AUTH REQUEST");
    if (!ok)
	goto protocol_failure;

    if (!server_getline(sp, s))
	goto protocol_failure;
    ok = !strcmp(s.c_str(), ROOT_PATH);
    if (!ok)
    {
	//
	// cvsclient.texi:
	// "The client must send the identical string for cvs root both
	// here and later in the Root request of the cvs protocol itself.
	// Servers are encouraged to enforce this restriction."
	//
        // We only allow one Root specification, exactly ROOT_PATH,
        // and we check it in both places.
	//
	server_e(sp, "%s: no such repository", s.c_str());
	goto auth_failure;
    }

    nstring user_name;
    if (!server_getline(sp, user_name))
	goto protocol_failure;
    nstring scrambled_password;
    if (!server_getline(sp, scrambled_password))
	goto protocol_failure;

    if (!server_getline(sp, s))
	goto protocol_failure;
    if (verify)
	ok = !strcmp(s.c_str(), "END VERIFICATION REQUEST");
    else
	ok = !strcmp(s.c_str(), "END AUTH REQUEST");
    if (!ok)
	goto protocol_failure;

    //
    // Check that the user name and password are acceptable.
    //
    nstring password = descramble(scrambled_password);
    struct passwd *pw =
	check_system_password(user_name.c_str(), password.c_str());
    if (!pw)
	goto auth_failure;

    //
    // Report success.
    //
    server_response_queue(sp, new response_love());
    server_response_flush(sp);

    if (!verify)
    {
#if HAVE_INITGROUPS
	if
	(
	    initgroups (pw->pw_name, pw->pw_gid) < 0
#ifdef EPERM
	&&
	    //
	    // Note that initgroups() only works as root.  But we do
	    // still want to report ENOMEM and whatever other errors
	    // initgroups() might dish up.
	    //
	    errno != EPERM
#endif
	)
	{
	    int             err;

	    //
	    // This could be a warning, but I'm not sure I see the point
	    // in doing that instead of an error given that it would happen
	    // on every connection.  We could log it somewhere and not tell
	    // the user.  But at least for now make it an error.
	    //
	    err = errno;
	    server_e(sp, "initgroups failed: %s", strerror(err));
	    goto auth_failure;
	}
#endif // HAVE_INITGROUPS

	//
	// Drop user privilege level.
	//
	if (setuid(pw->pw_uid) < 0)
	{
	    int             err;

	    err = errno;
	    server_e(sp, "setuid failed: %s", strerror(err));
	    goto auth_failure;
	}

	//
	// Let libaegis.a know we have changed uid.
	//
	os_become_reinit_mortal();

	//
	// Set a sensable umask.
	//
	umask(DEFAULT_UMASK);

	//
	// Set some environment variables.
	//
	env_set("LOGNAME", user_name.c_str());
	env_set("USER", user_name.c_str());

	//
	// Now run the simple server.
	//
	if (!spp->simple)
	    spp->simple = server_simple_new(sp->np);
	server_run(spp->simple);
    }
}


static server_method_ty vtbl =
{
    sizeof(server_password_ty),
    destructor,
    run,
    "password",
};


server_ty *
server_password_new(net_ty *np)
{
    server_ty       *sp;
    server_password_ty *spp;

    sp = server_new(&vtbl, np);
    spp = (server_password_ty *)sp;
    spp->simple = 0;
    return sp;
}
