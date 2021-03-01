//
// aegis - project change supervisor
// Copyright (C) 2008-2012 Peter Miller
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

#ifndef AEMAKEGEN_TARGET_DEBIAN_H
#define AEMAKEGEN_TARGET_DEBIAN_H

#include <aemakegen/target.h>

/**
  * The target_debian class is used to represent the processing needed
  * to create the contents of the debian/ directory for a Debian
  * package, using Aegis' file manifest.
  */
class target_debian:
    public target
{
public:
    /**
      * The destructor.
      */
    virtual ~target_debian();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param cid
      *     The location of the change identification
      */
    static pointer create(change_identifier &cid);

protected:
    // See base class for documentation.
    void process3_end(void);

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param cid
      *     The location of the change identification
      */
    target_debian(change_identifier &cid);

    void gen_control(void);
    void gen_rules(void);
    void gen_compat(void);
    void gen_copyright(void);
    void gen_changelog(void);
    void gen_source_format(void);

    /**
      * The redundant_license_file class method is used to determine
      * whether or not a given file is a redundant license file.  Debian
      * policy frowns on installing such files.
      *
      * The file name pattern is .../man[1-8]/...[-_]license.[1-8]
      * and similar, including "copyright" instead of "license".
      */
    static bool redundant_license_file(const nstring &filename);

    /**
      * The maybe_field method is used to emit a value for a field, if
      * that field is defined.  Nothing is printed of the field is not
      * set in the project specific attributes.
      *
      * @param op
      *     where to print the field
      * @param field_name
      *     The name of the field
      * @param package_name
      *     The name of the project (possibly massaged).
      */
    void maybe_field(const output::pointer &op, const nstring &field_name,
        const nstring &package_name);

    nstring_list filter_dev(const nstring_list &src) const;

    void fill_instance_variables(void);

    static nstring expand_make_macro(const nstring &text);

    nstring source_package_name;

    nstring binary_package_name;
    nstring_list binary_package_files;

    nstring documentation_package_name;
    nstring_list documentation_package_files;

    nstring runtime_package_name;
    nstring_list runtime_package_files;

    nstring debug_package_name;

    nstring developer_package_name;
    nstring_list developer_package_files;

    /**
      * The default constructor.  Do not use.
      */
    target_debian();

    /**
      * The copy constructor.  Do not use.
      */
    target_debian(const target_debian &);

    /**
      * The assignment operator.  Do not use.
      */
    target_debian &operator=(const target_debian &);
};

#endif // AEMAKEGEN_TARGET_DEBIAN_H
// vim: set ts=8 sw=4 et :
