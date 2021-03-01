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

#ifndef LIBAEGIS_INTROSPECTOR_STRUCTURE_H
#define LIBAEGIS_INTROSPECTOR_STRUCTURE_H

#include <common/symtab/template.h>
#include <libaegis/introspector.h>

/**
  * The introspector_structure class is used to represent
  */
class introspector_structure:
    public introspector
{
public:
    typedef aegis_shared_ptr<introspector_structure> pointer;

    /**
      * The destructor.
      */
    virtual ~introspector_structure();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param name
      *     The name of the class fo the structure being manipulated.
      */
    introspector_structure(const nstring &name);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param name
      *     The name of the class fo the structure being manipulated.
      */
    static pointer create(const nstring &name);

    class adapter
    {
    public:
        typedef aegis_shared_ptr<adapter> pointer;
        virtual ~adapter() {}
        virtual bool is_set() = 0;
        virtual bool redefinition_ok() = 0;
        virtual introspector::pointer introspector_factory() = 0;
    protected:
        adapter() {}
    private:
        adapter(const adapter &rhs);
        adapter &operator=(const adapter &rhs);
    };

    template <class T>
    class adapter_by_method:
        public adapter
    {
    public:
        ~adapter_by_method() {}

    private:
        adapter_by_method(T *a_obj, introspector::pointer (T::*a_fac)(),
                bool (T::*a_isset)() const) :
            obj(a_obj),
            fac(a_fac),
            isset(a_isset)
        {
        }

    public:
        static pointer
        create(T *a_obj, introspector::pointer (T::*a_fac)(),
            bool (T::*a_isset)() const)
        {
            return pointer(new adapter_by_method(a_obj, a_fac, a_isset));
        }

    protected:
        // See base class for documentation.
        introspector::pointer
        introspector_factory()
        {
            return (obj->*fac)();
        }

        // See base class for documentation.
        bool
        is_set()
        {
            return (obj->*isset)();
        }

        // See base class for documentation.
        bool
        redefinition_ok()
        {
            return false;
        }

    private:
        T *obj;
        introspector::pointer (T::*fac)();
        bool (T::*isset)() const;
    };

    /**
      * The register_member method is used to add another member name and
      * factory generator.
      *
      * @param name
      *     The name of the member
      * @param factory
      *     How to manipulate the member.
      */
    void register_member(const nstring &name, const adapter::pointer &factory);

protected:
    // See base class for documentation.
    nstring get_name() const;

    // See base class for documentation.
    introspector::pointer field(const nstring &name);

private:
    /**
      * The name instance variable is used to remember the name of the
      * class fo the structure being manipulated.
      */
    nstring name;

    /**
      * The members instance variable is used to remember the mapping
      * from member name to member introspector factory.
      */
    symtab<adapter::pointer> members;

    /**
      * The default constructor.  Do not use.
      */
    introspector_structure();

    /**
      * The copy constructor.  Do not use.
      */
    introspector_structure(const introspector_structure &);

    /**
      * The assignment operator.  Do not use.
      */
    introspector_structure &operator=(const introspector_structure &);
};

#endif // LIBAEGIS_INTROSPECTOR_STRUCTURE_H
