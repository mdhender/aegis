/*
 *	aegis - project change supervisor
 *	Copyright (C) 2004 Peter Miller;
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
 * MANIFEST: interface definition for aecvsserver/module.c
 */

#ifndef AECVSSERVER_MODULE_H
#define AECVSSERVER_MODULE_H

#include <str.h>

struct input_ty; /* forward */
struct server_ty; /* forward */
struct file_info_ty; /* forward */

/**
  * The module class is used to represent an abstract module.
  */
typedef struct module_ty module_ty;
struct module_ty
{
    const struct module_method_ty *vptr;
    int             reference_count;
    string_ty       *canonical_name;
};


/**
  * The module find method creates an instance of a suitable module,
  * based on the module name.  Use module_delete when you are done
  * with it.
  *
  * @param module_name
  *	The name of the module to search for.
  *
  * @returns
  *	A pointer to a module instance.
  */
module_ty *module_find(string_ty *module_name);


/**
  * The module find_trim method creates an instance of a suitable module,
  * based on the module name.  The module name may have a slash and
  * additional path appended to it; this will be trimmed befor the lookup
  * is performed via module_find.  Use module_delete when you are done
  * with it.
  *
  * @param module_name
  *	The name of the module to search for; extraneous path will be trimmed.
  *
  * @returns
  *	A pointer to a module instance.
  */
module_ty *module_find_trim(string_ty *module_name);

/**
  * The module_delete function is used to release the resources of a
  * module when you are done with it.
  */
void module_delete(module_ty *mp);

/**
  * The module::modified method is used to perform the appropriate
  * actions for a "Modified" request.
  *
  * @param mp
  *	The module to operate on.
  * @param sp
  *	The server to issue messages and errors to, plus additional
  *	context if required.
  * @param file_name
  *	The name of the file to operator on.  It is relative to the base
  *	of the named module.  It may contain slashes (/).
  * @param mode
  *	The permissions mode of the file.
  * @param contents
  *	An input source from which the contents of the file may be read.
  *	(You may safely ignore if you want to ignore the file contents,
  *	what you don't read will be thrown away.)
  */
void module_modified(module_ty *mp, struct server_ty *sp,
    string_ty *file_name, struct file_info_ty *, struct input_ty *contents);

/**
  * The module_bogus function is used to ask a module if it feels that
  * it is bogus.  Such modules return errors for all methods.
  */
int module_bogus(module_ty *);

/**
  * The module_name function is used to get the canonical name of the
  * given module.  DO NOT str_free() the results.
  */
string_ty *module_name(module_ty *);

/**
  * The module::checkout method is used to ask the module to perform
  * the appropriate actions for a "co" request.
  */
void module_checkout(module_ty *, struct server_ty *sp);

/**
  * The module::checkin method is used to ask the module to perform
  * the appropriate actions for a "ci" request.
  *
  * \param mp
  *     The module which handles this file.
  * \param sp
  *     The server which is handling this connection.
  * \param client_side
  *     The client-side name of the file being checked in.
  * \param server_side
  *     The server-side name of the file being checked in.
  * \returns
  *     zero (false) if unsuccessful, non-zero (true) is successful.
  */
int module_checkin(module_ty *mp, struct server_ty *sp, string_ty *client_side,
    string_ty *server_side);


typedef struct module_options_ty module_options_ty;
struct module_options_ty
{
    int             A;   /* Reset any sticky tags/date/kopts. */
    int             C;   /* Overwrite locally modified files with clean
                          * repository copies. */
    string_ty       *D;  /* Set date to update from (is sticky). */
    int             d;   /* Build directories, like checkout does. */
    int             f;   /* Force a head revision match if tag/date not
                          * found. */
    string_ty       *I;  /* More files to ignore (! to reset). */
    string_ty       *j;  /* Merge in changes made between current revision
                          * and rev. */
    string_ty       *k;  /* Use RCS kopt -k option on checkout. (is sticky) */
    int             l;   /* Local directory only, no recursion. */
    string_ty       *m;  /* Message to add to file or transction. */
    int             P;   /* Prune empty directories. */
    int             p;   /* Send updates to standard output (avoids
                          * stickiness). */
    int             R;   /* Process directories recursively. */
    string_ty       *r;  /* Update using specified revision/tag (is sticky). */
    string_ty       *W;  /* Wrappers specification line. */
};

void module_options_constructor(module_options_ty *);
void module_options_destructor(module_options_ty *);


/**
  * The module::update method is used to ask the module to perform the
  * appropriate actions for an "update" request.
  *
  * \param mp
  *     The module which handles this file.
  * \param sp
  *     The server which is handling this connection.
  * \param client_side
  *     The client-side name of the file or directory being updated.
  * \param server_side
  *     The server-side name of the file or directory being updated.
  * \param opt
  *     Options specified for this request.
  * \returns
  *     zero (false) if unsuccessful, non-zero (true) is successful.
  */
int module_update(module_ty *mp, struct server_ty *sp, string_ty *client_side,
    string_ty *server_side, module_options_ty *opt);

/**
  * The module::add method is used to ask the module to perform the
  * appropriate actions for an "add" request.
  *
  * \param mp
  *     The module which handles this file.
  * \param sp
  *     The server which is handling this connection.
  * \param client_side
  *     The client-side name of the file or directory being added.
  * \param server_side
  *     The server-side name of the file or directory being added.
  * \param opt
  *     Options specified for this request.
  * \returns
  *     zero (false) if unsuccessful, non-zero (true) is successful.
  */
int module_add(module_ty *mp, struct server_ty *sp, string_ty *client_side,
    string_ty *server_side, module_options_ty *opt);

/**
  * The module::remove method is used to ask the module to perform the
  * appropriate actions for an "remove" request.
  *
  * \param mp
  *     The module which handles this file.
  * \param sp
  *     The server which is handling this connection.
  * \param client_side
  *     The client-side name of the file or directory being removed.
  * \param server_side
  *     The server-side name of the file or directory being removed.
  * \param opt
  *     Options specified for this request.
  * \returns
  *     zero (false) if unsuccessful, non-zero (true) is successful.
  */
int module_remove(module_ty *mp, struct server_ty *sp, string_ty *client_side,
    string_ty *server_side, module_options_ty *opt);

/**
  * The is_update_prefix function is used to determine if a file is a
  * candidate for update.
  *
  * \param above
  *     The server relative name of a file or directory listed on the
  *     update command line.
  * \param below
  *     The server relative name of a file or directory to be tested to
  *     determine if it is an update candidate.
  * \param deep
  *     True if all files in all directories below the directory are
  *     candidates, and false if only files immeditately within the
  *     directoiry are candidates.
  */
int is_update_prefix(string_ty *above, string_ty *below, int deep);

#endif /* AECVSSERVER_MODULE_H */
