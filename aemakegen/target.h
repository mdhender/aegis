//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller;
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License,
//      version 3, as published by the Free Software Foundation.
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

#ifndef MAKEGEN_TARGET_H
#define MAKEGEN_TARGET_H

#include <common/nstring/list.h>

/**
  * The abstract target base class is used to represent an abstract
  * output target content.  This generalises all source file waking into
  * the one place, leaving the derived classes to implement specific
  * operations.
  */
class target
{
public:
    /**
      * The destructor.
      */
    virtual ~target();

    /**
      * The create class method is used to manufacture a new instance of
      * a target processor by name.
      *
      * @param name
      *     The name of the target class.
      */
    static target *create(const nstring &name);

    /**
      * The process method is used to process the list of files.
      * It may only be called once.
      */
    void process(const nstring_list &filenames);

    /**
      * The vpath method is used to append another directory to the
      * viewpath used to search for files.
      *
      * @param dirnam
      *     The path of a directory to add to the view path.
      */
    static void vpath(const nstring &dirnam);

    /**
      * The script method is used to append another file to the list of
      * extra script files to build.
      *
      * @param file_name
      *     The path of a script source file.
      */
    static void script(const nstring &file_name);

    /**
      * The resolve method is used to take a relative path and locate
      * the actual path of the file by looking down the view path.  The
      * first hit is returned.
      *
      * If the file is not found, the relpath is returned.
      *
      * @param relpath
      *     file name to search for
      * @returns
      *     path to the file
      */
    static nstring resolve(const nstring &relpath);

protected:
    /**
      * The default constructor.
      * Thou shalt derive from this class.
      */
    target();

    /**
      * The preprocess method is called by the process(nstring_list) for
      * each file name in the list <i>before</i> the begin() method is
      * called.  <b>Do not</b> generate any output from this method.
      *
      * @param filename
      *     The name of the file to be processed.
      * @note
      *     The file name has not been resolved to an absolute path via
      *     the vpath.  Use the resolve method for that.
      */
    virtual void preprocess(const nstring &filename);

    /**
      * The begin method is called by the process(nstring_list) method
      * immediately before it starts calling process(nstring) for each
      * file name.
      */
    virtual void begin() = 0;

    /**
      * The process method is called by the process(nstring_list) for
      * each file name in the list.
      *
      * @param filename
      *     The name of the file to be processed.
      * @param is_a_script
      *     true if file is a script to be built
      * @note
      *     The file name has not been resolved to an absolute path via
      *     the vpath.  Use the resolve method for that.
      */
    virtual void process(const nstring &filename, bool is_a_script) = 0;

    /**
      * The end method is called by the process(nstring_list) method
      * immediately after it finishes calling process(nstring) for each
      * file name.
      */
    virtual void end() = 0;

    /**
      * The exists method is used to determine if the given path exists
      * in the filer systems.  No particular type (file, drectory, etc)
      * is implied.
      *
      * @param path
      *     The path to test.
      * @returns
      *     bool; true if exists, false if not
      */
    static bool exists(const nstring &path);

    /**
      * The trim_script_suffix method is used to determine whether or
      * not script file need to have their file extension removed when
      * they are "built".
      */
    bool trim_script_suffix() const;

private:
    /**
      * The view_path instance variable is used to remember the list of
      * directories to search for source files.
      */
    static nstring_list view_path;

    /**
      * The scripts instance variable is used to remember this set of
      * script sources that are to be built.
      */
    static nstring_list scripts;

    /**
      * The copy constructor.  Do not use.
      */
    target(const target &);

    /**
      * The assignment operator.  Do not use.
      */
    target &operator=(const target &);
};

#endif // MAKEGEN_TARGET_H
