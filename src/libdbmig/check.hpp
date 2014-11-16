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

#ifndef DBMIG_CHECK_INCLUDED
#define DBMIG_CHECK_INCLUDED

#include <string>
#include <vector>
#include "script_action.hpp"
#include "semantic_version.hpp"

namespace dbmig
{
    enum class check_report_issue_type
    {
        ///
        /// An entry in the changelog does not have any corresponding script
        /// associated with it that can be found in the repository
        ///
        missing_from_repository,
        
        ///
        /// A script in the repository is versioned within the range found in
        /// the changelog, but there is no entry for it
        ///
        missing_from_changelog,
        
        ///
        /// A script appears in both the repository and the changelog, but the
        /// hash does not match, implying that the script has changed since
        /// deployment, or a logically different script was deployed
        ///
        hash_mismatch
    };

    struct check_report_issue
    {
        semver version;
        check_report_issue_type type;
        script_action repository_script_action;
        std::string repository_script_path;
        std::string repository_script_hash;
        script_action changelog_action;
        std::string changelog_script_path;
        std::string changelog_script_hash;
    };
    
    typedef std::vector<check_report_issue> check_report;
    
    ///
    /// Obtain a string equivalent for a report issue type
    ///
    const char *
    check_report_issue_type_to_str(const check_report_issue_type type);

    ///
    /// Check the compatibility of a repository with a given database
    ///
    const check_report
    perform_check(
            const std::string &conn_str,
            const std::string &changeset,
            const std::string &repository_path);

}

#endif // DBMIG_CHECK_INCLUDED

