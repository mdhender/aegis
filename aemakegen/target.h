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

#ifndef MAKEGEN_TARGET_H
#define MAKEGEN_TARGET_H

#include <common/ac/shared_ptr.h>
#include <list>

#include <common/nstring/list.h>
#include <libaegis/change/identifier.h>
#include <libaegis/output.h>

#include <aemakegen/process_data.h>
#include <aemakegen/process_queu.h>


/**
  * The abstract target base class is used to represent an abstract
  * output target content.  This generalises all source file waking into
  * the one place, leaving the derived classes to implement specific
  * operations.
  */
class target
{
public:
    typedef aegis_shared_ptr<target> pointer;

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
    static pointer create(const nstring &name, change_identifier &cid);

    /**
      * The list class method may be used to print a list of known
      * targets on the standard output.
      */
    static void list(void);

    /**
      * The set_flavour method is used to populate the #data instance
      * variable, as appropriate for the generation task.
      *
      * @param name
      *     The name of the flavour to set.  The empty string is
      *     interpreted to mean "generic".
      */
    void set_flavour(const nstring &name);

    /**
      * The process method is used to process the list of files.
      * It may only be called once.
      */
    void process(const nstring_list &filenames);

    /**
      * The script method is used to append another file to the list of
      * extra script files to build.  Appends to the #scripts instance
      * variable.
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
    nstring resolve(const nstring &relpath);

    // The following methods are public only so that the
    // aemakgen/proces_item/*.cc files can get at them.
    // There shall be no access by any other class.

    /**
      * The process_item_aegis_fmtgen method is used to implement
      * specific processing for "*.def" files in the source file
      * manifest.  These are taken to be "fmtgen" input files (the Aegis
      * meta-data file format).
      *
      * The default implementation resubmits the derived files,
      * but produces no output.
      *
      * @param filename
      *     The name of the file to be processed.
      */
    virtual void process_item_aegis_fmtgen(const nstring &filename);

    virtual void process_item_aegis_lib_doc(const nstring &filename);
    virtual void process_item_aegis_lib_else(const nstring &filename);
    virtual void process_item_aegis_lib_icon2(const nstring &filename);
    virtual void process_item_aegis_lib_junk(const nstring &filename);
    virtual void process_item_aegis_lib_sh(const nstring &filename);

    /**
      * The process_item_test_base64 method is used to implement specific
      * processing for "*.uue" files in the source file manifest.
      *
      * The default implementation resubmits the derived file,
      * but produces no output.
      *
      * @param filename
      *     The name of the file to be processed.
      */
    virtual void process_item_aegis_test_base64(const nstring &filename);

    /**
      * The process_item_configure_ac_in method is used to implement
      * specific processing for "configure.ac.in" files in the source
      * file manifest.  This is distinctly different processing than for
      * other "*.in" files.
      *
      * The default implementation resubmits the derived file, but
      * produces no output.
      *
      * @param filename
      *     The name of the file to be processed.
      */
    virtual void process_item_configure_ac_in(const nstring &filename);

    /**
      * The process_item_autoconf method is used to implement specific
      * processing for "*.in" files in the source file manifest.  These
      * are taken to be "status.config" input files.
      *
      * see Gnu Autoconf, the ./configure script creates config.stats,
      * and config .status proccesses *.in files to remove the suffix.
      *
      * The default implementation resubmits the derived file,
      * but produces no output.
      *
      * @param filename
      *     The name of the file to be processed.
      */
    virtual void process_item_autoconf(const nstring &filename);

    virtual void process_item_c(const nstring &filename);
    virtual void process_item_cxx(const nstring &filename);
    virtual void process_item_datadir(const nstring &filename);
    virtual void process_item_datarootdir(const nstring &filename);
    virtual void process_item_etc_man(const nstring &filename);
    virtual void process_item_etc_msgfmt_sh(const nstring &filename);
    virtual void process_item_etc_profile(const nstring &filename);
    virtual void process_item_extra_dist(const nstring &filename);
    virtual void process_item_flexible(const nstring &filename);
    virtual void process_item_i18n(const nstring &filename);
    virtual void process_item_include(const nstring &filename);

    /**
      * The process_item_lex method is used to implement specific
      * processing for "*.l" files in the source file manifest.  These
      * are taken to be Lex (Flex) input files.
      *
      * The default implementation resubmits the derived file,
      * but produces no output.
      *
      * @param filename
      *     The name of the file to be processed.
      */
    virtual void process_item_lex(const nstring &filename);

    virtual void process_item_libdir(const nstring &filename);
    virtual void process_item_library(const nstring &filename);
    virtual void process_item_man_page(const nstring &filename);
    virtual void process_item_pkgconfig(const nstring &filename);
    virtual void process_item_scripts(const nstring &filename);
    virtual void process_item_test_sh(const nstring &filename);

    /**
      * The process_item_uudecode method is used to implement specific
      * processing for "*.uue" files in the source file manifest.
      *
      * The default implementation resubmits the derived file,
      * but produces no output.
      *
      * @param filename
      *     The name of the file to be processed.
      */
    virtual void process_item_uudecode(const nstring &filename);

    /**
      * The process_item_yacc method is used to implement specific
      * processing for "*.y" files in the source file manifest.  These
      * are taken to be Yacc (Bison) input grammar files.
      *
      * The default implementation resubmits the derived files,
      * but produces no output.
      *
      * @param filename
      *     The name of the file to be processed.
      */
    virtual void process_item_yacc(const nstring &filename);

protected:
    friend class process_item;
    friend class flavour;

    /**
      * The constructor.
      * For use by derived classes only.
      *
      * @param cid
      *     The location of the change identification
      */
    target(change_identifier &cid);

    /**
      * The preprocess_configure_dot_ac method is used to examine the
      * contents of a "configure.ac" file, to see if any information can
      * be gleaned from it.  It calls #preprocess_configure_dot_ac to do
      * all the heavy lifting.
      *
      * @param filename
      *     the name of the file of interest
      */
    void preprocess_configure_dot_ac(const nstring &filename);

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
    bool trim_script_suffix(void) const;

    /**
      * The get_project_name method is used to obtain the name of the
      * project.
      */
    nstring get_project_name(void) const;

    /**
      * The get_project_version method is used to obtain the version of
      * the project (strictly speaking, of the change set).
      */
    nstring get_project_version(void) const;

    /**
      * The data instance variable is used to remember everything about
      * file processing to be performed.  This is becaise it is common
      * to target derived classes, and also each flavour.
      *
      * Derived classes may access this instance variable directly.
      */
    process_data data;

    /**
      * The get_library_libname method may be used to obtain the name
      * of the library containing common files to be linked with each
      * of the executables (basename, not including extension).  It
      * is guaranteed to start with "lib", adding it in front of the
      * library's name, if necessary.
      */
    nstring get_library_libname(void) const;

    change::pointer get_cp(void) { return cid.get_cp(); }
    project *get_pp(void) { return cid.get_pp(); }
    user_ty::pointer get_up(void) { return cid.get_up(); }

    /**
      * The register_process method is used to register a process to be
      * performed.  Each process is a file name pattern, and several
      * actions.  This method adds the action to the back of the queue,
      * which is the normal case.
      *
      * @param pip
      *     The process item to be added to the queue.
      */
    void register_process(const process_item::pointer &pip);

    /**
      * The register_process_front method is used to register a process
      * to be performed.  This method adds the action to the front of
      * the queue, which is quite slow, but occasionally necessary when
      * a derived class needs to override a base class's pattern(s).
      *
      * @param pip
      *     The process item to be added to the queue.
      */
    void register_process_front(const process_item::pointer &pip);

    /**
      * The is_installable method may be used to determine whether or
      * not the named command is installable.
      *
      * @param command_name
      *     The name of the executable.
      * @returns
      *     true if command is installable, false if command should not
      *     be installed.
      */
    bool is_installable(const nstring &command_name);

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

    /**
      * The contains_dot_xx_directive method is used to determine
      * whether or not a groff inpout file contains index directives
      * that need to be removed.
      *
      * @param filename
      *     The name of the file to be tested.
      */
    bool contains_dot_xx_directive(const nstring &filename);

    /**
      * The file_attr_noinst method may be used to determine whether or not
      * a source file has the aemakgen:debian:noinst=true attribute.
      *
      * @param filename
      *     The name of the source file of interest.
      * @returns
      *     true if attribute exists and is set to true, false
      *     if the attribute does not exist or exists and is set to false.
      */
    bool file_attr_noinst(const nstring &filename);

    void warning_this_file_is_generated(void);

    void warning_this_file_is_generated(const output::pointer &op) const;

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

    /**
      * The op instance variable is used to remember where to send the
      * output, rather than using stdio.  This means we can interpose
      * filters, and other cool tricks.
      */
    output::pointer op;

    /**
      * The location_comment method may be used to insert debugging
      * comments into the output at strategic locations.  Nothing is
      * printed unless --debug is in effect.
      */
    void location_comment(const char *file_name, int line_number);

    /**
      * The print_blank_line method may be used to force a blabk line in
      * the output.
      */
    void print_blank_line(void);

    /**
      * The print_comment method is used to print a wrapped comment to
      * the makefile being output.
      *
      * @param text
      *     The text to be wrapped and printed.
      */
    void print_comment(const nstring &text);

    /**
      * The print_assignment method is used to print a macro
      * assignment to the makefile being output.
      * If the right hand side is empty, no assignment is printed.
      *
      * @param name
      *     The name of the macro.
      * @param value
      *     The value of the macro.
      */
    void print_assignment(const nstring &name, const nstring &value);

    /**
      * The print_empty_assignment method is used to print a macro
      * assignment to the makefile being output.  The right hand side
      * will be empty.
      *
      * @param name
      *     The name of the macro.
      */
    void print_empty_assignment(const nstring &name);

    /**
      * The print_assignment method is used to print a macro
      * assignment to the makefile being output.
      *
      * @param name
      *     The name of the macro.
      * @param value
      *     The (possibly empty) value of the macro.
      */
    void print_assignment(const nstring &name, const nstring_list &value);

    /**
      * The print_assignment-sorted method is used to print a macro
      * assignment to the makefile being output.
      *
      * @param name
      *     The name of the macro.
      * @param value
      *     The (possibly empty) value of the macro.  These values will
      *     be sorted before output (original input unchanged).
      */
    void print_assignment_sorted(const nstring &name,
        const nstring_list &value);

    /**
      * The print_rule method is used to print a make rule to the
      * makefile being output.
      *
      * @param lhs
      *     The left hand side (targets, outputs) of the rule.
      * @param rhs
      *     The right hand side (ingredients, inputs) of the rule.
      * @param body
      *     The (possibly empty) body of the rule.
      */
    void print_rule(const nstring_list &lhs, const nstring_list &rhs,
        const nstring_list &body);

    /**
      * The print_rule method is used to print a make rule to the
      * makefile being output.  This is a greatly simplifiled form, for
      * expressing dependencies.
      *
      * @param lhs
      *     The left hand side (target, output) of the rule.
      * @param rhs
      *     The right hand side (ingredients, inputs) of the rule.
      */
    void print_rule(const nstring &lhs, const nstring &rhs);

    void print_clean_file_rule(const nstring &lhs, const nstring_list &rhs,
        const nstring_list &files);

    void print_clean_dirs_rule(const nstring &lhs, const nstring_list &rhs,
        const nstring_list &files);

    /**
      * The c_include_dependencies method is used to scan C and C++ source
      * files for include dependencies.
      *
      * @param filename
      *     The name of the file to be scanned.
      * @returns
      *     a space separated list of file names.
      */
    nstring_list c_include_dependencies(const nstring &filename);

    /**
      * The roff_include_dependencies method is used to scan *roff source
      * files for include dependencies.
      *
      * @param filename
      *     The name of the file to be scanned.
      * @returns
      *     a space separated list of file names.
      */
    nstring_list roff_include_dependencies(const nstring &filename);

    void emit_static_library_rules(const nstring &name);

    void emit_library_rules(const nstring &library_dirname,
        const nstring &library_libname, bool shared);

protected:
    /**
      * The processing processing instance variable is used to
      * remember all of the registered processing items to be
      * done.  It is populated by using the #register_process and
      * #register_process_front methods.
      */
    process_queue processing;

    /**
      * The file_is_in_manifest method may be used by derived classes
      * to dtermine whether or not the fiven file name is in the file
      * manifest.  Of most use to item processing methods.
      */
    bool file_is_in_manifest(const nstring &filename) const;

private:
    /**
      * The process1 method is called by the #process method to perform
      * processing to be done *before* the registered processes get to
      * preprocess the files.
      */
    void process1(const nstring_list &filenames);

    /**
      * The process2 method is called by the #process method to
      * perform the stuff around calling the registered process'
      * #process_item::preprocess method.
      */
    void process2(const nstring_list &filenames);

    /**
      * The process2_begin method is called by the #process2 method
      * before any of the preprocess items are done.  The default
      * implementation does nothing.
      */
    virtual void process2_begin(void);

    /**
      * The process2_end method is called by the #process2 method
      * after all of the preprocess items are done.
      */
    virtual void process2_end(void);

    /**
      * The process3 method is called by the #process method to perform
      * the target-specific stuff around calling the pregistered
      * process' process_item::process_automake (etc) methods.
      */
    void process3(const nstring_list &filenames);

    /**
      * The process3_begin method is called by the #process3 method
      * before any of the process items are done.
      * The default implementation does nothing.
      */
    virtual void process3_begin(void);

    /**
      * The process3_end method is called by the #process3 method
      * after all of the process items are done.
      * The default implementation does nothing.
      */
    virtual void process3_end(void);

    /**
      * The cid instance variable is used to remember the location of
      * the change identification information.
      */
    change_identifier &cid;

    /**
      * The view_path instance variable is used to remember the list of
      * directories to search for source files.
      */
    nstring_list view_path;

    /**
      * The manifest instance variable is used to remember all of the
      * file names passed to the #process method.  It is assigned before
      * any processing is done.
      */
    nstring_list manifest;

    /**
      * The scripts instance variable is used to remember this set of
      * script sources that are to be built.
      */
    static nstring_list scripts;

    /**
      * The preprocess_include_headers method is used to calculate the
      * closure of the master include file, and stash the results in the
      * include_headers instance variable.
      *
      * @param filename
      *     The file to scane for more includes.
      */
    void preprocess_include_headers(const nstring &filename);

    enum ostate_t
    {
        ostate_reset,
        ostate_blank_line,
        ostate_comment,
        ostate_assignment,
        ostate_rule
    };
    ostate_t ostate;

    void ostate_become(ostate_t new_state);

    void c_include_dependencies(nstring_list &result, const nstring &filename);

    void c_directive(const char *s, nstring_list &results);

    void roff_include_dependencies(nstring_list &result,
        const nstring &filename);

    bool is_an_include_candidate(const nstring &fn) const;

    /**
      * The default constructor.  Do not use.
      */
    target();

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
// vim: set ts=8 sw=4 et :
