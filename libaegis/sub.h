//
// aegis - project change supervisor
// Copyright (C) 1991-1995, 1997, 1999, 2002-2008, 2011, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef SUB_H
#define SUB_H

#include <common/ac/stdarg.h>
#include <common/ac/time.h>
#include <common/str.h>
#include <libaegis/change.h>
#include <libaegis/sub/diversion/stack.h>
#include <libaegis/sub/functor/list.h>

struct project; // forward
struct sub_context_ty; // forward
class nstring; // forward
class wstring; // forward
class wstring_list; // forward

/**
  * The sub_context_ty class represents the context (variables &co) for
  * a substitution.
  */
class sub_context_ty
{
public:
    /**
      * The destructor.
      */
    ~sub_context_ty();

    /**
      * The default constructor.
      */
    sub_context_ty(const char *file = 0, int line = 0);

    /**
      * The clear method is used to clear all of the substitution
      * variables.  Not usually needed manually, as this is done
      * automatically at the end of every substitute().
      */
    void clear();

    /**
      * The var_set_format method is used to set the value of a
      * substitution variable.  These variables are command specific,
      * as opposed to the functions which are always present.  The
      * user documentation does NOT make this distinction by using the
      * names "variable" and "function", they are always referred to as
      * "substitutions".
      *
      * @param name
      *     The name of the variable to be set.
      * @param fmt
      *     The format string for the value (see printf(3) fdor more
      *     information) and the format parameters follow in the succeeding
      arguments.
      */
    void var_set_format(const char *name, const char *fmt, ...)
                                                              ATTR_PRINTF(3, 4);
    /**
      * The var_set_vformat method is used to set a substitution variable
      * with a formatted value.
      *
      * @param name
      *     The name of the variable to be set.
      * @param fmt
      *     The format string for the value (see vprintf(3) for more
      *     information).
      * @param args
      *     The format parameters.
      */
    void var_set_vformat(const char *name, const char *fmt, va_list args)
                                                                ATTR_VPRINTF(3);
    /**
      * The var_set_string method is used to set a substitution variable.
      *
      * @param name
      *     The name of the variable to be set.
      * @param value
      *     The value of the variable to be set.
      */
    void var_set_string(const char *name, string_ty *value);

    /**
      * The var_set_string method is used to set a substitution variable.
      *
      * @param name
      *     The name of the variable to be set.
      * @param value
      *     The value of the variable to be set.
      */
    void var_set_string(const char *name, const nstring &value);

    /**
      * The var_set_charstar method is used to set a substitution variable.
      *
      * @param name
      *     The name of the variable to be set.
      * @param value
      *     The value of the variable to be set.
      */
    void var_set_charstar(const char *name, const char *value);

    /**
      * The var_set_long method is used to set a substitution variable.
      *
      * @param name
      *     The name of the variable to be set.
      * @param value
      *     The value of the variable to be set.
      */
    void var_set_long(const char *name, long value);

    /**
      * The var_set_time method is used to set a substitution variable.
      *
      * @param name
      *     The name of the variable to be set.
      * @param value
      *     The value of the variable to be set.
      */
    void var_set_time(const char *name, time_t value);

    /**
      * The var_optional method is used to mark a substitution variable
      * as optional, meaning that is doesn't have to be used.
      *
      * @param name
      *     The name of the optional variable.
      */
    void var_optional(const char *name);

    /**
      * The var_append_if_unused method is used to mark a substitution
      * variable as one which is appended if it is not used anywhere in
      * the substituition.
      *
      * @param name
      *     The name of the substitution variable.
      */
    void var_append_if_unused(const char *name);

    /**
      * The var_override method is used to mark a substitution
      * variable as one which is to override a built-in substitution.
      *
      * @param name
      *     The name of the substitution variable.
      * @note
      *     Use sparingly.
      */
    void var_override(const char *name);

    /**
      * The var_resubstitute method is used to mark a substitution
      * variable as one which is to have its substitution value
      * re-interpreted for more substitutions.  The default is not to do
      * this.
      *
      * @param name
      *     The name of the substitution variable.
      * @note
      *     Use sparingly.
      */
    void var_resubstitute(const char *name);

    /**
      * The errno_setx method is used to set the $ERRNO value in the
      * given substitition context.
      *
      * @note
      *     There is no method to directly read errno and set it, because
      *     there is at least one malloc() library call and possibly several
      *     others before you get arround to calling this function.
      *     You *must* instead copy the value of errno immediatly after
      *     the offending system call, and before doing inything towards
      *     preparing the error message.
      */
    void errno_setx(int value);

    /**
      * The substitute method is used to perform substitutions on
      * strings.  Usually command strings, but not always.
      *
      * The format of substitutions, and the commonly available
      * substitutions, are described in aesub(5).
      *
      * @param cp
      *     The change to provide context for the substitution.
      * @param the_command
      *     The string to be sustituted into.
      */
    string_ty *substitute(change::pointer cp, string_ty *the_command);

    /**
      * The substitute_p method is used to substitute into the given string.
      *
      * @param pp
      *     The project to provide context for the substitution.
      * @param the_command
      *     The string to be sustituted into.
      *     See aesub(5) for valid substitutions.
      */
    string_ty *substitute_p(project *pp, string_ty *the_command);

    /**
      * The sub_intl method is used to substitute into the given string.
      *
      * @param substme
      *     The string to be sustituted into.
      *     See aesub(5) for valid substitutions.
      */
    string_ty *subst_intl(const char *substme);

    /**
      * The subst_intl_project method is used to set the project for the
      * substitution context.
      *
      * @param pp
      *     The project for context.
      */
    void subst_intl_project(struct project *pp);

    /**
      * The subst_intl_change method is used to set the change for the
      * substitution context.
      *
      * @param cp
      *     The change for context.
      */
    void subst_intl_change(change::pointer cp);

    /**
      * The error_intl method is used to internationalize an error
      * message (vis the gettext function) then substitute it, and then
      * print it.
      *
      * @param message
      *     The error message to be processed.
      */
    void error_intl(const char *message);

    /**
      * The fatal_intl method is used to internationalize an error
      * message (vis the gettext function) then substitute it, then
      * print it, and then exit with exist status 1.
      *
      * @param message
      *     The error message to be processed.
      * @note
      *     This method does not return.
      */
    void fatal_intl(const char *message) NORETURN;

    /**
      * The verbose_intl method is used to internationalize an error
      * message (vis the gettext function) then substitute it, and then
      * print it.  If the verbose option is not in effect, nothing will
      * be printed.
      *
      * @param message
      *     The error message to be processed.
      */
    void verbose_intl(const char *message);

    /**
      * The error_set method is used to mark a substitution context as
      * erroneous.  This occurs when there is something wrong with the
      *  message being substituted.
      *
      * @param message
      *     The error message describing what is wrong with the
      *     substitution message currently being processed.
      * @note
      *     This method is only to be used by the built-in functions as
      *     feedback when something goes wrong.
      */
    void error_set(const char *message);

    /**
      * The project_get method is used to obtain a pointer to the
      * project for this context.
      *
      * @returns
      *     Pointer to the project, or NULL if no project is relevant at
      *     this point.
      * @note
      *     This method is only to be used by the built-in functions to
      *     obtain their context.
      */
    project *project_get();

    /**
      * The change_get method is used to obtain a pointer to the
      * change for this context.
      *
      * @returns
      *     Pointer to the change, or NULL if no change is relevant at
      *     this point.
      * @note
      *     This method is only to be used by the built-in functions to
      *     obtain their context.
      */
    change::pointer change_get();

    /**
      * The errno_sequester_get method is used to obtain the sequestered
      * errno value.  This method shall only be called by sub_errno().
      */
    int errno_sequester_get() const;

    /**
      * The subst method is used to substitute the given string.
      *
      * @param msg
      *     The string to be substituted into.
      * @returns
      *     The string result of the substitution.
      */
    wstring subst(const wstring &msg);

    /**
      * The subst_inst_wide method is used to substitute the given
      * string.  The message will be passed through gettext before being
      * substituted.
      *
      * @param msg
      *     The string to be substituted into.
      * @returns
      *     The string result of the substitution.
      */
    wstring subst_intl_wide(const char *msg);

private:
    sub_diversion_stack diversion_stack;
    sub_functor_list var_list;
    change::pointer cp;
    project *pp;

    const char *suberr;
    int errno_sequester;
    const char *file_name;
    int line_number;

    /**
      * The diversion_close method is used to release a diversion when
      * it has been exhausted.
      */
    void diversion_close();

    enum getc_type
    {
        getc_type_control,
        getc_type_data
    };

    /**
      * The getc_meta method is used to get a character from the current
      * input string.  When the current string is exhaused, the previous
      * string is resumed.
      *
      * @returns
      *      the chacater, or NUL to indicate end of input
      */
    wchar_t getc_meta(getc_type &c);

    /**
      * The getc_meta_undo function is used to give back a character
      * output by getc_meta.
      *
      * @param c
      *     character being given back
      * @note
      *    Only push back what was read.
      */
    void getc_meta_undo(wchar_t c);

    /**
      * The getch method is used to get the next character of input from
      * the diversion stack and characterize it appropriately.
      *
      * It calls the dollar() method at the appropriate times.
      */
    wchar_t getch(getc_type &tr);

    /**
      * The dollar method is used to perform dollar ($) substitutions.
      * On entry, the $ is expected to have been consumed.
      *
      * The substitution is usually achieved as a side-effect, by using
      * the diversion_stack.
      *
      * @returns
      *     wchar_t a character to deliver as output, or a wide NUL
      *     charcater if none.
      */
    wchar_t dollar();

    /**
      * The execute method is used to perform a substitution once all
      * the arguments are known.  The firsdt (0'th) argument is the name
      * of the substitution to perform.
      */
    void execute(const wstring_list &arg);

    /**
      * The copy constructor.  Do not use.
      */
    sub_context_ty(const sub_context_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    sub_context_ty &operator=(const sub_context_ty &);
};

inline sub_context_ty *
sub_context_New(const char *file, int line)
{
    return new sub_context_ty(file, line);
}

inline sub_context_ty *
sub_context_new()
{
    return new sub_context_ty();
}

inline void
sub_context_delete(sub_context_ty *scp)
{
    delete scp;
}

inline void
sub_var_clear(sub_context_ty *scp)
{
    scp->clear();
}

inline void ATTR_PRINTF(3, 4)
sub_var_set_format(sub_context_ty *scp, const char *name, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    scp->var_set_vformat(name, fmt, ap);
    va_end(ap);
}

/**
  * The sub_var_set_string function is used to set the value of a
  * substitution variable.
  *
  * @param scp
  *     The substitution context to set the variable within.
  * @param name
  *     The name of the variable to be set.
  * @param value
  *     The value of the variable to be set.
  */
inline void
sub_var_set_string(sub_context_ty *scp, const char *name, string_ty *value)
{
    scp->var_set_string(name, value);
}

/**
  * The sub_var_set_string function is used to set the value of a
  * substitution variable.
  *
  * @param scp
  *     The substitution context to set the variable within.
  * @param name
  *     The name of the variable to be set.
  * @param value
  *     The value of the variable to be set.
  */
inline void
sub_var_set_string(sub_context_ty *scp, const char *name, const nstring &value)
{
    scp->var_set_string(name, value);
}

inline void
sub_var_set_charstar(sub_context_ty *scp, const char *name, const char *value)
{
    scp->var_set_charstar(name, value);
}

inline void
sub_var_set_long(sub_context_ty *scp, const char *name, long value)
{
    scp->var_set_long(name, value);
}

inline void
sub_var_set_time(sub_context_ty *scp, const char *name, time_t value)
{
    scp->var_set_time(name, value);
}

inline void
sub_var_optional(sub_context_ty *scp, const char *name)
{
    scp->var_optional(name);
}

inline void
sub_var_append_if_unused(sub_context_ty *scp, const char *name)
{
    scp->var_append_if_unused(name);
}

inline void
sub_var_override(sub_context_ty *scp, const char *name)
{
    scp->var_override(name);
}

inline void
sub_var_resubstitute(sub_context_ty *scp, const char *name)
{
    scp->var_resubstitute(name);
}

/**
  * Set the $ERRNO value in the given substitition context.
  *
  * @note
  *     There is no method to directly read errno and set it, because
  *     there is at least one malloc() library call and possibly several
  *     others before you get around to calling this function.
  *     You *must* instead copy the value of errno immediatly after
  *     the offending system call, and before doing anything towards
  *     preparing the error message.
  */
inline void
sub_errno_setx(sub_context_ty *scp, int value)
{
    scp->errno_setx(value);
}

inline string_ty *
substitute(sub_context_ty *scp, change::pointer cp, string_ty *the_command)
{
    if (!scp)
    {
        sub_context_ty inner;
        return inner.substitute(cp, the_command);
    }
    return scp->substitute(cp, the_command);
}

inline string_ty *
substitute_p(sub_context_ty *scp, struct project *pp, string_ty *the_command)
{
    if (!scp)
    {
        sub_context_ty inner;
        return inner.substitute_p(pp, the_command);
    }
    return scp->substitute_p(pp, the_command);
}

inline string_ty *
subst_intl(sub_context_ty *scp, const char *substme)
{
    if (!scp)
    {
        sub_context_ty inner;
        return inner.subst_intl(substme);
    }
    return scp->subst_intl(substme);
}

inline void
subst_intl_project(sub_context_ty *scp, struct project *pp)
{
    if (!scp)
    {
        sub_context_ty inner;
        inner.subst_intl_project(pp);
    }
    else
        scp->subst_intl_project(pp);
}

inline void
subst_intl_change(sub_context_ty *scp, change::pointer cp)
{
    if (!scp)
    {
        sub_context_ty inner;
        inner.subst_intl_change(cp);
    }
    else
        scp->subst_intl_change(cp);
}

inline void
error_intl(sub_context_ty *scp, const char *message)
{
    if (!scp)
    {
        sub_context_ty inner;
        inner.error_intl(message);
    }
    else
        scp->error_intl(message);
}

inline void
fatal_intl(sub_context_ty *, const char *) NORETURN;

inline void
fatal_intl(sub_context_ty *scp, const char *message)
{
    if (!scp)
    {
        sub_context_ty inner;
        inner.fatal_intl(message);
    }
    else
        scp->fatal_intl(message);
}

inline void
verbose_intl(sub_context_ty *scp, const char *message)
{
    if (!scp)
    {
        sub_context_ty temp;
        temp.verbose_intl(message);
    }
    else
        scp->verbose_intl(message);
}

//
// The inline i18n function does nothing by itself, but it serves as a
// keyword for the xgettext program, when extracting internationalized
// msgid keys.
//
inline const char *
i18n(const char *x)
{
    return x;
}

inline DEPRECATED void
sub_context_error_set(sub_context_ty *scp, const char *message)
{
    scp->error_set(message);
}

inline project *
sub_context_project_get(sub_context_ty *scp)
{
    return scp->project_get();
}

inline change::pointer
sub_context_change_get(sub_context_ty *scp)
{
    return scp->change_get();
}

#endif // SUB_H
// vim: set ts=8 sw=4 et :
