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

#ifndef AE_CVS_SERVER_NET_H
#define AE_CVS_SERVER_NET_H

#include <common/nstring.h>
#include <common/str_list.h>
#include <libaegis/input.h>
#include <libaegis/output.h>

#include <aecvsserver/directory.h>

enum response_code_ty
{
    response_code_Checked_in,
    response_code_Checksum,
    response_code_Clear_static_directory,
    response_code_Clear_sticky,
    response_code_Copy_file,
    response_code_Created,
    response_code_E,
    response_code_error,
    response_code_F,
    response_code_hate,
    response_code_love,
    response_code_M,
    response_code_Mbinary,
    response_code_Merged,
    response_code_Mode,
    response_code_Mod_time,
    response_code_Module_expansion,
    response_code_MT,
    response_code_New_entry,
    response_code_Notified,
    response_code_ok,
    response_code_Patched,
    response_code_Rcs_diff,
    response_code_Removed,
    response_code_Remove_entry,
    response_code_Set_checkin_prog,
    response_code_Set_static_directory,
    response_code_Set_sticky,
    response_code_Set_update_prog,
    response_code_Template,
    response_code_Updated,
    response_code_Update_existing,
    response_code_Valid_requests,
    response_code_Wrapper_rcsOption,
    response_code_MAX // must be last
};

class output_ty; // forward
class response; // forward

/**
  * The net_ty class is used to remember the state of a network connection
  * to a client.
  */
class net_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~net_ty();

    /**
      * The default constructor.
      */
    net_ty();

    /**
      * The getline method is used to read one line from the input (up
      * to the next newline character or end of input).  The newline
      * is not included in the returned string.  Returns false if
      * end-of-file is reached.
      */
    bool getline(nstring &s);

    /**
      * The printf method is used to write output to the client.
      */
    void printf(const char *, ...)                            ATTR_PRINTF(2, 3);

    /**
      * The response_queue method is used to enqueue a response to be
      * transmitted at the next response_flush.
      */
    void response_queue(response *rp);

    /**
      * The response_flush method is used to flush any pending responses
      * (if any) to the client.
      */
    void response_flush(void);

    /**
      * The log_to_file method is used to enable logging of all requests
      * and responses.  This is for debugging.
      */
    void log_to_file(string_ty *);

    /**
      * The log_by_env method is used to enable logging if the given
      * environment variable has been set.  The value of the environment
      * variable is the name of the log file to use.
      */
    void log_by_env(const char *);

    /**
      * The argument method is used to append the given string to the
      * argument list.
      */
    void argument(string_ty *);

    /**
      * The argumentx method is used to append the given string to the
      * end of the last string on the argument list.
      */
    void argumentx(string_ty *);

    /**
      * The accumulator_reset method is used to discard the most
      * recently accumulated lists of directories, entries and arguments.
      */
    void accumulator_reset(void);

    /**
      * The file_info_find method is used to locate the file info
      * structure for a particular root-relative file name.
      *
      * @param server_side
      *     The server-side name of the file, including the module name,
      *     but excluding ROOT_PATH/
      * @param auto_alloc
      *     True if the name should be allocated if not foind, false if it
      *     should not.
      * @returns
      *     The corresponding file_info_ty data structure.  Returns NULL if
      *     auto_alloc was false, and the necessary data was not found.
      */
    struct file_info_ty *file_info_find(string_ty *server_side, int auto_alloc);

    /**
      * The directory_set method may be used to set the directory,
      * as given in in a Directory request.
      */
    void directory_set(string_ty *cs, string_ty *ss);

    /**
      * The directory_find_client_side method is used to search the
      * existing Directory names, looking for the given client-side directory.
      * Returns NULL if not found.  Do not free the result.
      */
    directory_ty *directory_find_client_side(string_ty *);

    /**
      * The directory_find_server_side method is used to search the
      * existing Directory names, looking for the given server-side directory.
      * Returns NULL if not found.  Do not free the result.
      */
    directory_ty *directory_find_server_side(string_ty *);

    /**
      * The get_is_rooted method is used to find out if we have seet a
      * Root request yet.
      */
    bool get_is_rooted() const { return rooted; }

    /**
      * The set_is_rooted method is used by the Root request to indicate
      * that a valid Root request has been processed.
      */
    void set_is_rooted() { rooted = true; }

    /**
      * The set_respose_valid method is used the the Valid-Response
      * request to indicate those responses which are valid for this
      * client.
      */
    void set_response_valid(int n) { response_valid[n] = true; }

    /**
      * The get_updating_verbose method is used to obtain the last
      * client-side directory we claimed to be updating.
      */
    string_ty *get_updating_verbose() const { return updating_verbose; }

    /**
      * The get_updating_verbose method is used to set the
      * client-side directory we are currently updating.
      */
    void set_updating_verbose(string_ty *);

    /**
      * The in_crop method is used to create a new input source which
      * reads the next \a length bytes.
      *
      * @param length
      *     The number of bytes to be consumed.
      * @returns
      *     an input stream, use it in the usual way.
      */
    input in_crop(long length);

    directory_ty *get_curdir() const { return curdir; }
    bool curdir_is_set() const { return (curdir != 0); }

    size_t argument_count() const { return argument_list.size(); }
    string_ty *argument_nth(size_t n) const { return argument_list[n]; }

private:
    input in;
    output::pointer out;
    output::pointer log_client;

    /**
      * The rooted instance variable is used to remeber whether we have
      * seet a Root request yet, or not.
      */
    bool rooted;

    /**
      * The set of responses which are currently valid.
      */
    bool response_valid[response_code_MAX];

    //
    // The response queue, the list of responses yet to be sent to
    // the client.
    //
    size_t response_queue_length;
    size_t response_queue_max;
    response **response_queue_item;

    //
    // The set of directories accumulated by Directory requests until
    // something eats them all, indexed by client-side path.
    //
    struct symtab_ty *dir_info_cs;

    //
    // The set of directories accumulated by Directory requests until
    // something eats them all, indexed by server-side path.
    //
    struct symtab_ty *dir_info_ss;

    //
    // The most recent Directory request.
    //
    directory_ty *curdir;

    //
    // The file_info field is used to remember a table of information
    // about files, indexed by root-relative file name.
    //
    struct symtab_ty *file_info;

    //
    // The list of strings accumulated by Argument and Argumentx requests
    // until something eats them all.
    //
    string_list_ty argument_list;

    /**
      * The updating_verbose instance variable is used to remember the
      * last client-side directory we claimed to be updating.
      */
    string_ty *updating_verbose;

private:
    /**
      * The copy constructor.  Do not use.
      */
    net_ty(const net_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    net_ty &operator=(const net_ty &);
};

#endif // AE_CVS_SERVER_NET_H
