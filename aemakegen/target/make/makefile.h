//
// aegis - project change supervisor
// Copyright (C) 2006-2012 Peter Miller;
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 3, as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef MAKEGEN_TARGET_MAKE_MAKEFILE_H
#define MAKEGEN_TARGET_MAKE_MAKEFILE_H

#include <aemakegen/target/make.h>

/**
  * The target_make_makefile class is used to represent the processing
  * necessary to produce a Makefile.in file for this project.
  */
class target_make_makefile:
    public target_make
{
public:
    /**
      * The destructor.
      */
    virtual ~target_make_makefile();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param cid
      *     The location of the change identifier
      */
    static pointer create(change_identifier &cid);

protected:
    // See base class for documentation.
    void process3_begin(void);

    // See base class for documentation.
    void process_item_aegis_lib_doc(const nstring &filename);

    // See base class for documentation.
    void process_item_aegis_lib_else(const nstring &filename);

    // See base class for documentation.
    void process_item_aegis_lib_icon2(const nstring &filename);

    // See base class for documentation.
    void process_item_aegis_lib_sh(const nstring &filename);

    // See base class for documentation.
    void process_item_aegis_test_base64(const nstring &filename);

    // See base class for documentation.
    void process_item_c(const nstring &filename);

    // See base class for documentation.
    void process_item_cxx(const nstring &filename);

    // See base class for documentation.
    void process_item_datadir(const nstring &filename);

    // See base class for documentation.
    void process_item_datarootdir(const nstring &filename);

    // See base class for documentation.
    void process_item_etc_man(const nstring &filename);

    // See base class for documentation.
    void process_item_etc_profile(const nstring &filename);

    // See base class for documentation.
    void process_item_i18n(const nstring &filename);

    // See base class for documentation.
    void process_item_include(const nstring &filename);

    // See base class for documentation.
    void process_item_lex(const nstring &filename);

    // See base class for documentation.
    void process_item_libdir(const nstring &filename);

    // See base class for documentation.
    void process_item_man_page(const nstring &filename);

    // See base class for documentation.
    void process_item_pkgconfig(const nstring &filename);

    // See base class for documentation.
    void process_item_scripts(const nstring &filename);

    // See base class for documentation.
    void process_item_test_sh(const nstring &filename);

    // See base class for documentation.
    void process_item_uudecode(const nstring &filename);

    // See base class for documentation.
    void process_item_yacc(const nstring &filename);

    // See base class for documentation.
    void process3_end(void);

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead/
      *
      * @param cid
      *     The location of the change identifier
      */
    target_make_makefile(change_identifier &cid);

    /**
      * The need_ar method is used to determine whether or not the AR=ar
      * set-up is needed in the generated Makefile.in file.
      */
    bool need_ar(void) const;

    /**
      * The default constructor.  Do not use.
      */
    target_make_makefile();

    /**
      * The copy constructor.  Do not use.
      */
    target_make_makefile(const target_make_makefile &);

    /**
      * The assignment operator.  Do not use.
      */
    target_make_makefile &operator=(const target_make_makefile &);
};

// vim: set ts=8 sw=4 et :
#endif // MAKEGEN_TARGET_MAKE_MAKEFILE_H
