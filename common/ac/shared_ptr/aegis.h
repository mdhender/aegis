//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//

#ifndef COMMON_AC_SHARED_PTR_AEGIS_H
#define COMMON_AC_SHARED_PTR_AEGIS_H

#include <common/error.h> // for assert

/**
  * The aegis_shared_ptr class is used to represent a smart pointer with
  * reference counted copy semantics.  The object pointed to is deleted
  * when the last aegis_shared_ptr pointing to it is destroyed or reset.
  *
  * This class is only used of std::tr1::shared_ptr and
  * boost::shared_ptr are both unavailable.  It only implements as much
  * of a shared_ptr as the Aegis code actually uses.
  */

template<class T>
class aegis_shared_ptr
{
private:
    /**
      * The reference_count instanve variable is used to remember the
      * location of the reference count.  By having the reference count
      * separate from the subject, we can cope with compatible pointers,
      * not just exact pointers.
      *
      * This is not ideal because it allocates huge numbers of small
      * objects.  Some heap implementations go slowly when faced with
      * many small allocations.  Some heap implementations waste a lot
      * of memory when faced with many small allocations.
      */
    long *reference_count;

    /**
      * The subject instance variable is used to remember the location
      * of the object being reference counted.  By having it separate
      * from the reference count, we can skip one indirection.
      */
    T *subject;

    /**
      * The valid method is used to establish the correctness of the
      * internal state.  Think of it as the contract.
      */
    bool
    valid()
        const
    {
        return
            (
                (reference_count != 0)
            ?
                (*reference_count > 0 && subject != 0)
            :
                (subject == 0)
            );
    }

public:
    /**
      * The destructor.
      *
      * @Note
      *     This destructor is not virtual.  Thou shalt not derive from
      *     this class.
      */
    ~aegis_shared_ptr()
    {
        reset();
    }

    /**
      * The reset method is used to drop the reference, if there is one.
      */
    void
    reset()
    {
        assert(valid());
        if (subject)
        {
            --*reference_count;
            if (*reference_count <= 0)
            {
                delete reference_count;
                reference_count = 0;
                delete subject;
                subject = 0;
            }
            assert(valid());
        }
    }

    /**
      * The constructor.
      */
    aegis_shared_ptr() :
        reference_count(0),
        subject(0)
    {
        assert(valid);
    }

    /**
      * The constructor.  It permits initialization from any compatable
      * pointer.  Y must be a complete type.
      *
      * @param rhs
      *     The object pointed to, the object to be managed.
      */
    template<class Y> explicit
    aegis_shared_ptr(Y *rhs) :
        reference_count(0),
        subject(0)
    {
        if (rhs)
        {
            reference_count = new long;
            *reference_count = 1;
            subject = rhs;
        }
        assert(valid());
    }

    /**
      * The copy constructor.
      *
      * @param rhs
      *     The smart pointer to be copied.
      */
    aegis_shared_ptr(const aegis_shared_ptr &rhs) :
        reference_count(0),
        subject(0)
    {
        assert(rhs.valid());
        if (rhs.subject)
        {
            reference_count = rhs.reference_count;
            subject = rhs.subject;
            ++*reference_count;
        }
        assert(valid());
    }

    /**
      * The copy constructor.
      *
      * @param rhs
      *     The smart pointer to be copied.
      */
    template<class Y>
    aegis_shared_ptr(const aegis_shared_ptr<Y> &rhs) :
        reference_count(0),
        subject(0)
    {
        assert(rhs.valid());
        if (rhs.subject)
        {
            reference_count = rhs.reference_count;
            subject = rhs.subject;
            ++*reference_count;
        }
        assert(valid());
    }

    /**
      * The assignment operator.
      *
      * @param rhs
      *     The right hand side of the assigment, the pointer to copy.
      */
    aegis_shared_ptr &
    operator=(const aegis_shared_ptr &rhs)
    {
        assert(valid());
        assert(rhs.valid());
        if (this != &rhs && subject != rhs.subject)
        {
            reset();
            if (rhs.subject)
            {
                reference_count = rhs.reference_count;
                subject = rhs.subject;
                ++*reference_count;
            }
        }
        assert(valid());
        return *this;
    }

    /**
      * The assignment operator.
      *
      * @param rhs
      *     The right hand side of the assigment, the pointer to copy.
      */
    template<class Y>
    aegis_shared_ptr &
    operator=(const aegis_shared_ptr<Y> &rhs)
    {
        assert(valid());
        assert(rhs.valid());
        if (this != &rhs && subject != rhs.subject)
        {
            reset();
            if (rhs)
            {
                reference_count = rhs.get_reference_count();
                subject = rhs.get();
                ++*reference_count;
            }
        }
        assert(valid());
        return *this;
    }

#if 0
    /**
      * The constructor
      *
      * @param rhs
      *     The pointer we are to manage.
      */
    template<class Y> explicit
    aegis_shared_ptr(std::auto_ptr<Y> &rhs) :
        reference_count(0),
        subject(0)
    {
        Y *tmp = r.get();
        if (tmp)
        {
            reference_count = new long;
            *reference_count = 1;
            subject = tmp;
        }
        assert(valid());
    }

    /**
      * the assignment operator.
      *
      * @param rhs
      *     The pointer we are to manage.
      */
    template<class Y>
    aegis_shared_ptr &
    operator=(std::auto_ptr<Y> &rhs)
    {
        assert(valid());
        reset();
        if (tmp)
        {
            reference_count = new long;
            *reference_count = 1;
            subject = tmp;
        }
        assert(valid());
        return *this;
    }

#endif

    /**
      * The dereference operator.  Return a reference to the object, not
      * to the smart pointer.
      *
      * This is part of why it's called a "smart pointer" when it's
      * actually neither - it *acts* like a pointer.
      */
    T &
    operator*()
        const
    {
        assert(subject != 0);
        return *subject;
    }

    /**
      * The "pointing at" operator.  Return a pointer to the object, not
      * to the smart pointer.
      *
      * This is part of why it's called a "smart pointer" when it's
      * actually neither - it *acts* like a pointer.
      */
    T *
    operator->()
    {
        assert(subject != 0);
        return subject;
    }

    /**
      * The "pointing at" operator.  Return a pointer to the object, not
      * to the smart pointer.
      *
      * This is part of why it's called a "smart pointer" when it's
      * actually neither - it *acts* like a pointer.
      */
    const T *
    operator->()
        const
    {
        assert(subject != 0);
        return subject;
    }

    /**
      * The get method is used to obtain the pointer to the object.
      */
    T *
    get()
    {
        return subject;
    }

    /**
      * The get method is used to obtain the pointer to the object.
      */
    const T *
    get()
        const
    {
        return subject;
    }

    /**
      * The bool operator, for implicit conversion to bool.
      * Test of the pointers not NULL.
      */
    operator bool()
        const
    {
        return (subject != 0);
    }

    /**
      * The logical not operator.
      * Test if the pointer is NULL.
      */
    bool
    operator!()
        const
    {
        return (subject == 0);
    }

    inline bool
    operator==(const aegis_shared_ptr &rhs)
        const
    {
        return (subject == rhs.get());
    }

    template<class U>
    inline bool
    operator==(const aegis_shared_ptr<U> &rhs)
        const
    {
        return (subject == rhs.get());
    }

    inline bool
    operator!=(aegis_shared_ptr &rhs)
        const
    {
        return (subject != rhs.get());
    }

    template<class U>
    inline bool
    operator!=(aegis_shared_ptr<U> &rhs)
        const
    {
        return (subject != rhs.get());
    }
};

#endif // COMMON_AC_SHARED_PTR_AEGIS_H
