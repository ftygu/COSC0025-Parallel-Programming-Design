/*
    This file is part of Dirt, the Dartmouth introductory ray tracer.

    Copyright (c) 2017-2019 by Wojciech Jarosz

    Dirt is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Dirt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <dirt/common.h>
#include <dirt/argparse.h>
#include <fstream>
#include <iomanip>

// local functions
namespace
{

// parse value
json cmdlineParseValue(const string & str, const CommandlineParser::Argument & opt, const CommandlineParser & cmd)
{
    try
    {
        if (typeid(int) == opt.type)
        {
            return json(std::stoi(str));
        }
        else if (typeid(float) == opt.type)
        {
            return json(std::stod(str));
        }
        else if (typeid(double) == opt.type)
        {
            return json(std::stod(str));
        }
        else if (typeid(string) == opt.type)
        {
            return json(str);
        }
        else
            error("unknown type");
    }
    catch (const std::exception &)
    {
        cmd.printUsageAndQuit(tfm::format("invalid value for %s", opt.name));
    }
    return json();
}

string typeString(const CommandlineParser::Argument & opt)
{
	if (typeid(int) == opt.type)
		return "int";
	else if (typeid(float) == opt.type ||
			 typeid(double) == opt.type)
		return "real";
	else if (typeid(string) == opt.type)
		return "string";
	else if (typeid(bool) == opt.type)
		return "";

	error("unknown type");
	return "unknown";
}

} // namespace


// print usage information
void CommandlineParser::printUsageAndQuit(const string & msg) const
{
    if (!msg.empty())
        error("%s\n\n", msg);

    int maxWidth = 25;

    cerr << rang::style::bold << "usage: " << rang::style::reset << progname;
    cerr << rang::fgB::cyan;
    for (auto opt : options)
    {
        cerr << " ";
        auto optname = (opt.flag == "") ? "--"+opt.name : "-"+opt.flag;
        auto optval = (opt.type != typeid(bool)) ? " <"+opt.name+">" : "";
        if (opt.opt)
            cerr << "[" << optname << optval << "]";
        else
        	cerr << optname << optval;
    }
    cerr << rang::fgB::green;
    for (auto arg : arguments)
    {
        cerr << " ";
        if (arg.opt)
            cerr << "[" << arg.name << "]";
        else
            cerr << "<" << arg.name << ">";
    }
    cerr << rang::fg::reset << endl << endl;

    if (!options.empty() || !arguments.empty())
    {
        cerr << rang::style::bold << "options:\n" << rang::style::reset;
        for (auto opt : options)
        {
            string optline = string("  ")
                           + ((opt.flag == "") ? "" : "-" + opt.flag + "/")
                           + "--" + opt.name
                           + ((opt.type != typeid(bool))
                                ? " <" + typeString(opt) + ">:"
                                : string(":"));
            cerr << rang::fgB::cyan
                 << std::left << std::setw(maxWidth) << optline
                 << rang::fg::reset;
            cerr << rang::style::italic
                 << opt.desc << "\n"
                 << rang::style::reset;
        }

	    cerr << rang::style::bold << "\narguments:\n" << rang::style::reset;
        for (auto arg : arguments)
        {
            string argline = string("  <") + arg.name + ">:";
            cerr << rang::fgB::green
                 << std::left << std::setw(maxWidth) << argline
                 << rang::fg::reset;
            cerr << rang::style::italic
                 << arg.desc << "\n"
                 << rang::style::reset;
        }
        cerr << "\n\n";
    }

	cerr << flush;

    exit(msg.empty() ? EXIT_SUCCESS : EXIT_FAILURE);
}


// parsing values
json CommandlineParser::parse(const vector<string> & args) const
{
    auto parsed = json::object_t();
    auto largs = args; // make a copy to change
    set<string> visited;
    for (auto opt : options)
    {
        auto pos = -1;
        for (auto i : range(int(largs.size())))
        {
            if (largs[i] == "--"+opt.name || largs[i] == "-"+opt.flag)
            {
                pos = i; break;
            }
        }
        if (pos < 0)
        {
            if (opt.opt)
                parsed[opt.name] = opt.def;
            else
                printUsageAndQuit(tfm::format("required option -%s", opt.flag));
        }
        else
        {
            if (opt.type != typeid(bool))
            {
                if (pos == int(largs.size())-1)
                    printUsageAndQuit(tfm::format("no value for argument %s", opt.name));
                auto sval = largs[pos+1];
                largs.erase(largs.begin()+pos,largs.begin()+pos+2);
                parsed[opt.name] = cmdlineParseValue(sval, opt, *this);
            }
            else
            {
                parsed[opt.name] = true;
                largs.erase(largs.begin()+pos);
            }
        }
    }
    for (auto arg : largs)
    {
        if (arg[0] == '-')
            printUsageAndQuit(tfm::format("unknown option %s", arg));
    }
    for (auto arg : arguments)
    {
        if (largs.empty())
        {
            if (arg.opt)
                parsed[arg.name] = arg.def;
            else
                printUsageAndQuit(tfm::format("missing required argument %s", arg.name));
        }
        else
        {
            parsed[arg.name] = cmdlineParseValue(largs[0], arg, *this);
            largs.erase(largs.begin());
        }
    }
    if (not largs.empty())
        printUsageAndQuit("too many arguments");
    return json(parsed);
}

// parsing values
json CommandlineParser::parse(int argc, char** argv) const
{
    auto args = vector<string>();
    for (auto i : range(1, argc))
        args.push_back(argv[i]);
    return parse(args);
}
