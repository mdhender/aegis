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

#ifndef AEMAKEGEN_FLAVOUR_H
#define AEMAKEGEN_FLAVOUR_H

#include <common/ac/shared_ptr.h>

#include <common/nstring.h>

#include <aemakegen/process_item.h>

class target; // forward

/**
  * The flavour class is used to represent the kind of target that that
  * a target instance is working with.  This is how difference between
  * the "generic" and "aegis" flavours of targets (automake, debian,
  * makefile, etc) is applied.
  *
  * It is important to remember that the lifetime of a flavour instance
  * is very short, do not assume that the flavour instance will be
  * present when the processing items it creates and used to process the
  * files.
  */
class flavour
{
public:
    typedef aegis_shared_ptr<flavour> pointer;

    /**
      * The destructor.
      */
    virtual ~flavour();

    /**
      * The factory class method is used to manufacture suitable
      * derived class instances, based on the flavour name.
      *
      * @param name
      *     The name of the flavour
      * @param tgt
      *     The target instance to bind to.
      * @returns
      *     pointer to new instance.  If name is unknown, will fail via
      *     fatal error and not return.
      */
    static pointer factory(const nstring &name, target &tgt);

    /**
      * The list class method is used to print a list of flavours on the
      * standard output.
      */
    static void list(void);

    /**
      * The set_process method is used to fill in tgt's processing
      * register, for handling files, no matter what the target.
      */
    virtual void set_process(void) = 0;

protected:
    /**
      * The constructor.
      * For use by derived classes only.
      *
      * @param tgt
      *     the target instance we are bound to.
      */
    flavour(target &tgt);

    /**
      * The register_process method is used to register a process to be
      * performed.  Each process is a file name pattern, and several
      * actions.  This method adds the action to the back of the queue,
      * which is the normal case.
      *
      * @param pip
      *     The process item to be added to the queue.
      */
    void register_process(const process_item::pointer &proc);

    /**
      * The register_process_front method is used to register a process
      * to be performed, but ahead of all processing to date.  Each
      * process is a file name pattern, and several actions.  This
      * method adds the action to the front of the queue, use sparingly.
      */
    void register_process_front(const process_item::pointer &proc);

private:
    /**
      * The tgt instance variable is used to remember the target
      * instance we are bound to.
      */
    target &tgt;

    /**
      * The default constructor.
      * Do not use.
      */
    flavour();

    /**
      * The copy constructor.  Do not use.
      */
    flavour(const flavour &);

    /**
      * The assignment operator.  Do not use.
      */
    flavour &operator=(const flavour &);
};

// vim: set ts=8 sw=4 et :
#endif // AEMAKEGEN_FLAVOUR_H
