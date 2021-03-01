//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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

#include <aecvsserver/module.h>


module_ty::options::~options()
{
    A = 0;
    C = 0;
    if (D)
	str_free(D);
    D = 0;
    d = 0;
    f = 0;
    if (I)
	str_free(I);
    I = 0;
    if (j)
	str_free(j);
    j = 0;
    if (k)
	str_free(k);
    k = 0;
    l = 0;
    if (m)
	str_free(m);
    m = 0;
    P = 0;
    p = 0;
    R = 0;
    if (r)
	str_free(r);
    r = 0;
    if (W)
	str_free(W);
    W = 0;
}


module_ty::options::options() :
    A(0),
    C(0),
    D(0),
    d(0),
    f(0),
    I(0),
    j(0),
    k(0),
    l(0),
    m(0),
    P(0),
    p(0),
    R(0),
    r(0),
    W(0)
{
}


module_ty::options::options(const options &arg) :
    A(arg.A),
    C(arg.C),
    D(arg.D ? str_copy(arg.D) : 0),
    d(arg.d),
    f(arg.f),
    I(arg.I ? str_copy(arg.I) : 0),
    j(arg.j ? str_copy(arg.j) : 0),
    k(arg.k ? str_copy(arg.k) : 0),
    l(arg.l),
    m(arg.m ? str_copy(arg.m) : 0),
    P(arg.P),
    p(arg.p),
    R(arg.R),
    r(arg.r ? str_copy(arg.r) : 0),
    W(arg.W ? str_copy(arg.W) : 0)
{
}


module_ty::options &
module_ty::options::operator=(const options &arg)
{
    if (this != &arg)
    {
	A = arg.A;
	C = arg.C;
	D = arg.D ? str_copy(arg.D) : 0;
	d = arg.d;
	f = arg.f;
	I = arg.I ? str_copy(arg.I) : 0;
	j = arg.j ? str_copy(arg.j) : 0;
	k = arg.k ? str_copy(arg.k) : 0;
	l = arg.l;
	m = arg.m ? str_copy(arg.m) : 0;
	P = arg.P;
	p = arg.p;
	R = arg.R;
	r = arg.r ? str_copy(arg.r) : 0;
	W = arg.W ? str_copy(arg.W) : 0;
    }
    return *this;
}
