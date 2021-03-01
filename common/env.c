/*
 *	aegis - project change supervisor
 *	Copyright (C) 1990, 1991, 1992, 1993, 1994 Peter Miller.
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
 * MANIFEST: functions to manipulate environment variables
 */

#include <ac/stddef.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <env.h>
#include <error.h>
#include <mem.h>
#include <trace.h>


extern	char	**environ;
static	size_t	nenvirons;
static	int	initialized;


/*
 * NAME
 *	env_initialize - start up environment
 *
 * SYNOPSIS
 *	void env_initialize(void);
 *
 * DESCRIPTION
 *	The env_initialize function is used to copy all of the environment
 *	variables into dynamic memory, so that they may be altered by the
 *	env_set and env_unset functions.
 */

void
env_initialize(void)
{
    size_t	    j;
    char	    **old;

    if (initialized)
	    return;
    ++initialized;

    trace(("env_initialize()\n{\n"));
    nenvirons = 0;
    for (j = 0; environ[j]; ++j)
	++nenvirons;
    old = environ;
    environ = (char **)mem_alloc((nenvirons + 1) * sizeof(char *));
    for (j = 0; j < nenvirons; ++j)
    {
	char		*cp;
	char		*was;

	was = old[j];
	cp = mem_alloc(strlen(was) + 1);
	strcpy(cp, was);
	environ[j] = cp;
    }
    environ[nenvirons] = 0;
    env_set("SHELL", CONF_SHELL);
    trace((/*{*/"}\n"));
}


/*
 * NAME
 *	env_set - set environment variable
 *
 * SYNOPSIS
 *	void env_set(char *name, char *value);
 *
 * DESCRIPTION
 *	The env_set function is used to set the given environment variable to
 *	the given value.
 *
 * CAVEAT
 *	Assumes that the env_initialize function has already been called.
 */

void
env_set(char *name, char *value)
{
    size_t	    name_len;
    size_t	    j;
    char	    *cp;
    size_t	    nbytes;

    trace(("env_set(name = \"%s\", value = \"%s\")\n{\n", name, value));
    if (!initialized)
	env_initialize();
    cp = 0;
    name_len = strlen(name);
    nbytes = name_len + strlen(value) + 2;
    for (j = 0; j < nenvirons; ++j)
    {
	cp = environ[j];
	assert(cp);
	if
	(
    	    !memcmp(cp, name, name_len)
	&&
    	    (cp[name_len] == '=' || !cp[name_len])
	)
    	    break;
    }
    if (j < nenvirons)
    {
	environ[j] = mem_change_size(environ[j], nbytes);
	cp = environ[j];
    }
    else
    {
	environ = mem_change_size(environ, (nenvirons + 2) * sizeof(char *));
	environ[nenvirons] = mem_alloc(nbytes);
	cp = environ[nenvirons];
	++nenvirons;
	environ[nenvirons] = 0;
    }
    memcpy(cp, name, name_len);
    cp[name_len] = '=';
    strcpy(cp + name_len + 1, value);
    trace(("}\n"));
}


/*
 * NAME
 *	env_unset - remove environment variable
 *
 * SYNOPSIS
 *	void env_unset(char *name);
 *
 * DESCRIPTION
 *	The env_unset function is used to remove the named variable from the
 *	environment.
 *
 * RETURNS
 *	void
 *
 * CAVEAT
 *	Assumes that the env_initialize function has been called already.
 */

void
env_unset(char *name)
{
    size_t	    name_len;
    size_t	    j;
    char	    *cp;

    trace(("env_unset(name = \"%s\")\n{\n", name));
    if (!initialized)
	env_initialize();
    name_len = strlen(name);
    cp = 0;
    for (j = 0; j < nenvirons; ++j)
    {
	cp = environ[j];
	assert(cp);
	if
	(
    	    !memcmp(cp, name, name_len)
	&&
    	    (cp[name_len] == '=' || !cp[name_len])
	)
    	    break;
    }
    if (!environ[j])
	return;
    environ[j] = 0;
    if (cp)
	mem_free(cp);
    --nenvirons;
    for ( ; j < nenvirons; ++j)
	environ[j] = environ[j + 1];
    trace(("}\n"));
}
