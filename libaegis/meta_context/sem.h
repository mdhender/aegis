//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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

#ifndef LIBAEGIS_META_CONTEXT_SEM_H
#define LIBAEGIS_META_CONTEXT_SEM_H

#include <libaegis/meta_context.h>

class input; // forward
class meta_type; // forward

/**
  * The meta_context_sem class is used to represent the processig
  * required to parse an Aegis meta-data file, using the original style
  * of meta-data handling.
  */
class meta_context_sem:
    public meta_context
{
public:
    /**
      * The destructor.
      */
    virtual ~meta_context_sem();

    /**
      * The default constructor.
      */
    meta_context_sem();

    /**
      * The parse method is used to parse a file against the supplied
      * meta-data type.
      *
      * @param filename
      *     The name of the file to be parsed.
      * @param type
      *     The type of data expected.
      */
    void *parse(const nstring &filename, meta_type *type);

    /**
      * The parse_env method is used to parse an environment variable
      * against the supplied meta-data type.
      *
      * @param name
      *     The name of the environment variable to be parsed.
      * @param type
      *     The type of data expected.
      */
    void *parse_env(const nstring &name, meta_type *type);

    /**
      * The parse_input method is used to parse the given input against
      * the supplied meta-data type.
      *
      * @param ifp
      *     The input stream to be parsed.
      * @param type
      *     The type of data expected.
      */
    void *parse_input(input &ifp, meta_type *type);

protected:
    // See base class for documentation.
    void integer(long n);

    // See base class for documentation.
    void real(double n);

    // See base class for documentation.
    void string(const nstring &s);

    // See base class for documentation.
    void enumeration(const nstring &s);

    // See base class for documentation.
    void list();

    // See base class for documentation.
    void list_end();

    // See base class for documentation.
    void field(const nstring &name);

    // See base class for documentation.
    void field_end();

    // See base class for documentation.
    void end();

private:
    struct sem_ty
    {
        meta_type *type;
        sem_ty *next;
        void *addr;
    };

    /**
      * The root instance variable is used to remember the top of the
      * linked list, describing the push-down stack of types being
      * parsed.
      */
    sem_ty *root;

    /**
      * The sem_push method is used to push another entry onto the stack
      * of types being parsed.
      *
      * @param type
      *     The type of data being parsed.
      * @param addr
      *     The address of the data being parsed.
      */
    void sem_push(meta_type *type, void *addr);

    /**
      * The sem_pop method is used to pop the top entry from the stack
      * of types being parsed.
      */
    void sem_pop();

    /**
      * The copy constructor.  Do not use.
      */
    meta_context_sem(const meta_context_sem &);

    /**
      * The assignment operator.  Do not use.
      */
    meta_context_sem &operator=(const meta_context_sem &);
};

#endif // LIBAEGIS_META_CONTEXT_SEM_H
