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

#include <nowide/iostream.hpp>
#include <string>
#include <check.hpp>

///
/// Check the compatibility of a repository with a given database
///
void check(
    const std::string &conn_str,
    const std::string &changeset,
    const bool verbose,
    const std::string &repository_path)
{
    using std::string;
    using nowide::cout;
    using nowide::cerr;
    using std::endl;

    // Perform the check.
    auto report = dbmig::perform_check(conn_str, changeset, repository_path);
    auto num_issues = report.size();

    // Print results as needed.
    if (verbose || num_issues > 1) {
        if (num_issues == 1)
            cout << "Found 1 issue" << endl;
        else
            cout << "Found " << num_issues << " issues" << endl;
    }
    
    // Print any issues.
    int i = 0;
    for (auto &issue : report) {
        ++i;
        switch (issue.type) {
            case dbmig::check_report_issue_type::missing_from_repository:
                cout << i << ". Version " << issue.version
                    << " is missing from the repository:" << endl;
                cout << "* Changelog script:  "
                    << dbmig::to_string(issue.changelog_action) << ":"
                    << issue.changelog_script_path << endl;
                cout << "* Changelog hash:    "
                    << issue.changelog_script_hash << endl;
                break;
            case dbmig::check_report_issue_type::missing_from_changelog:
                cout << i << ". Version " << issue.version
                    << " is missing from the changelog:" << endl;
                cout << "* Repository script: "
                    << dbmig::to_string(issue.repository_script_action) << ":"
                    << issue.repository_script_path << endl;
                cout << "* Repository hash:   "
                    << issue.repository_script_hash << endl;
                break;
            case dbmig::check_report_issue_type::hash_mismatch:
                cout << i << ". Version " << issue.version
                    << " is different between the repository and changelog:"
                    << endl;
                cout << "* Changelog script:  "
                    << dbmig::to_string(issue.changelog_action) << ":"
                    << issue.changelog_script_path << endl;
                cout << "* Repository script: "
                    << dbmig::to_string(issue.repository_script_action) << ":"
                    << issue.repository_script_path << endl;
                cout << "* Changelog hash:    "
                    << issue.changelog_script_hash << endl;
                cout << "* Repository hash:   "
                    << issue.repository_script_hash << endl;
                break;
            default:
                cerr << "Unknown report issue found: "
                    << dbmig::check_report_issue_type_to_str(issue.type)
                    << endl;
        }
    }
}

