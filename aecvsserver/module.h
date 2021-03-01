//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#ifndef AECVSSERVER_MODULE_H
#define AECVSSERVER_MODULE_H

#include <common/str.h>
#include <libaegis/input.h>

class module_ty; // forward

/**
  * The module class is a "smart pointer" to a reference counted
  * dynamically allocated module_ty instance.
  */
class module
{
public:
    /**
      * The destructor.
      *
      * It is not virtual; thou shalt not derive from this class.
      */
    ~module();

    /**
      * The default constructor.
      */
    module() : ref(0) { }

    /**
      * The constructor.
      *
      * @param arg
      *     the module_ty instance to be managed.  The reference count
      *     is NOT increaed, it is assumed that you are gibing the smart
      *     pointer the "dumb" pointer to look after.
      */
    module(module_ty *arg);

    /**
      * The copy constructor.
      */
    module(const module &);

    /**
      * The assignment operator.
      */
    module &operator=(const module &);

    /**
      * This is why it's called a smart pointer: it passes averything to
      * the real instance for all the heavy lifting.
      */
    inline module_ty *
    operator->()
    {
	return ref;
    }

    /**
      * This is why it's called a smart pointer: it passes averything to
      * the real instance for all the heavy lifting.
      */
    inline const module_ty *
    operator->()
	const
    {
	return ref;
    }

    /**
      * The find class method creates an instance of a suitable module,
      * based on the module name.  Use module_delete when you are done
      * with it.
      *
      * @param module_name
      *     The name of the module to search for.
      *
      * @returns
      *     A pointer to a module instance.
      */
    static module find(string_ty *module_name);

    /**
      * The find_trim class method creates an instance of a
      * suitable module, based on the module name.  The module name may
      * have a slash and additional path appended to it; this will be
      * trimmed before the lookup is performed via module_find.  Use
      * the reference_count_down method when you are done with it.
      *
      * @param module_name
      *     The name of the module to search for; extraneous path will
      *     be trimmed.
      *
      * @returns
      *     A pointer to a module instance.
      */
    static module find_trim(string_ty *module_name);

private:
    /**
      * The ref instance variable is used to remember the memory_ty
      * instance being managed.
      */
    module_ty *ref;

    bool valid() const;
};

struct server_ty; // forward
struct file_info_ty; // forward

/**
  * The module class is used to represent an abstract module.
  */
class module_ty
{
protected:
    /**
      * The destructor.
      *
      * Not to be called directly.  Use the reference_count_down method
      * instead.
      */
    virtual ~module_ty();

    /**
      * The default constructor.
      */
    module_ty();

public:
    /**
      * The modified method is used to perform the appropriate actions
      * for a "Modified" request.
      *
      * @param sp
      *     The server to issue messages and errors to, plus additional
      *     context if required.
      * @param file_name
      *     The name of the file to operator on.  It is relative to the
      *     base of the named module.  It may contain slashes (/).
      * @param mode
      *     The permissions mode of the file.
      * @param contents
      *     An input source from which the contents of the file may be
      *     read.  (You may safely ignore if you want to ignore the file
      *     contents, what you don't read will be thrown away.)
      */
    virtual void modified(server_ty *sp, string_ty *file_name,
	file_info_ty *fip, input &contents) = 0;

    /**
      * The is_bogus method is used to ask a module if it feels that it is
      * bogus.  Such modules return errors for all methods.
      */
    virtual bool is_bogus() const;

    /**
      * The name method is used to get the canonical name of the given
      * module.  DO NOT str_free() the results.
      */
    string_ty *name();

    void reference_count_up();

    void reference_count_down();

    bool reference_count_valid() const { return (reference_count > 0); }

    /**
      * The checkout method is used to ask the module to perform the
      * appropriate actions for a "co" request.
      *
      * It actually calls the update method with the appropriate magic
      * arguments.
      */
    void checkout(server_ty *sp);

    /**
      * The checkin method is used to ask the module to perform the
      * appropriate actions for a "ci" request.
      *
      * \param sp
      *     The server which is handling this connection.
      * \param client_side
      *     The client-side name of the file being checked in.
      * \param server_side
      *     The server-side name of the file being checked in.
      * \returns
      *     bool; false if unsuccessful, true if successful.
      */
    virtual bool checkin(server_ty *sp, string_ty *client_side,
	string_ty *server_side) = 0;

    struct options
    {
	~options();
	options();
	options(const options &);
	options &operator=(const options &);

	int             A;   // Reset any sticky tags/date/kopts.
	int             C;   // Overwrite locally modified files with clean
			     // repository copies.
	string_ty       *D;  // Set date to update from (is sticky).
	int             d;   // Build directories, like checkout does.
	int             f;   // Force a head revision match if tag/date not
			     // found.
	string_ty       *I;  // More files to ignore (! to reset).
	string_ty       *j;  // Merge in changes made between current revision
			     // and rev.
	string_ty       *k;  // Use RCS kopt -k option on checkout. (is sticky)
	int             l;   // Local directory only, no recursion.
	string_ty       *m;  // Message to add to file or transction.
	int             P;   // Prune empty directories.
	int             p;   // Send updates to standard output (avoids
			     // stickiness).
	int             R;   // Process directories recursively.
	string_ty       *r;  // Update using specified revision/tag (is sticky).
	string_ty       *W;  // Wrappers specification line.
    };

    /**
      * The update method is used to ask the module to perform the
      * appropriate actions for an "update" request.
      *
      * \param sp
      *     The server which is handling this connection.
      * \param client_side
      *     The client-side name of the file or directory being updated.
      * \param server_side
      *     The server-side name of the file or directory being updated.
      * \param opt
      *     Options specified for this request.
      * \returns
      *     bool; false if unsuccessful, true if successful.
      */
    virtual bool update(server_ty *sp, string_ty *client_side,
	string_ty *server_side, const options &opt) = 0;

    /**
      * The add method is used to ask the module to perform the
      * appropriate actions for an "add" request.
      *
      * \param sp
      *     The server which is handling this connection.
      * \param client_side
      *     The client-side name of the file or directory being added.
      * \param server_side
      *     The server-side name of the file or directory being added.
      * \param opt
      *     Options specified for this request.
      * \returns
      *     bool; false if unsuccessful, true is successful.
      */
    virtual bool add(server_ty *sp, string_ty *client_side,
	string_ty *server_side, const options &opt) = 0;

    /**
      * The remove method is used to ask the module to perform the
      * appropriate actions for an "remove" request.
      *
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
    virtual bool remove(server_ty *sp, string_ty *client_side,
	string_ty *server_side, const options &opt) = 0;

protected:
    /**
      * The name method is used to get the canonical name of the given
      * module.  DO NOT str_free() the results.
      */
    virtual string_ty *calculate_canonical_name() const = 0;

private:
    /**
      * The reference_count instance variable is used to remember
      */
    int reference_count;

    /**
      * The canonical_name instance variable is used to remember
      */
    string_ty *canonical_name;

    /**
      * The copy constructor.  Do not use.
      */
    module_ty(const module_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    module_ty &operator=(const module_ty &);
};


/**
  * The is_update_prefix function is used to determine if a file
  * is a candidate for update.
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
bool is_update_prefix(string_ty *above, string_ty *below, bool deep);

#endif // AECVSSERVER_MODULE_H
