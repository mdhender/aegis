//
// aegis - project change supervisor
// Copyright (C) 1994-1996, 1999, 2002-2008, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <common/nstring/accumulator.h>
#include <common/trace.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/sprintf.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>

//
// maximum width for numbers
//
#define MAX_WIDTH 200

//
// info for cranking through the arguments
//
static size_t ac;
static size_t ai;
static rpt_value::pointer *av;


//
// NAME
//      build fake - construct formatting specifier string
//
// SYNOPSIS
//      void build_fake(char *fake, size_t fake_len, int flag, int width,
//              int prec, int qual, int spec);
//
// DESCRIPTION
//      The build_fake function is used to construct a format
//      specification string from the arguments presented.  This is
//      used to guarantee exact replication of sprintf behaviour.
//
// ARGUMENTS
//      fake    - buffer to store results
//      flag    - the flag specified (zero if not)
//      width   - the width specified (zero if not)
//      prec    - the precision specified (zero if not)
//      qual    - the qualifier specified (zero if not)
//      spec    - the formatting specifier specified
//

static void
build_fake(char *fake, size_t fake_len, int flag, int width, int precision,
    int qualifier, int specifier)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    char            *fp;

    fp = fake;
    *fp++ = '%';
    if (flag)
        *fp++ = flag;
    if (width > 0)
    {
        snprintf(fp, fake + fake_len - fp - 5, "%d", width);
        fp += strlen(fp);
    }
    *fp++ = '.';
    snprintf(fp, fake + fake_len - fp - 3, "%d", precision);
    fp += strlen(fp);
    if (qualifier)
        *fp++ = qualifier;
    *fp++ = specifier;
    *fp = 0;
}


rpt_func_sprintf::~rpt_func_sprintf()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_func_sprintf::rpt_func_sprintf()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_func::pointer
rpt_func_sprintf::create()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_func_sprintf());
}


const char *
rpt_func_sprintf::name()
    const
{
    return "sprintf";
}


bool
rpt_func_sprintf::optimizable()
    const
{
    return true;
}


bool
rpt_func_sprintf::verify(const rpt_expr::pointer &ep)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return (ep->get_nchildren() >= 1);
}


static rpt_value::pointer
get_arg(const rpt_expr::pointer &ep)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (ai >= ac)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "sprintf");
        nstring s(sc.subst_intl(i18n("$function: too few arguments")));
        return rpt_value_error::create(ep->get_pos(), s);
    }
    return av[ai++];
}


static rpt_value::pointer
get_arg_string(const rpt_expr::pointer &ep)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value::pointer vp1 = get_arg(ep);
    assert(!vp1->is_an_error());
    rpt_value::pointer vp2 = rpt_value::stringize(vp1);
    if (dynamic_cast<rpt_value_string *>(vp2.get()))
        return vp2;

    sub_context_ty sc;
    sc.var_set_charstar("Function", "sprintf");
    sc.var_set_charstar("Name", vp2->name());
    sc.var_set_long("Number", (long)ai);
    nstring s
    (
        sc.subst_intl
        (
            i18n("$function: argument $number: string value required "
                "(was given $name)")
        )
    );
    return rpt_value_error::create(ep->get_pos(), s);
}


static rpt_value::pointer
get_arg_integer(const rpt_expr::pointer &ep, bool real_ok)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value::pointer vp1 = get_arg(ep);
    assert(!vp1->is_an_error());

    if (real_ok)
    {
        rpt_value::pointer vp2 = rpt_value::integerize(vp1);
        if (dynamic_cast<rpt_value_integer *>(vp2.get()))
            return vp2;
    }
    else
    {
        rpt_value::pointer vp2 = rpt_value::arithmetic(vp1);
        if (dynamic_cast<rpt_value_integer *>(vp2.get()))
            return vp2;
    }

    sub_context_ty sc;
    sc.var_set_charstar("Function", "sprintf");
    sc.var_set_charstar("Name", vp1->name());
    sc.var_set_long("Number", (long)ai);
    nstring s
    (
        sc.subst_intl
        (
            i18n("$function: argument $number: integer value required "
                "(was given $name)")
        )
    );
    return rpt_value_error::create(ep->get_pos(), s);
}


static rpt_value::pointer
get_arg_real(const rpt_expr::pointer &ep)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value::pointer vp1 = get_arg(ep);
    assert(!vp1->is_an_error());
    rpt_value::pointer vp2 = rpt_value::realize(vp1);
    if (dynamic_cast<rpt_value_real *>(vp2.get()))
        return vp2;

    sub_context_ty sc;
    sc.var_set_charstar("Function", "sprintf");
    sc.var_set_charstar("Name", vp1->name());
    sc.var_set_long("Number", (long)ai);
    nstring s
    (
        sc.subst_intl
        (
            i18n("$function: argument $number: real value required (was "
                "given $name)")
        )
    );
    return rpt_value_error::create(ep->get_pos(), s);
}


rpt_value::pointer
rpt_func_sprintf::run(const rpt_expr::pointer &ep, size_t argc,
    rpt_value::pointer *argv) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));

    //
    // Build the result string in a temporary buffer.
    // Grow the temporary buffer as necessary.
    //
    static nstring_accumulator buffer;
    buffer.clear();

    //
    // It is important to only make one pass across the variable argument
    // list.  Behaviour is undefined for more than one pass.
    //
    ac = argc;
    ai = 0;
    av = argv;

    //
    // get the format string
    //
    rpt_value::pointer fmt_vp = get_arg_string(ep);
    if (fmt_vp->is_an_error())
        return fmt_vp;
    rpt_value_string *fmt_vsp = dynamic_cast<rpt_value_string *>(fmt_vp.get());
    assert(fmt_vsp);
    const char *fmt = fmt_vsp->query().c_str();

    //
    // interpret the format string
    //
    for (;;)
    {
        unsigned char c = *fmt++;
        if (!c)
            break;
        if (c != '%')
        {
            buffer.push_back(c);
            continue;
        }
        c = *fmt++;
        if (!c)
            break;

        //
        // get optional flag
        //
        int flag = 0;
        switch (c)
        {
        case '+':
        case '-':
        case '#':
        case '0':
        case ' ':
            flag = c;
            c = *fmt++;
            break;
        }

        //
        // get optional width
        //
        int width = 0;
        bool width_set = false;
        switch (c)
        {
        case '*':
            {
                rpt_value::pointer w = get_arg_integer(ep, false);
                if (w->is_an_error())
                    return w;

                rpt_value_integer *wip =
                    dynamic_cast<rpt_value_integer *>(w.get());
                assert(wip);
                width = wip->query();
                if (width < 0)
                {
                    flag = '-';
                    width = -width;
                }
                if (width > MAX_WIDTH)
                {
                    sub_context_ty sc;
                    sc.var_set_charstar("Function", "sprintf");
                    sc.var_set_long("Number", (long)ai);
                    sc.var_set_long("Value", width);
                    nstring s
                    (
                        sc.subst_intl
                        (
                            i18n("$function: argument $number: width "
                                "$value out of range")
                        )
                    );
                    return rpt_value_error::create(ep->get_pos(), s);
                }
                c = *fmt++;
                width_set = true;
            }
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            for (;;)
            {
                width = width * 10 + c - '0';
                c = *fmt++;
                switch (c)
                {
                default:
                    break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    continue;
                }
                break;
            }
            if (width > MAX_WIDTH)
            {
                sub_context_ty sc;
                sc.var_set_charstar("Function", "sprintf");
                sc.var_set_long("Number", (long)ai);
                sc.var_set_long("Value", width);
                nstring s
                (
                    sc.subst_intl
                    (
                        i18n("$function: argument $number: width $value "
                            "out of range")
                    )
                );
                return rpt_value_error::create(ep->get_pos(), s);
            }
            width_set = true;
            break;

        default:
            break;
        }

        //
        // get optional precision
        //
        int prec = 0;
        bool prec_set = false;
        if (c == '.')
        {
            c = *fmt++;
            switch (c)
            {
            default:
                prec_set = true;
                break;

            case '*':
                {
                    c = *fmt++;
                    rpt_value::pointer p = get_arg_integer(ep, false);
                    if (p->is_an_error())
                        return p;
                    rpt_value_integer *pip =
                        dynamic_cast<rpt_value_integer *>(p.get());
                    assert(pip);
                    prec = pip->query();
                    if (prec < 0 || prec > MAX_WIDTH)
                    {
                        sub_context_ty sc;
                        sc.var_set_charstar("Function", "sprintf");
                        sc.var_set_long("Number", (long)ai);
                        sc.var_set_long("Value", prec);
                        nstring s
                        (
                            sc.subst_intl
                            (
                                i18n("$function: argument $number: "
                                    "precision of $value is out of range")
                            )
                        );
                        return rpt_value_error::create(ep->get_pos(), s);
                    }
                    prec_set = true;
                }
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                for (;;)
                {
                    prec = prec * 10 + c - '0';
                    c = *fmt++;
                    switch (c)
                    {
                    default:
                        break;

                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        continue;
                    }
                    break;
                }
                if (prec > MAX_WIDTH)
                {
                    sub_context_ty sc;
                    sc.var_set_charstar("Function", "sprintf");
                    sc.var_set_long("Number", (long)ai);
                    sc.var_set_long("Value", prec);
                    nstring s
                    (
                        sc.subst_intl
                        (
                            i18n("$function: argument $number: precision "
                                "of $value is out of range")
                        )
                    );
                    return rpt_value_error::create(ep->get_pos(), s);
                }
                prec_set = true;
                break;
            }
        }

        //
        // get the optional qualifier
        //
        for (;;)
        {
            switch (c)
            {
            case 'h':
            case 'j':
            case 'l':
            case 'L':
            case 't':
            case 'z':
                c = *fmt++;
                continue;

            default:
                break;
            }
            break;
        }

        //
        // get conversion specifier
        //
        if (!c)
            break;
        switch (c)
        {
        default:
            {
                sub_context_ty sc;
                sc.var_set_charstar("Function", "sprintf");
                sc.var_set_format("Name", "%c", c);
                nstring s
                (
                    sc.subst_intl
                    (
                        i18n("$function: unknown format specifier '$name'")
                    )
                );
                return rpt_value_error::create(ep->get_pos(), s);
            }

        case '%':
            buffer.push_back(c);
            continue;

        case 'c':
            {
                rpt_value::pointer tmp = get_arg_integer(ep, true);
                if (tmp->is_an_error())
                    return tmp;
                rpt_value_integer *ip =
                    dynamic_cast<rpt_value_integer *>(tmp.get());
                assert(ip);
                long a = ip->query();

                if (!prec_set)
                    prec = true;
                char fake[MAX_WIDTH];
                build_fake(fake, sizeof(fake), flag, width, prec, 0, c);
                char num[MAX_WIDTH + 1];
                // g++ -Wformat-nonlitteral will warn here, it is safe to ignore
                snprintf(num, sizeof(num), fake, a);
                buffer.push_back(num);
            }
            break;

        case 'd':
        case 'i':
            {
                rpt_value::pointer tmp = get_arg_integer(ep, true);
                if (tmp->is_an_error())
                    return tmp;
                rpt_value_integer *ip =
                    dynamic_cast<rpt_value_integer *>(tmp.get());
                assert(ip);
                long a = ip->query();

                if (!prec_set)
                    prec = true;
                char fake[MAX_WIDTH];
                build_fake(fake, sizeof(fake), flag, width, prec, 'l', c);
                char num[MAX_WIDTH + 1];
                // g++ -Wformat-nonlitteral will warn here, it is safe to ignore
                snprintf(num, sizeof(num), fake, a);
                buffer.push_back(num);
            }
            break;

        case 'e':
        case 'f':
        case 'g':
        case 'E':
        case 'F':
        case 'G':
            {
                rpt_value::pointer tmp = get_arg_real(ep);
                if (tmp->is_an_error())
                    return tmp;
                rpt_value_real *rp = dynamic_cast<rpt_value_real *>(tmp.get());
                assert(rp);
                double a = rp->query();

                if (!prec_set)
                    prec = 6;
                if (prec > MAX_WIDTH)
                    prec = MAX_WIDTH;
                char fake[MAX_WIDTH];
                build_fake(fake, sizeof(fake), flag, width, prec, 0, c);
                char num[MAX_WIDTH + 1];
                // g++ -Wformat-nonlitteral will warn here, it is safe to ignore
                snprintf(num, sizeof(num), fake, a);
                buffer.push_back(num);
            }
            break;

        case 'u':
        case 'o':
        case 'x':
        case 'X':
            {
                rpt_value::pointer tmp = get_arg_integer(ep, true);
                if (tmp->is_an_error())
                    return tmp;
                rpt_value_integer *ip =
                    dynamic_cast<rpt_value_integer *>(tmp.get());
                assert(ip);
                unsigned long a = ip->query();

                if (!prec_set)
                    prec = true;
                char fake[MAX_WIDTH];
                build_fake(fake, sizeof(fake), flag, width, prec, 'l', c);
                char num[MAX_WIDTH + 1];
                // g++ -Wformat-nonlitteral will warn here, it is safe to ignore
                snprintf(num, sizeof(num), fake, a);
                buffer.push_back(num);
            }
            break;

        case 's':
            {
                rpt_value::pointer tmp = get_arg_string(ep);
                if (tmp->is_an_error())
                    return tmp;
                rpt_value_string *sp =
                    dynamic_cast<rpt_value_string *>(tmp.get());
                assert(sp);
                nstring a(sp->query());

                size_t len = a.size();
                if (!prec_set)
                    prec = len;
                if (len < (size_t)prec)
                    prec = len;
                if (!width_set)
                    width = prec;
                if (width < prec)
                    width = prec;
                len = width;
                if (flag != '-')
                {
                    while (width > prec)
                    {
                        buffer.push_back(' ');
                        width--;
                    }
                }
                buffer.push_back(a.c_str(), prec);
                width -= prec;
                if (flag == '-')
                {
                    while (width > 0)
                    {
                        buffer.push_back(' ');
                        width--;
                    }
                }
            }
            break;
        }
    }
    // assert(buffer.count_nul_characters() == 0);

    if (ai < argc)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "sprintf");
        sc.var_set_long("Number1", (long)argc);
        sc.var_set_long("Number2", (long)ai);
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: too many arguments ($number1 given, "
                    "only $number2 used)")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }

    //
    // all done
    //
    return rpt_value_string::create(buffer.mkstr());
}


// vim: set ts=8 sw=4 et :
