//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AEMAKEGEN_PROCESS_QUEU_H
#define AEMAKEGEN_PROCESS_QUEU_H

#include <list>

#include <common/nstring/list.h>

#include <aemakegen/process_item.h>

/**
  * The process_queue class is used to represent an ordered list of
  * processing to be performed.
  */
class process_queue
{
public:
    /**
      * The destructor.
      */
    virtual ~process_queue();

    /**
      * The default constructor.
      */
    process_queue();

    /**
      * The register_process_item method is used to register a process
      * to be performed.  Each process is a file name pattern, and
      * several actions.  This method adds the action to the back of the
      * queue, which is the normal case.
      *
      * @param pip
      *     The process item to be added to the queue.
      */
    void register_process_item(const process_item::pointer &pip);

    /**
      * The register_process_item_front method is used to register a
      * process to be performed.  Each process is a file name pattern,
      * and several actions.  This method adds the action to the front
      * of the queue, which is slow.  This can be necessary when you
      * need to override an item in the queue.
      *
      * @param pip
      *     The process item to be added to the queue.
      */
    void register_process_item_front(const process_item::pointer &pip);

    /**
      * The register-default method may be used to supply a process item
      * to be used in the case where no other process item matches a
      * file.  (The condition method will be ignored.)
      *
      * The NULL pointer is acceptable, it means "do nothing", which is
      * also the default default.
      */
    void register_default(const process_item::pointer &pip);

    /**
      * The run_preprocess method is used to run each of the file names
      * through the registered processes, stopping each time after the
      * process that first matches the file.
      *
      * @param filenames
      *     The list of files to be processed.
      */
    void run_preprocess(const nstring_list &filenames);

    // only public so that process_item classes can have a second bite
    void run_preprocess(const nstring &filename);

    /**
      * The run_process method is used to run each of the file names
      * through the registered processes, stopping each time at the
      * process that first matches the file.
      *
      * @param filenames
      *     The list of files to be processed.
      */
    void run_process(const nstring_list &filenames);

    // only public so that process_item classes can have a second bite
    void run_process(const nstring &filename);

private:
    typedef std::list<process_item::pointer> queue_t;

    /**
      * The queue instance variable is used to remember the ordered list
      * of #process_item instances to work with.
      */
    queue_t queue;

    /**
      * The dflt instance variable is used to remember the process item
      * to be used in the case where no other process item matches a
      * file.  (The condition method will be ignored.)  The NULL pointer
      * is acceptable, it means "do nothing", which is also the default
      * default.
      */
    process_item::pointer dflt;

    /**
      * The copy constructor.  Do not use.
      */
    process_queue(const process_queue &);

    /**
      * The assignment operator.  Do not use.
      */
    process_queue &operator=(const process_queue &);
};

// vim: set ts=8 sw=4 et :
#endif // AEMAKEGEN_PROCESS_QUEU_H
