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
    printf("destructor\n");
}


junk::junk() :
    a(0)
{
    printf("default constructor\n");
}


junk::pointer
junk::create()
{
    return pointer(new junk());
}


junk::junk(int arg) :
    a(arg)
{
    printf("constructor\n");
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

    return 0;
}
