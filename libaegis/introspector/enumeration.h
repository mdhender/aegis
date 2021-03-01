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

#ifndef LIBAEGIS_INTROSPECTOR_ENUMERATION_H
#define LIBAEGIS_INTROSPECTOR_ENUMERATION_H

#include <common/nstring.h>
#include <common/symtab/template.h>

#include <libaegis/introspector.h>

/**
  * The introspector_enumeration class is used to represent the manipulations
  * required to manage enumerations by the meta-data parser.
  */
class introspector_enumeration:
    public introspector
{
public:
    /**
      * The destructor.
      */
    virtual ~introspector_enumeration();

    /**
      * The adapter class is used to map an integer into an enumerated
      * value, so that when it is set, it is still type safe.  This
      * is specifically to cope with compilers which are able to have
      * enumerated types which are smaller than integers.
      */
    class adapter
    {
    public:
        typedef aegis_shared_ptr<adapter> pointer;

        virtual ~adapter() {}

    protected:
        adapter() {}

    public:
        virtual void set(int x) = 0;
        virtual bool is_set() = 0;

    private:
        adapter(const adapter &rhs);
        adapter &operator=(const adapter &rhs);
    };

    /**
      * The adapter_by_reference template class is used to create
      * specific adapter classes for enumerations of interest.
      */
    template <typename T>
    class adapter_by_reference:
        public adapter
    {
    public:
        virtual
        ~adapter_by_reference()
        {
        }

    private:
        adapter_by_reference(T &arg) :
            where(arg)
        {
        }

    public:
        static pointer
        create(T &arg)
        {
            return pointer(new adapter_by_reference(arg));
        }

    protected:
        void
        set(int x)
        {
            where = T(x);
        }

        bool
        is_set()
        {
            return false;
        }

    private:
        T &where;

        adapter_by_reference();
        adapter_by_reference(const adapter_by_reference &rhs);
        adapter_by_reference &operator=(const adapter_by_reference &rhs);
    };

    /**
      * The adapter_by_method template class is used to create specific
      * adapter classes for enumerations of interest, wher those fields
      * are operated upon by class methods..
      */
    template <class C, typename E>
    class adapter_by_method:
        public adapter
    {
    public:
        virtual
        ~adapter_by_method()
        {
        }

    private:
        adapter_by_method(C *a_obj, void (C::*a_set)(E),
                bool (C::*a_isset)() const) :
            obj(a_obj),
            set_method(a_set),
            is_set_method(a_isset)
        {
        }

    public:
        static pointer
        create(C *a_obj, void (C::*a_set)(E), bool (C::*a_is_set)() const)
        {
            return pointer(new adapter_by_method(a_obj, a_set, a_is_set));
        }

    protected:
        void
        set(int x)
        {
            (obj->*set_method)(E(x));
        }

        bool
        is_set()
        {
            return (obj->*is_set_method)();
        }

    private:
        C *obj;
        void (C::*set_method)(E value);
        bool (C::*is_set_method)() const;

        adapter_by_method();
        adapter_by_method(const adapter_by_method &rhs);
        adapter_by_method &operator=(const adapter_by_method &rhs);
    };

    /**
      * The report_init method is used to initialize enumeration values
      * in the report generator.
      *
      * @param names
      *     an array of names to be instantiated, values implied by
      *     zero-based array index
      * @param len
      *     the length of the array
      */
    static void report_init(const char **names, size_t len);

protected:
    /**
      * The constructor.  May only be called by derived classes.
      *
      * @param name
      *     The name of the enumerated type.
      * @param how
      *     The indirection required to cast an integer and assign it to
      *     the managed value.
      */
    introspector_enumeration(const nstring &name, const adapter::pointer &how);

    // See base class for documentation.
    void enumeration(const nstring &name);

    // See base class for documentation.
    nstring get_name() const;

private:
    /**
      * The name instance variable is sued to remember the name of the
      * enumerated type, for better error messages.
      */
    nstring name;

    /**
      * The how instance variable is ued to remember where to put the
      * value, once parsed.
      */
    const adapter::pointer &how;

    /**
      * The members instance variable is used to remember the mapping
      * from enumeration tag name to integer value.
      */
    symtab<int> members;

protected:
    /**
      * The default constructor.
      */
    introspector_enumeration();

    /**
      * The register_tags method is used by derived classes to enumerate
      * the names and values of the declared enumeration tags.
      *
      * @param names
      *     The base of an array of enumeration tag names.
      * @param size
      *     The number of names in the array.
      */
    void register_tags(const char **names, size_t size);

private:
    /**
      * The copy constructor.  Do not use.
      */
    introspector_enumeration(const introspector_enumeration &);

    /**
      * The assignment operator.  Do not use.
      */
    introspector_enumeration &operator=(const introspector_enumeration &);
};

#endif // LIBAEGIS_INTROSPECTOR_ENUMERATION_H
