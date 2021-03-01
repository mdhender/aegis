//
//	aegis - project change supervisor
//	Copyright (C) 1995, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate languages
//

#include <common/ac/libintl.h>
#include <common/ac/locale.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/env.h>
#include <common/error.h>
#include <common/language.h>
#include <common/libdir.h>
#include <common/mem.h>

#ifdef DEBUG

enum state_ty
{
    state_uninitialized,
    state_C,
    state_human
};

static state_ty state;

#endif // DEBUG

static bool     setlocale_is_working;
static int      message_catalog_set;

//
// This is the lost of locale names to try if the default "" setlocale fails.
//
static const char *const trial_locales[] =
{
    "en_US",
    "en",
    "en_US.UTF-8",
    "en_US.utf8",
    "en_US.ISO8859-1",
    "en_US.iso8859-1",
};


#if defined(HAVE_SETLOCALE) && defined(HAVE_GETTEXT)
//
// NAME
//	setlocale_wrapper - common setlocale processing
//
// DESCRIPTION
//	The setlocale_wrapper function is used to call the setlocale
//	function, with defualt arguments of
//
//		setlocale(LC_ALL, "")
//
//	The function return true (non-zero) if setlocale succeeds,
//	and false (zero) if setlocale fails.
//
//	When setlocale returns "C" or "POSIX" this is considered a
//	failure, because it will result in the short form of the error
//	messages being used.  The return value will be 0 in these
//	cases, too.
//

static bool
setlocale_wrapper(void)
{
    //
    // At this time (Oct-2004) Cygwin only partially implements
    // localisation.  Returning a hard coded "yes" here at least results
    // in long error messages for now.
    //
#ifndef __CYGWIN__
    const char *cp = setlocale(LC_ALL, "");
    if (!cp)
	return false;
    if (0 == strcmp("C", cp) || 0 == strcmp("POSIX", cp))
	return false;
#endif // !defined(__CYGWIN__)
    return true;
}
#endif

//
// NAME
//	language_init - initialize language functions
//
// DESCRIPTION
//	The language_init function must be called at the start of the
//	program, to set the various locale features.
//
//	This function must be called after the setuid initialization.
//	If you forget to call me, all bets are off.
//

void
language_init(void)
{
    const char      *lib;
#if HAVE_SETLOCALE && HAVE_GETTEXT
    static const char package[] = "aegis";
#endif

    //
    // Protect against multiple invokation.
    //
#ifdef DEBUG
    if (state != state_uninitialized)
	fatal_raw("language_init() called more than once (bug)");
    state = state_C;
#endif

    //
    // Set the locale to the default (as defined by the environment
    // variables) and set the message domain information.
    //
    lib = getenv("AEGIS_MESSAGE_LIBRARY");
    if (!lib || !*lib)
	lib = configured_nlsdir();
    else
        message_catalog_set = 1;

    setlocale_is_working = false;
#ifdef HAVE_SETLOCALE
#ifdef HAVE_GETTEXT
    setlocale_is_working = setlocale_wrapper();
    if (!setlocale_is_working)
    {
	char            *lc_all;
	size_t          j;

	//
        // Remember the LC_ALL value in case we have to reset it.
	//
	lc_all = getenv("LC_ALL");
	if (lc_all)
	    lc_all = mem_copy_string(lc_all);

	//
	// Try the list of alternatives for the English messages.
	//
        // We set the LC_ALL environment variable, because it trumps
        // the LC_xxx and LANG environment variables.
	//
	for (j = 0; j < SIZEOF(trial_locales); ++j)
	{
	    env_set("LC_ALL", trial_locales[j]);
	    setlocale_is_working = setlocale_wrapper();
	    if (setlocale_is_working)
		break;
	}

	//
	// If we didn't find a working locale, restore the LC_ALL setting.
	//
	if (!setlocale_is_working && lc_all)
	{
	    env_set("LC_ALL", lc_all);
	    mem_free(lc_all);
	}
	else
	    env_unset("LC_ALL");
    }
    bindtextdomain(package, lib);
    textdomain(package);
#endif // HAVE_GETTEXT

    //
    // set the main body of the program use use the C locale
    //
    setlocale(LC_ALL, "C");
#endif // HAVE_SETLOCALE
}


//
// NAME
//	language_human - set for human conversation
//
// DESCRIPTION
//	The language_human function must be called to change the general
//	mode over to the default locale (usually dictated by the LANG
//	environment variable, et al).
//
//	The language_human and language_C functions MUST bracket human
//	interactions, otherwise the mostly-english C locale will be
//	used.  The default locale through-out the program is otherwise
//	assumed to be C.
//

void
language_human(void)
{
    if (!setlocale_is_working)
	return;
#ifdef DEBUG
    switch (state)
    {
    case state_uninitialized:
	fatal_raw("you must call language_init() in main (bug)");

    case state_human:
	fatal_raw("unbalanced language_human() call (bug)");

    case state_C:
	break;
    }
    state = state_human;
#endif
#ifdef HAVE_SETLOCALE
#ifdef HAVE_GETTEXT
    //
    // only need to flap the locale about like this
    // if we are using the gettext function
    //
    setlocale(LC_ALL, "");
#endif // HAVE_GETTEXT
#endif // HAVE_SETLOCALE
}


//
// NAME
//	language_C - set for program conversation
//
// DESCRIPTION
//	The language_C function must be called to restore the locale to
//	C, so that all the non-human stuff will work.
//
//	The language_human and language_C functions MUST bracket human
//	interactions, otherwise the mostly-english C locale will be
//	used.  The default locale through-out the program is otherwise
//	assumed to be C.
//

void
language_C(void)
{
    if (!setlocale_is_working)
	return;
#ifdef DEBUG
    switch (state)
    {
    case state_uninitialized:
	fatal_raw("you must call language_init() in main (bug)");

    case state_C:
	fatal_raw("unbalanced language_C() call (bug)");

    case state_human:
	break;
    }
    state = state_C;
#endif
#ifdef HAVE_SETLOCALE
#ifdef HAVE_GETTEXT
    //
    // only need to flap the locale about like this
    // if we are using the gettext function
    //
    setlocale(LC_ALL, "C");
#endif // HAVE_GETTEXT
#endif // HAVE_SETLOCALE
}

#if HAVE_GETTEXT
static int
lang_set(void)
{
    const char      *cp;

    cp = getenv("LC_ALL");
    if (cp && *cp)
	return 1;
    cp = getenv("LC_MESSAGES");
    if (cp && *cp)
	return 1;
    cp = getenv("LANG");
    if (cp && *cp)
	return 1;
    return 0;
}
#endif

void
language_check_translations()
{
#ifndef SOURCE_FORGE_HACK
#if HAVE_GETTEXT
    static int      done;

    if (!done)
    {
	done = 1;
	if (!message_catalog_set)
	{
	    //
            // Only whinge about this stuff if they *haven't* messed
            // with the AEGIS_MESSAGE_LIBRARY environment varaible.
	    //
	    if (setlocale_is_working)
	    {
		char            *cp;

		language_human();
		cp = gettext("");
		if (!cp || !*cp)
		{
		    if (lang_set())
		    {
			error_raw
			(
                            "Warning: You are seeing the short form "
                            "of the error messages.  The message "
                            "catalogues may not have been installed "
                            "correctly, or you may need to check "
                            "the settings of your LC_ALL and LANG "
                            "environment variables."
			);
		    }
		    else
		    {
			error_raw
			(
                            "Warning: You are seeing the short form "
                            "of the error messages.  Longer and "
                            "more informative error messages are "
                            "available using the English message "
                            "catalogue available via the GNU Gettext "
                            "facilities; use the command \"LANG=en_US; "
                            "export LANG\" to enable them.  The "
                            "message catalogues (including the English "
                            "catalogue) may not have been installed "
                            "correctly.  A number of alternate message "
                            "catalogues in other languages are also "
                            "available."
			);
		    }
		}
		language_C();
	    }
	    else
	    {
		error_raw
		(
		    "Warning: The setlocale() function failed.  "
		    "This results in you seeing the short form of the "
		    "error messages.  You may need to check the settings "
		    "of your LC_ALL and LANG environment variables."
#ifdef HAVE_LOCALE_PROG
		    "  Use the \"locale -a\" command to obtain a list "
		    "of valid locales."
#ifndef HAVE_LOCALE_GEN_PROG
		    "  If the list is very short, you may have a glibc "
		    "install problem."
#endif
#endif
#ifdef HAVE_LOCALE_GEN_PROG
		    "  See the locale-gen(8) man page for how to create "
		    "additional locales."
#endif
		);
	    }
	}
    }
#endif // HAVE_GETTEXT
#endif // !SOURCE_FORGE_HACK
}
