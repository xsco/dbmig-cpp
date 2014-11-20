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

#include <iostream>
#include <string>
#include <stdexcept>
#include <repository.hpp>
#include <changelog.hpp>
#include <migrate.hpp>

#include "console_util.hpp"


using namespace std;

///
/// Install a baseline version into the target environment
///
static dbmig::semver install_baseline(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose,
    const bool force,
    dbmig::repository &repo,
    const dbmig::semver &target_version)
{
    // There is no version of anything currently installed in the database.
    // Look through the repository for the nearest install script to our
    // desired target version.
    if (verbose) {
        std::cout << "No version installed; looking for install scripts"
                  << std::endl;
    }
    
    auto install_scripts = repo.nearest_install_script(target_version);
    if (install_scripts.first == install_scripts.second) {
        // The repository doesn't contain any install scripts!  This is a
        // fatal problem, since we must install some initial version in
        // order to have any kind of target version installed.
        throw range_error{"No suitable install script earlier than " +
                          target_version.to_str() + " in repo"};
    }
    
    auto &install_script = install_scripts.first;
    if (verbose) {
        std::cout << "INSTALL to " << install_script->first
                  << ", script " << install_script->second
                  << std::endl;
    }

    if (!force) {
        std::string msg = "Run install script " + install_script->second + "?";
        if (!console_confirmation(msg.c_str()))
            throw user_driven_cancel{};
    }
    
    // Run the install script and write to changelog (in one txn!)
    dbmig::run_install_script(conn_str, changeset, install_script->first,
            repo.install_script_path(), install_script->second);
    
    // Return the new version of the baseline installation.
    return install_script->first;
}

static dbmig::semver upgrade(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose,
    const bool force,
    dbmig::repository &repo,
    const dbmig::semver &from_version,
    const dbmig::semver &target_version)
{
    if (verbose) {
        std::cout << "Finding upgrade scripts to go from "
            << from_version << " to " << target_version << std::endl;
    }
    
    auto uscripts = repo.upgrade_scripts(from_version, target_version);
    dbmig::semver current_version = from_version;
    for (auto s = uscripts.first; s != uscripts.second; ++s) {
        auto ver = s->first;
        auto path = s->second;

        if (verbose) {
            std::cout << "UPGRADE from " << current_version << " to "
                << ver << ", script " << path << std::endl;
        }
        
        if (!force) {
            std::string msg = "Run upgrade script " + path + "?";
            if (!console_confirmation(msg.c_str()))
                throw user_driven_cancel{};
        }
        
        // Run the script and write to changelog (in one txn!)
        dbmig::run_upgrade_script(conn_str, changeset, ver,
                repo.upgrade_script_path(), path);
        
        // Count up the version.
        current_version = ver;
    }
    
    return current_version;
}

static dbmig::semver rollback(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose,
    const bool force,
    dbmig::repository &repo,
    const dbmig::semver &start_from_version,
    const dbmig::semver &target_version)
{
    if (verbose) {
        std::cout << "Searching changelog history for path from "
            << start_from_version << " to " << target_version << std::endl;
    }

    // Try to obtain rollback steps to our desired target version.
    dbmig::changelog cl{conn_str, changeset};
    auto rollback_steps = cl.rollback_steps(target_version);
    if (verbose) {
        int n = rollback_steps.size();
        if (n == 1)
            std::cout << "Determined 1 rollback step" << std::endl;
        else if (n > 0)
            std::cout << "Determined " << n << " rollback steps" << std::endl;
    }
    
    // Find out if the target version is in the changelog history.
    if (rollback_steps.empty()) {
        // Nope, not possible.
        throw out_of_range{"No known path to rollback to target version " +
                           target_version.to_str() + " in changelog"};
    }
    // We expect the first rollback step to be rolling back the 'from' version.
    auto &first_step_from_ver = rollback_steps[0].from_version;
    if (first_step_from_ver != start_from_version) {
        // This is some kind of internal inconsistency error.  Eurgh!
        throw out_of_range{"The latest version in the changelog is " +
                           first_step_from_ver.to_str() + " but dbmig " +
                           "has got confused and thinks we're starting from " +
                           start_from_version.to_str()};
    }
    
    // Go through all the rollback steps.
    dbmig::semver current_version = start_from_version;
    for (auto step : rollback_steps) {
    
        auto &from_ver = step.from_version;
        auto &to_ver   = step.to_version;
        auto &cl_hash  = step.sha256_hash;
        
        if (verbose) {
            std::cout << "Looking for script to rollback from " << from_ver
                      << " to " << to_ver << "..." << std::endl;
        }
        
        // Find the relevant script.
        auto range = repo.upgrade_script_at(from_ver);
        if (range.first == range.second) {
            throw out_of_range{"We want to rollback the script with version " +
                               from_ver.to_str() + " but it cannot be " +
                               "found in the repository"};
        }
        
        // Roll back the script.
        auto &script_path = range.first->second;
        if (verbose) {
            std::cout << "ROLLBACK from " << from_ver << " to "
                << to_ver << ", script " << script_path << std::endl;
        }
        
        if (!force) {
            std::string msg = "Run rollback script " + script_path + "?";
            if (!console_confirmation(msg.c_str()))
                throw user_driven_cancel{};
        }
        current_version = dbmig::run_rollback_script(conn_str, changeset,
            to_ver, repo.upgrade_script_path(), script_path, cl_hash);
    }
    
    return current_version;
}

static void migrate(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose,
    const bool force,
    dbmig::repository &repo,
    const dbmig::semver &target_version)
{
    // Do we need to install an initial version of the database?
    dbmig::changelog cl{conn_str, changeset};
    auto current_version = cl.version();
    if (current_version.is_zero()) {
        // Install a baseline version.
        current_version = install_baseline(conn_str, changeset, verbose, force,
                                           repo, target_version);
    }
    else {
        std::cout << "Currently-installed version is " << current_version
            << std::endl;
    }
    
    // Are we going forward or backwards?
    if (target_version > current_version) {
        // Upgrade!
        current_version = upgrade(conn_str, changeset, verbose, force, repo,
                                  current_version, target_version);

        if (verbose) {
            std::cout << "Upgraded to version " << current_version << std::endl;
        }
    }
    else if (target_version < current_version) {
        // Rollback!
        current_version = rollback(conn_str, changeset, verbose, force, repo,
                                   current_version, target_version);
        
        if (verbose) {
            std::cout << "Rolled back to version " << current_version
                      << std::endl;
        }
    }
    else {
        // No further work needed, as we're already at the target version
        if (verbose) {
            std::cout << "Already at target version.  No further upgrade "
                         "scripts need to be run" << std::endl;
        }
    }
    
    // Check we reached the version.
    if (current_version != target_version) {
        std::cerr << "Warning: migrated to version " << current_version
                  << ", rather than requested version " << target_version
                  << std::endl;
    }
}

///
/// Migrate a database to the latest version
///
void migrate(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose,
    const bool force,
    const std::string &repository_path)
{
    dbmig::repository repo{repository_path};
    
    // Find the latest version in the repository.
    auto target_version = repo.latest_version();
    if (verbose) {
        std::cout << "Will attempt to migrate to version " << target_version
            << std::endl;
    }
    
    migrate(conn_str, changeset, verbose, force, repo, target_version);
}

///
/// Migrate a database to a given version
///
void migrate(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose,
    const bool force,
    const std::string &repository_path,
    const std::string &version_str)
{
    dbmig::repository repo{repository_path};
    
    // Parse the version.
    auto target_version = dbmig::semver::parse(version_str);
    if (verbose) {
        std::cout << "Will attempt to migrate to latest version "
            << target_version << std::endl;
    }
    
    migrate(conn_str, changeset, verbose, force, repo, target_version);
}

