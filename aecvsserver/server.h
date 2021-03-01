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

#ifndef AE_CVS_SERVER_SERVER_H
#define AE_CVS_SERVER_SERVER_H

#include <common/nstring.h>
#include <aecvsserver/net.h>

class response; // forward

#define ROOT_PATH "/aegis"

/**
  * The server_ty class is used to represent a CVS server connection
  * and authentication mode.
  */
struct server_ty
{
    // private:
    const struct server_method_ty *vptr;
    struct net_ty   *np;
};

/**
  * The server_ty::run method is used to run the server.  First
  * authentication will be performed, and then requests will be served.
  */
void server_run(server_ty *);

/**
  * The destructor (actually, the delete operator) releases resources
  * used by the server.  Use this when you are done with it.
  */
void server_delete(server_ty *);

/**
  * The server_response_queue function is used to enqueue a response to
  * be transmaitted at the next server_response_flush().
  */
void server_response_queue(server_ty *, response *);

/**
  * The server_response_flush function is used to flush any pending
  * responses (if any) to the client.
  */
void server_response_flush(server_ty *);

/**
  * The server_getline function is used to read one line from the client
  * (up to the next newline character or end of input).  The newline is
  * not included in the returned string.  Returns NULL if end-of-file
  * is reached.
  */
bool server_getline(server_ty *sp, nstring &result);

/**
  * The server_file_mode_get function is used to read a file mode line
  * from the client and parse it into a valid file mode.
  */
int server_file_mode_get(server_ty *);

/**
  * The server_file_contents_get function returns a pointer to an
  * input source suitabl for reading the contents of a file from.
  * Use input_delete when you are done with it.
  */
input server_file_contents_get(server_ty *);

/**
  * The server_error function is used to queue an ok response to
  * the client.
  */
void server_ok(server_ty *);

/**
  * The server_error function is used to queue an error response to
  * the client.
  */
void server_error(server_ty *, const char *fmt, ...)          ATTR_PRINTF(2, 3);

/**
  * The server_e function is used to queue an error response to
  * the client (it's informative, rather than fatal).
  */
void server_e(server_ty *, const char *fmt, ...)              ATTR_PRINTF(2, 3);

/**
  * The server_m function is used to queue an error response to
  * the client (it's stdout, rather than stderr).
  */
void server_m(server_ty *, const char *fmt, ...)              ATTR_PRINTF(2, 3);

/**
  * The server_root_required function is used to test if the Root request
  * has already been sent.  If it has not, an error response complaining
  * that the Root request has yet to be received.  if true (non-zero)
  * is returned, the calling request should short circuit.
  */
int server_root_required(server_ty *, const char *caption);

/**
  * The server_directory_required function is used to test if at least
  * one Directory request has already been sent.  If it has not, an
  * error response complaining that the Directory request has yet to
  * be received.  If true (non-zero) is returned, the calling request
  * should short circuit.
  */
int server_directory_required(server_ty *, const char *caption);

/**
  * The server_execute function is used to run a command and send the
  * stdout and stderr to the client.  If the exit status is not zero,
  * an error message will be sent to the client, too.
  *
  * @param sp
  *	Server context (client in particular) to run the command.
  * @param cmd
  *	This is the command to be executed.
  * @returns
  *	This function returns true (non-zero) on success, and false
  *     (zero) on failure.
  */
int server_execute(server_ty *sp, string_ty *cmd);

/**
  * The server_argument function is used to append the given string to
  * the argument list.
  */
void server_argument(server_ty *sp, string_ty *arg);

/**
  * The server_argument function is used to append the given string to
  * the last argument on the argument list.
  */
void server_argumentx(server_ty *sp, string_ty *arg);

/**
  * The server_accumulator_reset function is used to discard the most
  * recently accumulated lists of directories, entries and arguments.
  */
void server_accumulator_reset(server_ty *sp);

/**
  * The server_file_info_find function is used to obtain file info for
  * the given root-relative file name.
  *
  * @param sp
  *     The server object to operate on.
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
struct file_info_ty *server_file_info_find(server_ty *sp,
    string_ty *server_side, int auto_alloc);

/**
  * The server_mkdir function may be used to ensure that the relevant
  * client side directory exists.  The parent directories will also be
  * created, if necessary.
  *
  * \param sp
  *     The server context to work within.
  * \param client_side
  *     The directory path to be created on the client side.
  * \param server_side
  *     The corresponding directory path on the server side.
  */
void server_mkdir(server_ty *sp, string_ty *client_side,
    string_ty *server_side);

/**
  * The server_mkdir_above function may be used to ensure that the
  * <b>parent</b> directory of the given client side path exists.
  * The all necessary parent directories will also be created.
  *
  * \param sp
  *     The server context to work within.
  * \param client_side
  *     The directory path to be created on the client side.
  * \param server_side
  *     The corresponding directory path on the server side.
  */
void server_mkdir_above(server_ty *sp, string_ty *client_side,
    string_ty *server_side);

/**
  * The server_updating_verbose function is used to issue "Updating
  * <dirname>..." message to the client.
  *
  * \param sp
  *     The server context for the message.
  * \param client_side
  *     The name of the <i>file</i> being updated.  The directory is
  *     extracted, and if different to last time, it is printed.
  */
void server_updating_verbose(server_ty *sp, string_ty *client_side);

/**
  * The server_directory_find_client_side function is used to turn a
  * server-side path into a client-side path.  This is done by looking
  * for the longest matching server side path, and using its corresponding
  * client side path to build the new client side path for the given file.
  *
  * \param sp
  *     The server context for the operation.
  * \param server_side
  *     The name of the file being operated upon.
  */
string_ty *server_directory_calc_client_side(server_ty *sp,
    string_ty *server_side);

#endif // AE_CVS_SERVER_SERVER_H
