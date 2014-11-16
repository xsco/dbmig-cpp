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

#ifndef DBMIG_MIGRATE_INCLUDED
#define DBMIG_MIGRATE_INCLUDED

#include <string>
#include "semantic_version.hpp"

namespace dbmig
{
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
            const std::string &script_path);

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
            const std::string &script_path);

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
            const std::string &script_path);
    semver run_rollback_script(
            const std::string &conn_str,
            const std::string &changeset,
            const semver &rollback_to_version,
            const std::string &repo_upgrade_path,
            const std::string &script_path,
            const std::string &alleged_sha256_sum);
}

#endif // DBMIG_MIGRATE_INCLUDED

