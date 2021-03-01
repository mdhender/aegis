//
//	aegis - project change supervisor
//	Copyright (C) 2001-2008 Peter Miller
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

#ifndef LIBAEGIS_PROJECT_FILE_ROLL_FORWARD_H
#define LIBAEGIS_PROJECT_FILE_ROLL_FORWARD_H

#include <common/ac/time.h>

#include <common/symtab/template.h>
#include <libaegis/change/list.h>
#include <libaegis/fstate.h>
#include <libaegis/project.h>
#include <libaegis/file/event/list.h>

class nstring_list; // forward
struct cstate_src_ty; // forward
class file_event;


class project_file_roll_forward
{
public:
    /**
      * The destructor.
      */
    virtual ~project_file_roll_forward();

    /**
      * The default constructor.
      */
    project_file_roll_forward();

    /**
      * See the set() method for documentation.
      */
    project_file_roll_forward(project_ty *pp, time_t limit, int detailed);

    /**
      * The set method is used to recapitilate
      * the project's history, constructing information about the state
      * of all files as it goes.  The project_file_roll_forward_get
      * function is used to extract the results.
      *
      * \param pp
      *     The project to apply the listing to.
      *     All parent branches will be visited, too.
      * \param limit
      *     The time limit for changes.  Changes on or before this
      *     time will be included.
      * \param detailed
      *     If this is false, only the parent branches and the
      *     project itself are visited.  If this is true, all branches
      *     completed within the limt will be visited.
      *
      * \note
      *    This function is one really big memory leak.
      */
    void set(project_ty *pp, time_t limit, int detailed);

    /**
      * The project_file_roll_forward_get function is used to obtain the
      * events for a given file, once project_file_roll_forward has been
      * called to construct the information.
      *
      * \param src
      *    The file description of the file to fetch the event
      *    list.  Will use the uuid if available (or, for backwards
      *    compatibility) the file name.
      * \returns
      *    Pointer to the event list for the named file, or NULL if the
      *    file has never existed at the time (delta) specified.
      * \note
      *    Do not free the change pointed to, as it may be referenced by
      *    other files' histories.
      */
    file_event_list::pointer get(fstate_src_ty *src);

    /**
      * The get method is used to obtain the events for a given file,
      * once project_file_roll_forward has been called to construct the
      * information.
      *
      * \param src
      *    The file description of the file to fetch the event
      *    list.  Will use the uuid if available (or, for backwards
      *    compatibility) the file name.
      * \returns
      *    Pointer to the event list for the named file, or NULL if the
      *    file has never existed at the time (delta) specified.
      * \note
      *    Do not free the change pointed to, as it may be referenced by
      *    other files' histories.
      */
    file_event_list::pointer get(cstate_src_ty *src);

    /**
      * The project_file_roll_forward_get function is used to obtain the
      * events for a given file, once project_file_roll_forward has been
      * called to construct the information.
      *
      * \param filename
      *    The name of the file to fetch the event list
      * \returns
      *    Pointer to the event list for the named file, or NULL if the
      *    file has never existed at the time (delta) specified.
      * \note
      *    Do not free the change pointed to, as it may be referenced by
      *    other files' histories.
      */
    file_event_list::pointer get(const nstring &filename);

    /**
      * The project_file_roll_forward_get function is used to obtain the
      * events for a given file, once project_file_roll_forward has been
      * called to construct the information.
      *
      * \param filename
      *    The name of the file to fetch the event list
      * \returns
      *    Pointer to the event list for the named file, or NULL if the
      *    file has never existed at the time (delta) specified.
      * \note
      *    Do not free the change pointed to, as it may be referenced by
      *    other files' histories.
      * \note
      *    This method will be DEPRECATED one day.
      */
    file_event_list::pointer get(string_ty *filename);

    /**
      * The get_last method is used to get the last file event, used by
      * most functions which deal with deltas.
      *
      * \param filename
      *    The name of the file to fetch the last event
      * \returns
      *    Pointer to the last event for the named file, or NULL if the
      *    file has never existed at the time (delta) specified.
      */
    file_event *get_last(const nstring &filename);

    /**
      * The get_last method is used to get the last file event, used by
      * most functions which deal with deltas.
      *
      * \param filename
      *    The name of the file to fetch the last event
      * \returns
      *    Pointer to the last event for the named file, or NULL if the
      *    file has never existed at the time (delta) specified.
      * \note
      *    This method will be DEPRECATED one day.
      */
    file_event *get_last(string_ty *filename);

    /**
      * The get_last method is used to get the last file event,
      * using the file's meta data to identify it.
      *
      * \param src
      *    The meta-data describing the file (any revision data, if
      *    present, will be ignored)
      * \returns
      *    Pointer to the last event for the named file, or NULL if the
      *    file has never existed at the time (delta) specified.
      */
    file_event *get_last(fstate_src_ty *src);

    /**
      * The project_file_roll_forward_get_older function is used to get the
      * last-but-one file event, used by aecp -rescind to roll back a change.
      *
      * \param filename
      *    The name of the file to fetch the last event
      * \returns
      *    Pointer to the last event for the named file, or NULL if the
      *    file has never existed at the time (delta) specified.
      */
    file_event *get_older(const nstring &filename);

    /**
      * The project_file_roll_forward_get_older function is used to get the
      * last-but-one file event, used by aecp -rescind to roll back a change.
      *
      * \param filename
      *    The name of the file to fetch the last event
      * \returns
      *    Pointer to the last event for the named file, or NULL if the
      *    file has never existed at the time (delta) specified.
      * \note
      *    This method will be DEPRECATED one day.
      */
    file_event *get_older(string_ty *filename);

    /**
      * The keys method is used to get a list of filenames for which
      * file event lists are available.
      *
      * \param file_name_list
      *     Where to put the list of file names.
      */
    void keys(nstring_list &file_name_list);

    /**
      * The is_set method is used to determine if the set() method has
      * been invoked, directly or indirectly.
      *
      * \returns
      *      bool: true if it has been set, false if not.
      */
    bool is_set() const { return !uuid_to_felp.empty(); }

    /**
      * The get_last_change method is used to get the pointer to the
      * last change set in the reconstructed history.
      */
    change::pointer get_last_change() const;

private:
    /**
      * The get_by_uuid method is used by several of the simpler get
      * methods, once they have performed the uuid-or-filename transform.
      *
      * @param uuid
      *     The UUID of the file, if it has one, otherwise the name of
      *     the file.
      */
    file_event_list::pointer get_by_uuid(string_ty *uuid);

    /**
      * The uuid_to_felp instance variable is used to remember the
      * mapping from UUIS to file history (for backwards compatibility,
      * index by filename if no UUID is available).
      */
    symtab<file_event_list::pointer> uuid_to_felp;

    /**
      * The filename_to_uuid method is used to map user perception of
      * filename (which can vary over time) to the file's UUID (which is
      * invariant).
      */
    symtab<string_ty> filename_to_uuid;

    /**
      * The stp_time instance variable is used to remember the most
      * recent event for any file for the whole reconstruction.
      */
    time_t stp_time;

    /**
      * The last_change instance variable is used to remember the last
      * change in the historical reconstruction.
      */
    change::pointer last_change;

    /**
      * The recapitulate method is used to replay the changes of a
      * branch, indexing each file as it goes.  It recurses into parent
      * branches.
      *
      * \param pp
      *     The project to recapitulate.
      * \param limit
      *     The lime linit; any events after this will be ignored.
      * \param detailed
      *     If true, also recurse into child branches, this gives the
      *     maximum amount of detail available, but usually of interests
      *     to humans (listings) rather than for file content (aecp & co).
      * \returns
      *     The latest time found in any event (<= limit).
      */
    time_t recapitulate(project_ty *pp, time_t limit, int detailed);

    /**
      * The copy constructor.  Do not use.
      */
    project_file_roll_forward(const project_file_roll_forward &);

    /**
      * The assignment operator.  Do not use.
      */
    project_file_roll_forward &operator=(const project_file_roll_forward &);
};

#endif // LIBAEGIS_PROJECT_FILE_ROLL_FORWARD_H
