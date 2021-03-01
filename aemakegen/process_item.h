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

#ifndef AEMAKEGEN_PROCESS_ITEM_H
#define AEMAKEGEN_PROCESS_ITEM_H

#include <common/ac/shared_ptr.h>

#include <common/nstring.h>

class target; // forward
class process_data; // forward

/**
  * The process_item class is used to represent one item of processing
  * by the target_makefile::process method.
  *
  * The reason process_item has methods for each of the possible output
  * types, is because we need all those output to mesh perfectly, and
  * that means using identical code to generate the various lists of
  * files.
  *
  * Do not assume the file names will be presented for processing in any
  * particular order.  The fact that the code may or may not sort the
  * file names <b>shall not</b> berelied upon.
  */
class process_item
{
public:
    typedef aegis_shared_ptr<process_item> pointer;

    typedef void (target::*target_method_ptr)(const nstring &filename);

    /**
      * The destructor.
      */
    virtual ~process_item();

    /**
      * The run_preprocess method is used to determine whether or not
      * the given filename matches the #condition.
      * If it matches, run the #preprocess method and return true.
      * If it doesn't match, simply return false.
      *
      * @param filename
      *     The name of the file of interest.
      */
    bool run_preprocess(const nstring &filename);

    /**
      * Some process items want to have a final word, once all the
      * preprocessing is completed.  Use sparingly.
      */
    virtual void preprocess_end(void);

    /**
      * The run_process method is used to determine whether or not the
      * given filename matches the #condition.  If it matches, run the
      * target::*method and return true.  If it doesn't match, simply
      * return false.
      *
      * @param filename
      *     The name of the file of interest.
      */
    bool run_process(const nstring &filename);

protected:
    /**
      * The constructor.
      * For use by derived classes only.
      *
      * @param tgt
      *     The target instance to bind to.
      * @param method
      *     The method if the target to be called.
      */
    process_item(target &tgt, target_method_ptr method);

    /**
      * The data instance variable is used to hold the data common to the
      * process_item class tree and the flavour class tree.
      */
    process_data &data;

    /**
      * The condition method is used to determine whether or not this
      * process_item applies to the given file.
      *
      * @param filename
      *     The name of the file of interest.
      * @returns
      *     true if the file is a match, false if not.
      */
    virtual bool condition(const nstring &filename) = 0;

    /**
      * The preprocess method is used to update the common data store
      * (see the #process_data class).  It shall never emit anything to
      * stdout.
      *
      * @param filename
      *     The name of the file of interest.
      */
    virtual void preprocess(const nstring &filename) = 0;

    /**
      * The process method is used to invoke the target method, which
      * will take care of printing per-file stuff out.
      *
      * @param filename
      *     The name of the file of interest.
      */
    void process(const nstring &filename);

    /**
      * The filename_implies_is_a_script method is used to determine
      * whether or not the given filename appears to be a script, by
      * examining the file extension, and also the first line for a
      * script "#!" signature.
      *
      * @param filename
      *     The name of the file of interest.
      * @returns
      *     true if it looks like a script,
      *     false if it does not look like a script.
      */
    bool filename_implies_is_a_script(const nstring &filename);

protected:
    /**
      * The tgt_preprocess method is called by derived classes
      * to submit files "upstream" for processing.
      *
      * @param filename
      *     The name of the file to be processed.
      */
    void tgt_preprocess(const nstring &filename);

    /**
      * The is_installable method may be used to determine whether or
      * not the named command is installable.
      *
      * @param program_name
      *     The name of the executable.
      * @returns
      *     true if command is installable, false if command should not
      *     be installed.
      */
    bool is_installable(const nstring &program_name);

    /**
      * The get_project_name method is used to obtain the name of the
      * project.
      */
    nstring get_project_name(void);

    /**
      * The is_hash_bang method is used to determine wether or not a
      * file starts with "#!" indicating that it is a script fiole of
      * some sort.  Uses #resolve method to ocate the actual file.
      *
      * @param filename
      *     The name of the file to be tested.
      * @returns
      *     true of the file starts with "#!", false if it does not
      */
    bool is_hash_bang(const nstring &filename);

    /**
      * The contains_dot_so_directive method is used to determine
      * whether or not a groff inpout file contains include directives
      * that need to be resolved.
      *
      * @param filename
      *     The name of the file to be tested.
      */
    bool contains_dot_so_directive(const nstring &filename);

private:
    /**
      * The tgt instance variable is used to rememner the target
      * instance we are bound to.
      */
    target &tgt;

    /**
      * The method instance variable i sused to remember the method to
      * be called onthe target, by the #process method.
      */
    target_method_ptr method;

    /**
      * The default constructor.
      * For use by derived classes only.
      */
    process_item();

    /**
      * The copy constructor.
      * Do not use.
      */
    process_item(const process_item &);

    /**
      * The assignment operator.
      * Do not use.
      */
    process_item &operator=(const process_item &);
};

// vim: set ts=8 sw=4 et :
#endif // AEMAKEGEN_PROCESS_ITEM_H
