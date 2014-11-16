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

#include "migrate.hpp"

#include <fstream>
#include <iostream>
#include <soci/soci.h>
#include "script_stream.hpp"
#include "script_action.hpp"
#include "changelog_table.hpp"
#include "time.hpp"
#include "exception.hpp"

namespace dbmig {

///
/// Run a single install script against a target database
///
/// Returns the new resultant version of the target database.
/// The act of running the install script and modifying the changelog will
/// take place within a single transaction.
///
semver run_install_script(
        const std::string &conn_str,
        const std::string &changeset,
        const semver &script_version,
        const std::string &repo_install_path,
        const std::string &script_path)
{
//    auto start_time = time::now();
    soci::session s{conn_str};
    // Start transaction
    soci::transaction txn{s};
    
    // Run the script.
    std::ifstream ifs{repo_install_path + "/" + script_path};
    install_statements statements{ifs};
    for (auto statement : statements) {
        s << statement;
    }
    
    // Update the changelog.
    auto end_time = time::now();
    double seconds = 0.0; // TODO
    changelog_table cl{s, changeset};
    cl.write(end_time,
            script_path,
            script_version,
            statements.sha256_sum(),
            seconds);
    
    // Commit transaction
    txn.commit();
    return script_version;
}

///
/// Run a single upgrade script against a target database
///
/// Returns the new resultant version of the target database.
/// The act of running the upgrade script and modifying the changelog will
/// take place within a single transaction.
///
semver run_upgrade_script(
        const std::string &conn_str,
        const std::string &changeset,
        const semver &script_version,
        const std::string &repo_upgrade_path,
        const std::string &script_path)
{
//    auto start_time = time::now();
    soci::session s{conn_str};
    // Start transaction
    soci::transaction txn{s};
    
    // Get the existing version from the changelog.
    changelog_table cl{s, changeset};
    auto existing_ver = cl.version();
    
    // Run the script.
    std::ifstream ifs{repo_upgrade_path + "/" + script_path};
    upgrade_statements statements{ifs};
    for (auto statement : statements) {
        s << statement;
    }
    
    // Update the changelog.
    auto end_time = time::now();
    double seconds = 0.0; // TODO
    cl.write(end_time,
            script_path,
            script_action::upgrade,
            existing_ver,
            script_version,
            statements.sha256_sum(),
            seconds);
    
    // Commit transaction
    txn.commit();
    return script_version;
}

static semver internal_run_rollback_script(
        const std::string &conn_str,
        const std::string &changeset,
        const semver &rollback_to_version,
        const std::string &repo_upgrade_path,
        const std::string &script_path,
        const std::string &alleged_sha256_sum)
{
//    auto start_time = time::now();
    soci::session s{conn_str};
    // Start transaction
    soci::transaction txn{s};
    
    // Get the existing version from the changelog.
    changelog_table cl{s, changeset};
    auto existing_ver = cl.version();
    
    // Read statements (and hash) from the file.
    std::ifstream ifs{repo_upgrade_path + "/" + script_path};
    rollback_statements statements{ifs};
    
    // Note: blank hash passed in means skip the checksum check.
    if (alleged_sha256_sum != "" &&
        alleged_sha256_sum != statements.sha256_sum()) {
        // Hashes don't match!
        throw script_changed_since_deployment{
            alleged_sha256_sum, statements.sha256_sum(), script_path};
    }
    
    // Run the script.
    for (auto statement : statements) {
        s << statement;
    }
    
    // Update the changelog.
    auto end_time = time::now();
    double seconds = 0.0; // TODO
    cl.write(end_time,
            script_path,
            script_action::rollback,
            existing_ver,
            rollback_to_version,
            statements.sha256_sum(),
            seconds);
    
    
    txn.commit();
    return rollback_to_version;
}

///
/// Run a single rollback script against the target database
///
/// Returns the new resultant version of the target database.
/// The act of running the rollback script and modifying the changelog will
/// take place within a single transaction.
///
/// The overloaded versions of this function allow an alleged SHA256 hash
/// of the script to be passed, intended to represent the hash when the
/// script was first deployed to the database.  The idea is that it would be
/// possibly dangerous to rollback a script that has actually changed since
/// it was first run into a target database.
///
semver run_rollback_script(
        const std::string &conn_str,
        const std::string &changeset,
        const semver &rollback_to_version,
        const std::string &repo_upgrade_path,
        const std::string &script_path)
{
    return internal_run_rollback_script(conn_str, changeset,
                                        rollback_to_version, repo_upgrade_path,
                                        script_path, "");
}
semver run_rollback_script(
        const std::string &conn_str,
        const std::string &changeset,
        const semver &rollback_to_version,
        const std::string &repo_upgrade_path,
        const std::string &script_path,
        const std::string &alleged_sha256_sum)
{
    return internal_run_rollback_script(conn_str, changeset,
                                        rollback_to_version, repo_upgrade_path,
                                        script_path, alleged_sha256_sum);
}


} // dbmig namespace

