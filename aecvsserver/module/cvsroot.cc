//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <common/ac/ctype.h>

#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <common/error.h>
#include <aecvsserver/fake_version.h>
#include <aecvsserver/file_info.h>
#include <libaegis/gonzo.h>
#include <libaegis/input/string.h>
#include <aecvsserver/module/cvsroot.h>
#include <common/now.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <aecvsserver/response/clear_sticky.h>
#include <aecvsserver/response/clearstatdir.h>
#include <aecvsserver/response/created.h>
#include <aecvsserver/server.h>


module_cvsroot::~module_cvsroot()
{
}


module_cvsroot::module_cvsroot()
{
}


void
module_cvsroot::modified(server_ty *sp, string_ty *file_name, file_info_ty *,
    input &)
{
    //
    // Throw away the file contents the client is sending to us.  We only
    // pretend to allow them to modify us.  (Just closing it will take
    // care of it.)
    //
    server_m
    (
	sp,
	"Module \"%s\" file \"%s\" modify ignored; please\n"
	    "use Aegis project management commands instead.",
	name()->str_text,
	file_name->str_text
    );
}


string_ty *
module_cvsroot::calculate_canonical_name()
    const
{
    // FIXME: memory leak
    return str_from_c("CVSROOT");
}


static int
sanitary_length(string_ty *s, int llen)
{
    if (llen < 15)
	llen = 15;
    llen = 76 - 2 * llen;
    if (llen <= 0)
	return 0;
    const char *cp = s->str_text;
    const char *cp_max = cp + llen;
    while (cp < cp_max && *cp && isprint((unsigned char)*cp))
	++cp;
    return (cp - s->str_text);
}


static void
checkout_modules_inner(string_list_ty *modules, project_ty *pp)
{
    long            k;
    string_ty       *s;
    string_ty       *desc;
    int             desc_len;

    //
    // Add the cannonical name of this project to the list
    // with a project description as a comment.
    //
    s = project_name_get(pp);
    desc = project_brief_description_get(pp);
    desc_len = sanitary_length(desc, s->str_length);
    s =
	str_format
	(
	    "%-15s %-15s # %.*s\n",
	    s->str_text,
	    s->str_text,
	    desc_len,
	    desc->str_text
	);
    modules->push_back(s);
    str_free(s);

    //
    // check each change
    // add it to the list of it is being developed
    // recurse if it is an active branch
    //
    for (k = 0; ; ++k)
    {
	long		cn;
	change::pointer cp2;

	if (!project_change_nth(pp, k, &cn))
    	    break;
	cp2 = change_alloc(pp, cn);
	change_bind_existing(cp2);
	// active only
	if (change_is_a_branch(cp2))
	{
	    project_ty *pp2 = pp->bind_branch(cp2);
	    checkout_modules_inner(modules, pp2);
	    project_free(pp2);
	}
	else
	{
	    if (cp2->is_being_developed())
	    {
		string_ty       *s2;

		s =
		    str_format
		    (
			"%s.C%3.3ld",
			project_name_get(pp)->str_text,
			cn
		    );
		desc = change_brief_description_get(cp2);
		desc_len = sanitary_length(desc, s->str_length);
		s2 =
		    str_format
		    (
			"%-15s %-15s # %.*s\n",
			s->str_text,
			s->str_text,
			desc_len,
			desc->str_text
		    );
		str_free(s);
		modules->push_back(s2);
		str_free(s2);
	    }
	    change_free(cp2);
	}
    }
    // do NOT free "lp"
    // do NOT free "cp"
}


void
module_cvsroot::checkout_modules(server_ty *sp)
{
    string_list_ty  toplevel;
    size_t          j;
    string_ty       *server_side;
    string_ty       *client_side;
    string_ty       *version;
    int             mode;
    string_ty       *s;

    //
    // The modules file is line oriented.  In its simplest form each line
    // contains the name of the module, whitespace, and the directory where
    // the module resides.  The directory is a path relative to $CVSROOT.
    // The last four lines in the example above are examples of such lines.
    //
    // The modules file records your definitions of names for collections
    // of source code.  cvs will use these definitions if you use cvs to
    // update the modules file (use normal commands like add, commit, etc).
    //
    // The modules file may contain blank lines and comments (lines beginning
    // with #) as well as module definitions.  Long lines can be continued
    // on the next line by specifying a backslash (\) as the last character
    // on the line.
    //
    // There are three basic types of modules: alias modules, regular
    // modules, and ampersand modules.  The difference between them is the
    // way that they map files in the repository to files in the working
    // directory.  In all of the following examples, the top-level repository
    // contains a directory called first-dir, which contains two files,
    // file1 and file2, and a directory sdir.  first-dir/sdir contains a
    // file sfile.
    //
    //
    // Alias modules
    //
    // Alias modules are the simplest kind of module:
    //
    // mname -a aliases
    //     This represents the simplest way of defining a module mname.
    //     The -a flags the definition as a simple alias: cvs will treat
    //     any use of mname (as a command argument) as if the list of names
    //     aliases had been specified instead.  aliases may contain either
    //     other module names or paths.  When you use paths in aliases,
    //     checkout creates all intermediate directories in the working
    //     directory, just as if the path had been specified explicitly in
    //     the cvs arguments.
    //
    // For example, if the modules file contains:
    //
    //     amodule -a first-dir
    //
    // then the following two commands are equivalent:
    //
    //     $ cvs co amodule
    //     $ cvs co first-dir
    //
    // and they each would provide output such as:
    //
    //     cvs checkout: Updating first-dir
    //     U first-dir/file1
    //     U first-dir/file2
    //     cvs checkout: Updating first-dir/sdir
    //     U first-dir/sdir/sfile
    //
    //
    // Regular modules
    //
    // mname [ options ] dir [ files ]
    //     In the simplest case, this form of module definition reduces
    //     to mname dir.  This defines all the files in directory dir
    //     as module mname.  dir is a relative path (from $CVSROOT) to a
    //     directory of source in the source repository.  In this case, on
    //     checkout, a single directory called mname is created as a working
    //     directory; no intermediate directory levels are used by default,
    //     even if dir was a path involving several directory levels.
    //
    // For example, if a module is defined by:
    //
    //     regmodule first-dir
    //
    // then regmodule will contain the files from first-dir:
    //
    //     $ cvs co regmodule
    //     cvs checkout: Updating regmodule
    //     U regmodule/file1
    //     U regmodule/file2
    //     cvs checkout: Updating regmodule/sdir
    //     U regmodule/sdir/sfile
    //     $
    //
    // By explicitly specifying files in the module definition after dir, you
    // can select particular files from directory dir.  Here is an example:
    //
    //     regfiles first-dir/sdir sfile
    //
    // With this definition, getting the regfiles module will create a
    // single working directory regfiles containing the file listed, which
    // comes from a directory deeper in the cvs source repository:
    //
    //     $ cvs co regfiles
    //     U regfiles/sfile
    //     $
    //
    //
    // Ampersand modules
    //
    // A module definition can refer to other modules by including &module
    // in its definition.
    //
    //     mname [ options ] &module
    //
    // Then getting the module creates a subdirectory for each such module,
    // in the directory containing the module.  For example, if modules
    // contains
    //
    //     ampermod &first-dir
    //
    // then a checkout will create an ampermod directory which contains
    // a directory called first-dir, which in turns contains all the
    // directories and files which live there.  For example, the command
    //
    //     $ cvs co ampermod
    //
    // will create the following files:
    //
    //     ampermod/first-dir/file1
    //     ampermod/first-dir/file2
    //     ampermod/first-dir/sdir/sfile
    //
    // There is one quirk/bug: the messages that cvs prints omit the
    // ampermod, and thus do not correctly display the location to which
    // it is checking out the files:
    //
    //     $ cvs co ampermod
    //     cvs checkout: Updating first-dir
    //     U first-dir/file1
    //     U first-dir/file2
    //     cvs checkout: Updating first-dir/sdir
    //     U first-dir/sdir/sfile
    //     $
    //
    // Do not rely on this buggy behavior; it may get fixed in a future
    // release of cvs.
    //
    //
    // Excluding directories
    //
    // An alias module may exclude particular directories from other modules
    // by using an exclamation mark (!)  before the name of each directory
    // to be excluded.
    //
    // For example, if the modules file contains:
    //
    //     exmodule -a !first-dir/sdir first-dir
    //
    // then checking out the module exmodule will check out everything in
    // first-dir except any files in the subdirectory first-dir/sdir.
    //
    // Note that the "!first-dir/sdir" sometimes must be listed before
    // "first-dir".  That seems like a probable bug, in which case perhaps
    // it should be fixed (to allow either order) rather than documented.
    // See modules4 in testsuite.
    //
    //
    // Module options
    //
    // Either regular modules or ampersand modules can contain options,
    // which supply additional information concerning the module.
    //
    // -d name
    //     Name the working directory something other than the module name.
    //
    // -e prog
    //     Specify a program prog to run whenever files in a module are
    //     exported.  prog runs with a single argument, the module name.
    //
    // -o prog
    //     Specify a program prog to run whenever files in a module are
    //     checked out.  prog runs with a single argument, the module name.
    //     See Module program options for information on how prog is called.
    //
    // -s status
    //     Assign a status to the module.  When the module file is printed
    //     with cvs checkout -s the modules are sorted according to primarily
    //     module status, and secondarily according to the module name.
    //     This option has no other meaning.  You can use this option for
    //     several things besides status: for instance, list the person
    //     that is responsible for this module.
    //
    // -t prog
    //     Specify a program prog to run whenever files in a module are
    //     tagged with rtag.  prog runs with two arguments: the module name
    //     and the symbolic tag specified to rtag.  It is not run when tag
    //     is executed.  Generally you will find that taginfo is a better
    //     solution (user-defined logging).
    //
    // You should also see Module program options about how the "program
    // options" programs are run.
    //
    //
    // Module program options
    //
    // For checkout, rtag, and export, the program is server-based, and as
    // such the following applies:-
    //
    // If using remote access methods (pserver, ext, etc.), cvs will execute
    // this program on the server from a temporary directory. The path is
    // searched for this program.
    //
    // If using "local access" (on a local or remote NFS file system, i.e.
    // repository set just to a path), the program will be executed from
    // the newly checked-out tree, if found there, or alternatively searched
    // for in the path if not.
    //
    // The programs are all run after the operation has effectively
    // completed.
    //

    //
    // Put the CVSROOT module in the list.
    //
    string_list_ty modules;
    s =
	str_from_c
	(
	    "#\n"
	    "# This file is generated.  You can not commit it.\n"
	    "# You must perform Aegis administration using Aegis commands.\n"
	    "#\n"
	    "CVSROOT         CVSROOT\n"
	);
    modules.push_back(s);
    str_free(s);

    //
    // Walk the project tree looking for active branches and
    // being-developed change sets.
    //
    gonzo_project_list(&toplevel);
    for (j = 0; j < toplevel.nstrings; ++j)
    {
	string_ty	*prjname;
	project_ty	*pp;
	int		err;

	prjname = toplevel.string[j];
	pp = project_alloc(prjname);
	pp->bind_existing();

	//
	// watch out for permissions
	// (returns errno of attempt to read project state)
	//
	err = project_is_readable(pp);

	//
	// Recurse into readable branch trees.
	//
	if (!err)
	    checkout_modules_inner(&modules, pp);
	else
	    modules.push_back(project_name_get(pp));
	project_free(pp);
    }
    toplevel.clear();

    //
    // Also extract the project aliases.
    //
    gonzo_alias_list(&toplevel);
    for (j = 0; j < toplevel.nstrings; ++j)
    {
	string_ty       *alias_name;
	string_ty       *other;

	alias_name = toplevel.string[j];
	other = gonzo_alias_to_actual(alias_name);
	assert(other);
	if (!other)
	    continue;
	s = str_format("%-12s -a %s\n", alias_name->str_text, other->str_text);
	modules.push_back(s);
	str_free(s);
    }

    //
    // sort the list of names
    // (C locale)
    //
    // Project names look a lot like versions strings (indeed,
    // the tail ends *are* version strings) so sort them as such.
    //
    modules.sort_version();

    //
    // Now build a string based on the list of module names.
    //
    s = modules.unsplit("");

    //
    // Now queue it all to be sent to the client.
    //
    server_side = str_from_c("CVSROOT/modules");
    client_side = server_directory_calc_client_side(sp, server_side);
    input ip = new input_string(nstring(s));
    str_free(s);
    mode = 0444;
    version = fake_version_now();
    server_mkdir_above(sp, client_side, server_side);
    server_updating_verbose(sp, client_side);
    server_response_queue
    (
	sp,
	new response_created(client_side, server_side, ip, mode, version)
    );
    str_free(version);
    str_free(client_side);
    str_free(server_side);
}


bool
module_cvsroot::update(server_ty *sp, string_ty *, string_ty *, const options &)
{
    //
    // FIXME: the client_side and serve_side COULD be refering to
    // individual files, not just the whole directory.
    //

    // checkoutlist
    // config
    // cvsignore
    // cvswrappers
    // history
    // loginfo
    // modules
    checkout_modules(sp);
    // rcsinfo
    // verifymsg

    //
    // Report success.
    //
    server_ok(sp);
    return true;
}


void
module_cvsroot::groan(server_ty *sp, const char *request_name)
{
    server_error
    (
	sp,
	"%s: You can not administer Aegis from this interface, "
	    "you must use Aegis commands directly.",
	request_name
    );
}


bool
module_cvsroot::checkin(server_ty *sp, string_ty *, string_ty *)
{
    groan(sp, "ci");
    return false;
}


bool
module_cvsroot::add(server_ty *sp, string_ty *, string_ty *, const options &)
{
    groan(sp, "add");
    return false;
}


bool
module_cvsroot::remove(server_ty *sp, string_ty *, string_ty *, const options &)
{
    groan(sp, "remove");
    return false;
}
