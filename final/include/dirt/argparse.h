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

#pragma once

#include <string>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4706)
#endif
#include <json.hh>
#if defined(_MSC_VER)
#pragma warning(pop) 
#endif

using json = nlohmann::json;
using std::string;
using std::vector;

/// Command-line argument parser
struct CommandlineParser
{
    /// Description of command line argument
    struct Argument
    {
        string name;                    ///< complete name
        string flag;                    ///< flag string name
        string desc;                    ///< description
        const std::type_info & type;    ///< supported: int, double, bool (flag), string
        bool opt;                       ///< whether it is an optional argument
        json def;                       ///< default value
    };

    // parameters
    string progname;                ///< program name
    vector<Argument>    options;    ///< program options (flags)
    vector<Argument>    arguments;  ///< program arguments

    CommandlineParser(const string &progname, const vector<Argument>&options,
    	        const vector<Argument>&arguments) :
        progname(progname), options(options), arguments(arguments)
    {
    }

    /// parse command line arguments to json
    json parse(const vector<string> & args) const;
    /// parse command line arguments to json
    json parse(int argc, char** argv) const;

    void printUsageAndQuit(const std::string & msg = {}) const;
};