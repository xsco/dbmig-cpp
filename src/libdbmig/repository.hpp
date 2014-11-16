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

#ifndef DBMIG_REPOSITORY_INCLUDED
#define DBMIG_REPOSITORY_INCLUDED

#include <string>
#include <memory>
#include <utility>
#include <iterator>
#include "script_action.hpp"
#include "semantic_version.hpp"
#include "script_dir.hpp"

namespace dbmig
{
    ///
    /// Represents a repository of database change scripts on disk
    ///
    class repository
    {
    public:
    
        explicit repository(const std::string &path);
        repository(
            const std::string &latest_schema_path,
            const std::string &upgrade_script_path,
            const std::string &install_script_path);
        ~repository();

        ///
        /// The path where latest-version DDL scripts will be searched for.
        ///
        const std::string &latest_schema_path() const;
        
        ///
        /// The path where upgrade/rollback scripts will be search for.
        ///
        const std::string &upgrade_script_path() const;
        
        ///
        /// The path where install scripts will be search for.
        ///
        const std::string &install_script_path() const;

        ///
        /// The latest version available in the repository.
        ///
        /// Returns the latest version in the repository that scripts are
        /// available for.  If the repository is empty, the return value is
        /// semver::zero().
        ///
        semver latest_version() const;
        
        ///
        /// Return iterator range over the (single) nearest install script
        ///
        /// If the repository contains no suitable install script, the range
        /// will be empty.
        ///
        script_dir::const_iterator_range nearest_install_script(
            const semver &target) const;
        
        ///
        /// Return iterator range over upgrade actions
        ///
        /// Given a starting version, and a target version that is later than
        /// the starting version, this method will provide an iterator range
        /// over upgrade_actions that will take an installation to the target
        /// version from the starting version.
        ///
        script_dir::const_iterator_range upgrade_scripts(
            const semver &start, const semver &target) const;
        
        ///
        /// Range over the (single) upgrade script matching a given version
        ///
        script_dir::const_iterator_range upgrade_script_at(
            const semver &ver) const;
        
    private:
    
        struct impl;
        std::unique_ptr<impl> pimpl_;
    };
    
    ///
    /// Convenience method to calculate the SHA256 hash of a given script
    ///
    std::string calculate_script_hash(const repository &repo,
                                      const script_action &action,
                                      const std::string &script_path);
}

#endif // DBMIG_REPOSITORY_INCLUDED

