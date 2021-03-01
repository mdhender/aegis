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
 * MANIFEST: interface definition for ae-cvs-server/net.c
 */

#ifndef AE_CVS_SERVER_NET_H
#define AE_CVS_SERVER_NET_H

#include <directory.h>
#include <str_list.h>

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
    response_code_MAX /* must be last */
};
typedef enum response_code_ty response_code_ty;

/**
  * The net_ty class is used to remember the state of a network connection
  * to a client.
  */
typedef struct net_ty net_ty;
struct net_ty
{
    struct input_ty *in;
    struct output_ty *out;
    struct output_ty *log_client;
    int             rooted;

    /**
      * The set of responses which are currently valid.
      */
    char response_valid[response_code_MAX];

    /*
     * The response queue, the list of responses yet to be sent to
     * the client.
     */
    size_t response_queue_length;
    size_t response_queue_max;
    struct response_ty **response_queue_item;

    /*
     * The set of directories accumulated by Directory requests until
     * something eats them all, indexed by client-side path.
     */
    struct symtab_ty *dir_info_cs;

    /*
     * The set of directories accumulated by Directory requests until
     * something eats them all, indexed by server-side path.
     */
    struct symtab_ty *dir_info_ss;

    /*
     * The most recent Directory request.
     */
    directory_ty *curdir;

    /*
     * The file_info field is used to remember a table of information
     * about files, indexed by root-relative file name.
     */
    struct symtab_ty *file_info;

    /*
     * The list of strings accumulated by Argument and Argumentx requests
     * until something eats them all.
     */
    string_list_ty argument_list;

    /**
      * The updating_verbose instance variable is used to remember the
      * last client-side dirdctory we claimed to be updating.
      */
    string_ty *updating_verbose;
};

/**
  * The net_new function is used to allocate and initialize a new network
  * instance (using stdin and stdout).
  */
net_ty *net_new(void);

/**
  * The net_delete function is used to release the resources held by a
  * net instance when you are done with it.
  */
void net_delete(net_ty *);

/**
  * The net_getline function is used to read one line from the input
  * (up to the next newline character or end of input).  The newline is
  * not included in the returned string.  Returns NULL if end-of-file
  * is reached.
  */
string_ty *net_getline(net_ty *);

/**
  * The net_printf function is used to write output to the client.
  */
void net_printf(net_ty *, const char *, ...)                  ATTR_PRINTF(2, 3);

/**
  * The net_response_queue function is used to enqueue a response to be
  * transmaitted at the next net_response_flush.
  */
void net_response_queue(net_ty *, struct response_ty *);

/**
  * The net_response_flush function is used to flush any pending responses
  * (if any) to the client.
  */
void net_response_flush(net_ty *);

/**
  * The net_log_to_file function is used to enable logging of all requests
  * and responses.  This is for debugging.
  */
void net_log_to_file(net_ty *, string_ty *);

/**
  * The net_log_by_env function is used to enable logging if the given
  * environment variable has been set.  The value of the environment
  * variable is the name of the log file to use.
  */
void net_log_by_env(net_ty *, const char *);

/**
  * The net_argument function is used to append the given string to the
  * argument list.
  */
void net_argument(net_ty *, string_ty *);

/**
  * The net_argumentx function is used to append the given string to the
  * end of the last string on the argument list.
  */
void net_argumentx(net_ty *, string_ty *);

/**
  * The net_accumulator_reset function is used to discard the most
  * recently accumulated lists of directories, entries and arguments.
  */
void net_accumulator_reset(net_ty *);

/**
  * The net_file_info_find command is used to locate the file info
  * structure for a particular root-relative file name.
  *
  * @param np
  *     The network endpoint to operate on.
  * @param server_side
  *     The server-side name of the file, including the module name,
  *     but excluding ROOT_PATH/
  * @param auto_alloc
  *     True if the name should be allocated if not foind, false if it
  *     should not.
  * @returns
  *     The corresponding file_unfo_ty data structure.  Returns NULL if
  *     auto_alloc was false, and the necessary data was not found.
  */
struct file_info_ty *net_file_info_find(net_ty *np, string_ty *server_side,
    int auto_alloc);

/**
  * The net_directory_set function may be used to set the directory,
  * as given in in a Directory request.
  */
void net_directory_set(net_ty *, string_ty *, string_ty *);

/**
  * The net_directory_find_client_side function is used to search the
  * existing Directory names, looking for the given client-side directory.
  * Returns NULL if not found.  Do not free the result.
  */
directory_ty *net_directory_find_client_side(net_ty *, string_ty *);

/**
  * The net_directory_find_server_side function is used to search the
  * existing Directory names, looking for the given server-side directory.
  * Returns NULL if not found.  Do not free the result.
  */
directory_ty *net_directory_find_server_side(net_ty *, string_ty *);

#endif /* AE_CVS_SERVER_NET_H */
