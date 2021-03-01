//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
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

#include <common/ac/stddef.h>
#include <list>

#include <common/ac/shared_ptr.h>
#include <common/ac/stdio.h>


class junk
{
public:
    typedef aegis_shared_ptr<junk> pointer;

    ~junk();
    junk();
    junk(int arg);
    junk(const junk &rhs);
    junk &operator=(const junk &rhs);
    static pointer create();
    static pointer create(int arg);
    int a;
};


junk::~junk()
{
    printf("destructor (%d)\n", a);
}


junk::junk() :
    a(0)
{
    printf("default constructor (0)\n");
}


junk::pointer
junk::create()
{
    return pointer(new junk());
}


junk::junk(int arg) :
    a(arg)
{
    printf("constructor (%d)\n", a);
}


junk::pointer
junk::create(int arg)
{
    return pointer(new junk(arg));
}


junk::junk(const junk &arg) :
    a(arg.a)
{
    printf("copy constructor\n");
}


junk &
junk::operator=(const junk &arg)
{
    if (this != &arg)
        a = arg.a;
    printf("assignment operator\n");
    return *this;
}


int
main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    junk::pointer j1 = junk::create(4);

    {
        junk::pointer j2 = j1;
        j2->a = 4;
    }

    std::list<junk::pointer> foo;
    foo.push_back(junk::create(1));
    foo.push_back(junk::create(2));
    std::list<junk::pointer> bar = foo;
    foo.push_back(junk::create(3));
    std::list<junk::pointer> baz;
    baz = foo;

    return 0;
}
