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

#ifndef DBMIG_CHANGELOG_TABLE_INCLUDED
#define DBMIG_CHANGELOG_TABLE_INCLUDED

#include <string>
#include <memory>
#include <chrono>
#include <vector>
#include <tuple>
#include <soci/soci.h>

#include "changelog_entry.hpp"
#include "script_action.hpp"
#include "semantic_version.hpp"

namespace dbmig
{
    ///
    /// Represents the physical changelog table within a database
    ///
    class changelog_table
    {
    public:
        changelog_table(soci::session &session, const std::string &changeset);
        ~changelog_table();

        ///
        /// Is a changelog table installed on the database?
        ///
        const bool installed() const;

        ///
        /// Get the currently-installed version of the database.
        ///
        const semver version() const;
        
        ///
        /// Get the most recent previously-installed version of the database.
        ///
        const semver previous_version() const;
        
        ///
        /// Get the version that the database could be atomically rolled back to
        ///
        const semver rollback_version() const;
        
        ///
        /// Get a list of steps to take to roll back to a given version
        ///
        const rollback_step_list rollback_steps(const semver &ver) const;
        
        ///
        /// Get a list of the last batch of contiguous changelog entries
        ///
        const changelog_entry_list
        contiguous_history(bool exclude_rolled_back) const;
        
        ///
        /// Force the changelog to a certain version.
        ///
        void override_version(const semver &ver);
        
        ///
        /// Write a new entry to the changelog.
        ///
        void write(
            const std::time_t &applied,
            const std::string &script_path,
            const semver &install_version,
            const std::string &sha256_hash,
            const double seconds);
        void write(
            const std::time_t &applied,
            const std::string &script_path,
            const script_action &action,
            const semver &from_version,
            const semver &to_version,
            const std::string &sha256_hash,
            const double seconds);

    private:
    
        struct impl;
        std::unique_ptr<impl> pimpl_;
    };
}

#endif // DBMIG_CHANGELOG_TABLE_INCLUDED

