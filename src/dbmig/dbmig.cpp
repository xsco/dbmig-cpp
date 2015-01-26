/*
    dbmig - Database schema migration tool
    Copyright (C) 2012-2014  Adam Szmigin (adam.szmigin@xsco.net)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <config.h>

#include <boost/program_options.hpp>
#include <nowide/iostream.hpp>

#include "services.hpp"
#include "console_util.hpp"


namespace po = boost::program_options;
using std::string;
using nowide::cout;
using nowide::cerr;
using std::endl;

static void print_usage(std::ostream &os, const po::options_description &od)
{
    os << "Usage:" << endl;
    os << PACKAGE " command [-t database] [options]" << endl << endl;
    os << "Supported commands:" << endl;
    os << "  help                - "
       << "print this help message" << endl;
    os << "  print-version       - "
       << "print version and exit" << endl;
    os << "  show                - "
       << "print the currently-installed version of a database" << endl;
    os << "  check               - "
       << "check repository compatibility against a database" << endl;
    os << "  migrate             - "
       << "migrate a database to a new version" << endl;
    os << "  purge               - "
       << "permanently delete the whole of a database" << endl;
    os << "  create-unversioned  - "
       << "create an unversioned database from raw scripts" << endl;
    os << "  override-version    - "
       << "forcibly override the version of an existing database" << endl;
    os << od << endl;
}

static void check_target(const po::variables_map &vm)
{
    if (!vm.count("target"))
        throw std::domain_error(
            "a target database connection string must be provided ("
            "see the --target option)");
}

int main(int argc, char *argv[])
{
    // Hidden options (the positional "command" parameter).
    po::options_description od_hidden("Hidden options");
    od_hidden.add_options()
        ("command", po::value<string>(), "command to execute")
        ;
    // Options that make sense across any command.
    po::options_description od_generic("Generic options");
    od_generic.add_options()
        ("target,t", po::value<string>(),
            "target database connection string")
        ("changeset", po::value<string>()->default_value("default"),
            "name of changeset within target database")
        ("force,f", po::bool_switch(),
            "do not prompt for confirmation for any operation which "
            "modifies the database")
        ("verbose,v", po::bool_switch(),
            "print additional messages about what's going on")
        ;
    
    // The 'command' option is the positional parameter representing what we
    // want dbmig to do in any given circumstance.
    po::positional_options_description pos;
    pos.add("command", 1)
       .add("subargs", -1);
    
    // Parsed command-line options.
    po::options_description od_cmdline;
    od_cmdline.add(od_hidden).add(od_generic);
    // Visible command-line options.
    po::options_description od_visible;
    od_visible.add(od_generic);
    
    
    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc, argv)
        .options(od_cmdline)
        .positional(pos)
        .allow_unregistered() // For sub-command params
        .run();
    po::store(parsed, vm);
    po::notify(vm);
    
    if (vm.count("command") != 1 || vm["command"].as<string>() == "help")
    {
        print_usage(cout, od_visible);
        return 1;
    }
    
    bool verbose = vm["verbose"].as<bool>();
    string cmd = vm["command"].as<string>();
    bool force = vm["force"].as<bool>();
    
    try
    {
        if (cmd == "print-version")
        {
            cout << PACKAGE_STRING << endl;
        }
        else if (cmd == "show")
        {
            check_target(vm);
            show(
                vm["target"].as<string>(),
                vm["changeset"].as<string>(),
                verbose);
        }
        else if (cmd == "check")
        {
            // check has some specific options
            po::options_description ck_desc("check options");
            ck_desc.add_options()
                ("repo-dir", po::value<string>()->default_value("."),
                 "path to repository");
        
            // Any unrecognised options from the first pass are assumed to
            // belong to this sub-command.
            std::vector<string> opts = po::collect_unrecognized(
                parsed.options, po::include_positional);
            opts.erase(opts.begin()); // Remove the command itself.

            // Parse again...
            po::store(po::command_line_parser(opts).options(ck_desc).run(), vm);
            
            check_target(vm);
            check(
                vm["target"].as<string>(),
                vm["changeset"].as<string>(),
                verbose,
                vm["repo-dir"].as<string>());
        }
        else if (cmd == "override-version")
        {
            // override-version has some specific options
            po::options_description ov_desc("override-version options");
            ov_desc.add_options()
                ("version", po::value<string>(),
                 "target version to override to");
        
            // Any unrecognised options from the first pass are assumed to
            // belong to this sub-command.
            std::vector<string> opts = po::collect_unrecognized(
                parsed.options, po::include_positional);
            opts.erase(opts.begin()); // Remove the command itself.

            // Parse again...
            po::store(po::command_line_parser(opts).options(ov_desc).run(), vm);
            
            // TODO - print help pages for sub-commands
            if (!vm.count("version")) {
                throw std::domain_error(
                    "a target version must be provided for override-version "
                    "(see the --version option)");
            }

            check_target(vm);
            override_version(
                vm["target"].as<string>(),
                vm["changeset"].as<string>(),
                verbose, force,
                vm["version"].as<string>());
        }
        else if (cmd == "migrate")
        {
            // migrate has some specific options
            po::options_description mg_desc("override-version options");
            mg_desc.add_options()
                ("version", po::value<string>(),
                 "target version to migrate to")
                ("repo-dir", po::value<string>()->default_value("."),
                 "path to repository");
        
            // Any unrecognised options from the first pass are assumed to
            // belong to this sub-command.
            std::vector<string> opts = po::collect_unrecognized(
                parsed.options, po::include_positional);
            opts.erase(opts.begin()); // Remove the command itself.

            // Parse again...
            po::store(po::command_line_parser(opts).options(mg_desc).run(), vm);
            
            check_target(vm);
            if (!vm.count("version")) {
                // Migrate to latest version.
                migrate(
                    vm["target"].as<string>(),
                    vm["changeset"].as<string>(),
                    verbose, force,
                    vm["repo-dir"].as<string>());
            }
            else {
                // Migrate to specific version.
                migrate(
                    vm["target"].as<string>(),
                    vm["changeset"].as<string>(),
                    verbose, force,
                    vm["repo-dir"].as<string>(),
                    vm["version"].as<string>());
            }
        }
        else if (
            cmd == "purge" ||
            cmd == "create-unversioned")
        {
            cout << "Command: " << cmd << endl;
            // TODO - dispatch here
        }
        else if (cmd == "help")
        {
            print_usage(cout, od_visible);
        }
        else
        {
            cerr << "Unrecognised command \"" << cmd << "\"" << endl;
            print_usage(cerr, od_visible);
            return 1;
        }
    }
    catch (const std::exception &ex)
    {
        cerr << "error: " << ex.what() << endl;
        return 1;
    }
    
    return 0;
}

