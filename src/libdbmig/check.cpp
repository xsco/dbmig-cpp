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

#include "check.hpp"

#include "changelog.hpp"
#include "repository.hpp"
#include "script_stream.hpp"
#include "diff.hpp"

namespace dbmig {

///
/// Obtain a string equivalent for a report issue type
///
const char *
check_report_issue_type_to_str(const check_report_issue_type type)
{
    switch (type) {
        case check_report_issue_type::missing_from_repository:
            return "missing from repository";
        case check_report_issue_type::missing_from_changelog:
            return "missing from changelog";
        case check_report_issue_type::hash_mismatch:
            return "hash mismatch";
        default:
            return "unknown";
    }
}

///
/// Struct representing info about a script.
///
struct script_info
{
    script_action action;
    semver version;
    std::string path;
};

struct changelog_entry_script_info_cmp
{
    bool operator() (const changelog_entry &cle, const script_info &si) {
        return cle.to_version < si.version;
    }
};
struct changelog_entry_script_info_eq
{
    bool operator() (const changelog_entry &cle, const script_info &si) {
        return cle.to_version == si.version;
    }
};

///
/// Check the compatibility of a repository with a given database
///
const check_report
perform_check(
    const std::string &conn_str,
    const std::string &changeset,
    const std::string &repository_path)
{
    changelog cl{conn_str, changeset};
    repository repo{repository_path};
    
    // Start by getting a contiguous history of events in the changelog back
    // to when the database was last non-incrementally changed.
    auto cl_entries = cl.contiguous_history(true);
    auto cl_latest = cl.version();
    if (cl_entries.empty() || cl_latest.is_zero()) {
        return check_report{};
    }
    auto &first_version = cl_entries[0].to_version;
    
    // Get contiguous scripts from the repository from the earliest point in
    // the changelog history.
    typedef std::vector<script_info> script_list;
    script_list scripts;
    // Start with looking for an install script.
    auto install_script_range = repo.nearest_install_script(first_version);
    if (install_script_range.first != install_script_range.second) {
        // We have an install script.
        scripts.push_back({script_action::install,
                           install_script_range.first->first,
                           install_script_range.first->second});
    }
    // Look for upgrade scripts.
    auto upgrade_script_search_from = scripts.empty()
        ? cl_entries[0].to_version
        : scripts[0].version;
    auto upgrade_script_range = repo.upgrade_scripts(
        upgrade_script_search_from, cl_latest);
    for (auto &us : upgrade_script_range) {
        scripts.push_back({script_action::upgrade, us.first, us.second});
    }
    
    check_report report;
    
    // The changelog events and repository scripts are in ascending order of
    // version already.  Apply the diff algorithm.
    changelog_entry_script_info_cmp cmp;
    changelog_entry_script_info_eq  eq;
    diff(
        std::begin(cl_entries), std::end(cl_entries),
        std::begin(scripts),    std::end(scripts),
        [&report](changelog_entry &cle)
        {
            // The changelog has something not in the repository.
            report.push_back({cle.to_version,
                              check_report_issue_type::missing_from_repository,
                              script_action::install /* fake */, "", "",
                              cle.action,
                              cle.script_path,
                              cle.sha256_hash});
        },
        [&report,&repo](script_info &script)
        {
            // The repository has something not in the changelog.
            std::string script_hash = calculate_script_hash(
                repo, script.action, script.path);
            report.push_back({script.version,
                              check_report_issue_type::missing_from_changelog,
                              script.action,
                              script.path,
                              script_hash,
                              script_action::install /* fake */, "", ""});
        },
        [&report,&repo](changelog_entry &cle, script_info &script)
        {
            // The changelog and repository have something at the same version.
            // Check the hashes, actions, and paths.
            // TODO - is comparing script paths a sensible check?
            std::string script_hash = calculate_script_hash(
                repo, script.action, script.path);
            if (script_hash != cle.sha256_hash ||
                script.action != cle.action ||
                script.path != cle.script_path) {
                // Mismatch.
                report.push_back({script.version,
                                  check_report_issue_type::hash_mismatch,
                                  script.action,
                                  script.path,
                                  script_hash,
                                  cle.action,
                                  cle.script_path,
                                  cle.sha256_hash});
            }
        },
        cmp, eq);
    
    return report;
}

} // dbmig namespace

