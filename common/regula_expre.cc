//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <common/nstring/accumulator.h>
#include <common/regula_expre.h>
#include <common/trace.h>


regular_expression::~regular_expression()
{
    trace(("~regular_expression()\n"));
    if (compiled)
	regfree(&preg);
}


regular_expression::regular_expression(const nstring &arg) :
    lhs(arg),
    compiled(false)
{
    trace(("regular_expression(lhs = \"%s\")\n", lhs.c_str()));
}


void
regular_expression::set_error(int err)
{
    trace(("regular_expression::set_error()\n"));
    char buffer[100];
    regerror(err, &preg, buffer, sizeof(buffer));
    errstr = nstring(buffer);
}


bool
regular_expression::compile()
{
    if (compiled)
	return true;
    trace(("regular_expression::compile()\n"));
    int err = regcomp(&preg, lhs.c_str(), REG_EXTENDED);
    if (err)
	set_error(err);
    else
	compiled = true;
    return compiled;
}


const char *
regular_expression::strerror()
    const
{
    if (errstr.empty())
	return 0;
    return errstr.c_str();
}


bool
regular_expression::match(const nstring &actual, size_t offset)
{
    trace(("regular_expression::match(actual = \"%.*s\" / \"%s\", "
	"offset = %ld)\n{\n", (int)offset, actual.c_str(),
	actual.c_str() + offset, (long)offset));
    if (!compile())
    {
	trace(("return false;\n}\n"));
	return false;
    }
    int err =
	regexec
	(
	    &preg,
	    actual.c_str() + offset,
	    SIZEOF(regmatch),
	    regmatch,
	    (offset == 0 ? 0 : REG_NOTBOL)
	);
    if (err)
    {
	if (err == REG_NOMATCH)
	    errstr.clear();
	else
	    set_error(err);
	trace(("return false;\n}\n"));
	return false;
    }
#ifdef DEBUG
    for (size_t j = 0; j < 10; ++j)
    {
	if (regmatch[j].rm_so != -1)
	{
	    trace(("regmatch[%d] = {%3d,%3d };\n", (int)j, regmatch[j].rm_so,
		regmatch[j].rm_eo));
	}
    }
#endif
    trace(("return true;\n}\n"));
    return true;
}


bool
regular_expression::match_and_substitute(const nstring &rhs,
    const nstring &actual, long how_many_times, nstring &output)
{
    trace(("regular_expression::match_and_substitute(rhs = \"%s\", "
	"actual = \"%s\", how_many_times = %ld)\n{\n", rhs.c_str(),
	actual.c_str(), how_many_times));
    if (how_many_times <= 0)
	how_many_times = actual.size() + 1;
    nstring_accumulator nsa;
    bool suppress_on_zero = false;
    size_t offset = 0;
    while (offset < actual.size())
    {
	trace(("nsa = \"%.*s\";\n", (int)nsa.size(), nsa.get_data()));
	if (!match(actual, offset))
	{
	    if (!errstr.empty())
	    {
		trace(("return false\n}\n"));
		return false;
	    }
	    break;
	}

	//
        // There is a nasty boundary condition: we need to move past the
        // infinite loop of a zero-length match.  (This can happen for
        // non-trivial patterns.)
        //
        // This test occurs immediately after we have seen a match,
        // except at the beginning.  We step over zero length matches by
        // treating the next character as an invariant portion, mostly
        // as if the match was (1,1) instead of (0,0), but don't insert
        // the RHS (which is what the "suppress" name is all about).
	//
	if (regmatch[0].rm_eo == 0 && suppress_on_zero)
	{
	    nsa.push_back(actual[offset]);
	    ++offset;
	    suppress_on_zero = false;
	    continue;
	}

	//
	// copy the invariant portion
	//
	if (regmatch[0].rm_so)
	    nsa.push_back(actual.c_str() + offset, regmatch[0].rm_so);

	//
	// replace the matched portion with the right hand side
	//
	for (const char *cp = rhs.c_str(); *cp; ++cp)
	{
	    switch (*cp)
	    {
	    default:
		nsa.push_back(*cp);
		break;

	    case '&':
		nsa.push_back
		(
	    	    actual.c_str() + offset + regmatch[0].rm_so,
	    	    regmatch[0].rm_eo - regmatch[0].rm_so
		);
		break;

	    case '\\':
		++cp;
		switch (*cp)
		{
		default:
		    errstr =
			nstring::format
			(
			    "Replacement escape \\%c unknown",
			    *cp
			);
		    trace(("return false;\n}\n"));
		    return false;

		case '&':
		case '\\':
		    nsa.push_back(*cp);
		    break;

		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		case '8': case '9':
		    {
			regmatch_t *rm = &regmatch[*cp - '0'];
			if (rm->rm_so < 0)
			{
			    errstr =
				nstring::format
				(
				    "Replacement \\%c not available",
				    *cp
				);
			    trace(("return false;\n}\n"));
			    return false;
			}
			nsa.push_back
			(
			    actual.c_str() + offset + rm->rm_so,
			    rm->rm_eo - rm->rm_so
			);
		    }
		    break;

		case '\0':
		    errstr = "Replacement escape unterminated";
		    trace(("return false;\n}\n"));
		    return false;
		}
		break;
	    }
	}
	suppress_on_zero = true;

	//
	// Move past the matched portion.
	//
	offset += regmatch[0].rm_eo;

	//
	// There is a nasty boundary condition: we need to
	// move past the infinite loop of a zero-length match.
	// (This can happen for non-trivial patterns.)
	//
	suppress_on_zero = true;

	//
	// Limit how many times we go through this loop.
	//
	--how_many_times;
	if (how_many_times <= 0)
	    break;
    }

    //
    // Collect the tail-end of the input.
    //
    if (offset < actual.size())
	nsa.push_back(actual.c_str() + offset, actual.size() - offset);

    //
    // Build the answer.
    //
    output = nsa.mkstr();
    trace_nstring(output);
    trace(("return true;\n}\n"));
    return true;
}
