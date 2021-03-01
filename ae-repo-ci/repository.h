//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
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

#ifndef AE_VC_CHECKIN_REPOSITORY_H
#define AE_VC_CHECKIN_REPOSITORY_H

#include <libaegis/change.h>

class nstring; // forward
class change_identifier; // forward
class fstate_src_ty; // forward

/**
  * The abstract repository base class is used to represent a generic
  * interface to the check-in facilities of a version control
  * repository.
  *
  * The repository may or may not have a change-set concepts, but it
  * will be treated as if it did.
  */
class repository
{
public:
    /**
      * The destructor.
      */
    virtual ~repository();

    /**
      * The default constructor.
      */
    repository();

    /**
      * The checkout method is used to set-up for the commit.  All know
      * examples at this time require the creation of a work area to
      * apply the changes to.
      *
      * This method MUST be called before any of the following methods.
      *
      * @param module
      *     This is the name of the module to be checked out.
      * @param directory
      *     This is the absolute path of the directory to use to create
      *     the temporary work area below.
      */
    virtual void checkout(const nstring &module, const nstring &directory) = 0;

    /**
      * The get_directory method is used to obtain the absolute path of
      * the directory in which the checkout occurred.
      */
    virtual nstring get_directory() const = 0;

    /**
      * The remove_file method is used to remove a file from the work
      * area, and instruct the repository that the file is deleted in
      * this change set.
      *
      * Some repositories require that the file be deleted from the work
      * area first, and some do not.  Any repository specific actions
      * will be performed by this method.
      *
      * @param file_name
      *     The name of the file (relative to the checkout directory,
      *     relative to the Aegis baseline root) to be removed.
      */
    virtual void remove_file(const nstring &file_name) = 0;

    /**
      * The remove_file_wrapper method is used to perform all of the
      * checks against the checked-out directory to keep the remote
      * repository in synch with the Aegis one.  This may involve
      * rewriting the file actions.
      *
      * @param cid
      *     The change identification
      * @param src
      *     The file attributes
      */
    void remove_file_wrapper(change_identifier &cid, fstate_src_ty *src);

    /**
      * The add_file method is used to add a file to the work area, and
      * instruct the repository that the file is added in this change
      * set.
      *
      * Some repositories require that the file be created in the work
      * area first, and some do not.  Any repository specific actions
      * will be performed by this method.
      *
      * @param file_name
      *     The name of the file (relative to the checkout directory,
      *     relative to the Aegis baseline root) to be added.
      * @param content_path
      *     The absolute path of the file contents to be copied into the
      *     work area.
      */
    virtual void add_file(const nstring &filename,
	const nstring &content_path) = 0;

    /**
      * The add_file_wrapper method is used to perform all of the checks
      * against the checked-out directory to keep the remote repository
      * in synch with the Aegis one.  This may involve rewriting the
      * file actions.
      *
      * @param cid
      *     The change identification
      * @param src
      *     The file attributes
      */
    void add_file_wrapper(change_identifier &cid, fstate_src_ty *src);

    /**
      * The modify_file method is used to modify a file to the work
      * area, and instruct the repository that the file is modified in
      * this change set.
      *
      * Some repositories require that the file be modified in the work
      * area first, and some do not.  Any repository specific actions
      * will be performed by this method.
      *
      * @param file_name
      *     The name of the file (relative to the checkout directory,
      *     relative to the Aegis baseline root) to be added.
      * @param content_path
      *     The absolute path of the file contents to be copied into the
      *     work area.
      */
    virtual void modify_file(const nstring &filename,
	const nstring &content_path) = 0;

    /**
      * The modify_file_wrapper method is used to perform all of the
      * checks against the checked-out directory to keep the remote
      * repository in synch with the Aegis one.  This may involve
      * rewriting the file actions.
      *
      * @param cid
      *     The change identification
      * @param src
      *     The file attributes
      */
    void modify_file_wrapper(change_identifier &cid, fstate_src_ty *src);

    /**
      * The rename_file method is used to rename a file in the work
      * area, and instruct the repository that the file is renamed in
      * this change set.
      *
      * Some repositories require that the file be renamed in the work
      * area first, and some do not.  Any repository specific actions
      * will be performed by this method.
      *
      * The default implementation of this method is to call the
      * remove_file method and the add_file method, as a generic
      * action covering all repositories which do not support rename
      * explicitly.
      *
      * @param old_file_name
      *     The name of the file (relative to the checkout directory,
      *     relative to the Aegis baseline root) to be removed.
      * @param new_file_name
      *     The name of the file (relative to the checkout directory,
      *     relative to the Aegis baseline root) to be added.
      * @param content_path
      *     The absolute path of the file contents to be copied into the
      *     work area.
      */
    virtual void rename_file(const nstring &old_filename,
	const nstring &new_file_name, const nstring &content_path);

    /**
      * The rename_file_wrapper method is used to perform all of the
      * checks against the checked-out directory to keep the remote
      * repository in synch with the Aegis one.  This may involve
      * rewriting the file actions.
      *
      * @param cid
      *     The change identification
      * @param src
      *     The file attributes
      */
    void rename_file_wrapper(change_identifier &cid, fstate_src_ty *src);

    /**
      * The file_attributes method is used to attached versioned
      * attributes to the file.
      *
      * The default implementation does nothing.
      *
      * @param cid
      *     The change identification
      * @param src
      *     The file attributes
      */
    void file_attributes(change_identifier &cid, fstate_src_ty *src);

    /**
      * The file_wrapper method is used to perform all of the
      * checks against the checked-out directory to keep the remote
      * repository in synch with the Aegis one.  This may involve
      * rewriting the file actions.
      *
      * This method calls one of the create_file_wrapper,
      * modify_file_wrapper, remove_filea_wrapper or rename_file_wrapper
      * methods, and also the file_attributes method.
      *
      * @param cid
      *     The change identification
      * @param src
      *     The file attributes
      */
    void file_wrapper(change_identifier &cid, fstate_src_ty *src);

    /**
      * The commit method is used to commit all the changes in the
      * temorary work area into the repository.
      *
      * On success, the temporary work area will be completely removed.
      *
      * @param comment
      *     The comment to be applied to the change set when it is
      *     committed to the repository.  This should only be bound to
      *     individual files when the repository is completely unable
      *     to attach comments to the change set (often, because the
      *     repository doesn't actually *have* change sets).
      */
    virtual void commit(const nstring &comment) = 0;

    /**
      * The factory class method is used to create a new repository
      * instance based on the type of the repository.
      *
      * @param type
      *     The type of repository to be operated upon.
      * @returns
      *     a pointer to a dynamically allocated instance of a
      *     repository object.
      */
    static repository *factory(const char *type);

    /**
      * The list class method is used to list all of the currently
      * supported repository types on the standard output.
      */
    static void list(void);

    /**
      * The change_specific_attributes method is used to give
      * repositories the opportunity to use change attributes and
      * project attributes to influence the behaviour.
      *
      * While you can use any and all of the change and project state,
      * it is recommended that you limit yourself to the change
      * attributes and project specific attributes.
      *
      * The default implementation does nothing.
      */
    virtual void change_specific_attributes(change::pointer cp);

protected:
    /**
      * The is_binary class method is used to determine whether or not
      * a given file is binary.  International text can contain many
      * things, but it can't contain NUL characters.  The presence of
      * NUL characters indicates that a file is binary.
      *
      * @param path
      *     The path of the file to test
      * @returns
      *     bool; true if file is binary, false if it could be text
      */
    static bool is_binary(const nstring &path);

    /**
      * The file_attribute method is used to attached versioned
      * attributes to a file.  The default implementation does nothing.
      *
      * @param filename
      *     The name of the file the attribute is to be attached to
      * @param attribute_name
      *     The name of the file attribute
      * @param attribute_value
      *     The value of the attribute
      */
    virtual void file_attribute(const nstring &filename,
        const nstring &attribute_name, const nstring &attribute_value);

private:
    /**
      * The copy constructor. Do not use.
      */
    repository(const repository &);

    /**
      * The assignment operator. Do not use.
      */
    repository &operator=(const repository &);
};

#endif // AE_VC_CHECKIN_REPOSITORY_H
