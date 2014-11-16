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

#include "changelog_table.hpp"

#include "db_specific.hpp"
#include "time.hpp"

using namespace std;


namespace dbmig {

struct changelog_table::impl
{
    impl(
        soci::session &session,
        const std::string &changeset)
        :
        session_(session),
        changeset_(changeset),
        // Identify which back-end is in use from the session, and set the
        // appropriate DB-specific SQL strings.
        sql_(get_db_specific(session_.get_backend_name()))
    {}
    
    void create_changelog();
    void drop_changelog();

    soci::session &session_;
    std::string changeset_;
    const db_specific &sql_;
};


changelog_table::changelog_table(
    soci::session &session,
    const std::string &changeset)
    // DRY, grrr
    : pimpl_(new impl(session, changeset))
{}

changelog_table::~changelog_table() = default;

void changelog_table::impl::create_changelog()
{
    session_ << sql_.create_changelog_sql;
}

void changelog_table::impl::drop_changelog()
{
    session_ << sql_.drop_changelog_sql;
}

///
/// Is a changelog table installed on the database?
///
const bool changelog_table::installed() const
{
    using namespace soci;
    auto &session_   = pimpl_->session_;
    auto &sql_       = pimpl_->sql_;
    
    int num_changelog_tables;
    session_ << sql_.changelog_exists_sql, into(num_changelog_tables);
    return num_changelog_tables > 0;
}

///
/// Get the currently-installed version of the database.
///
const semver changelog_table::version() const
{
    if (!installed())
        return semver::zero();

    using namespace soci;
    auto &session_   = pimpl_->session_;
    auto &sql_       = pimpl_->sql_;
    auto &changeset_ = pimpl_->changeset_;
    
    string ver_str;
    session_ << sql_.latest_version_sql, into(ver_str), use(changeset_);
    return semver::parse(ver_str);
}

///
/// Get the most recent previously-installed version of the database.
///
const semver changelog_table::previous_version() const
{
    if (!installed())
        return semver::zero();

    using namespace soci;
    auto &session_   = pimpl_->session_;
    auto &sql_       = pimpl_->sql_;
    auto &changeset_ = pimpl_->changeset_;

    string ver_str;
    session_ << sql_.previous_version_sql, into(ver_str), use(changeset_);
    return semver::parse(ver_str);
}

///
/// Get the version that the database could be atomically rolled back to
///
const semver changelog_table::rollback_version() const
{
    // TODO
    return semver{0,0,0};
}

///
/// Get a list of steps to take to roll back to a given version
///
const rollback_step_list
changelog_table::rollback_steps(const semver &ver) const
{
    if (!installed())
        return rollback_step_list{};

    using namespace soci;
    auto &session_   = pimpl_->session_;
    auto &sql_       = pimpl_->sql_;
    auto &changeset_ = pimpl_->changeset_;
    
    // TODO all this needs testing!  What is from_ver is null?!?
    
    // Get all changelog entries since the rollback target.
    rollback_step_list steps;
    string target_ver_str = ver.to_str();
    string action_str, from_ver_str, to_ver_str, hash;
    statement st = (session_.prepare << sql_.rollback_steps_sql,
               into(action_str), into(from_ver_str), into(to_ver_str),
               into(hash),
               use(changeset_, "changeset"),
               use(target_ver_str, "rollback_ver"));
    st.execute();
    int num_to_skip = 0;
    while (st.fetch()) {
        // Go through rows, and filter out entries already rolled back
        auto action = script_action_parse(action_str);
        if (action == script_action::rollback) {
            ++num_to_skip;
            continue;
        }
        else if (num_to_skip > 0) {
            --num_to_skip;
            continue;
        }
        
        // Add to the list.
        auto from_ver = semver::parse(from_ver_str);
        auto to_ver   = semver::parse(to_ver_str);
        steps.push_back({to_ver, from_ver, hash});
    }

    return steps;
}

///
/// Get a list of the last batch of contiguous changelog entries
///
const changelog_entry_list
changelog_table::contiguous_history(bool exclude_rolled_back) const
{
    if (!installed())
        return changelog_entry_list{};

    using namespace soci;
    auto &session_   = pimpl_->session_;
    auto &sql_       = pimpl_->sql_;
    auto &changeset_ = pimpl_->changeset_;

    // Get all changelog entries since last install/override.
    changelog_entry_list entries;
    string script_path, action_str, from_ver_str, to_ver_str, hash;
    indicator from_ver_ind;
    statement st = (session_.prepare << sql_.contiguous_history_sql,
               into(script_path), into(action_str),
               into(from_ver_str, from_ver_ind), into(to_ver_str), into(hash),
               use(changeset_, "changeset"));
    st.execute();
    
    int num_to_skip = 0;
    while (st.fetch()) {
        // Skip overrides, as these are not real script actions.
        if (action_str == "override")
            continue;
    
        auto action = script_action_parse(action_str);
        if (exclude_rolled_back) {
            // Filter out entries already rolled back
            if (action == script_action::rollback) {
                ++num_to_skip;
                continue;
            }
            else if (num_to_skip > 0) {
                --num_to_skip;
                continue;
            }
        }
        
        // Add to the list.
        auto from_ver = (from_ver_ind == i_null)
                        ? semver::zero()
                        : semver::parse(from_ver_str);
        auto to_ver   = semver::parse(to_ver_str);
        // Note that we push onto the front, since the resultset is in
        // reverse chronological order, and we want to return chronological.
        entries.push_front({script_path, action, from_ver, to_ver, hash});
    }
    return entries;
}

///
/// Force the changelog to a certain version.
///
void changelog_table::override_version(const semver &ver)
{
    using namespace soci;
    auto &session_   = pimpl_->session_;
    auto &sql_       = pimpl_->sql_;
    auto &changeset_ = pimpl_->changeset_;

    // If there is no changelog table installed, create it now.
    if (!installed())
        pimpl_->create_changelog();

    auto now_local = time::localtime(time::now());
    string script_path = ""; // No path when the version is forced
    string action_str = "override";
    string from_version;
    indicator from_version_ind = i_null;
    string to_version = ver.to_str();
    // This is the SHA256 hash of the empty string
    string hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    string interval = "00:00:00"; // zero interval since nothing ran

    // Insert a new changelog row.
    session_ << sql_.insert_sql,
        use(changeset_, "changeset"),
        use(now_local, "applied"),
        use(script_path, "script_path"),
        use(action_str, "action"),
        use(from_version, from_version_ind, "from_version"),
        use(to_version, "to_version"),
        use(hash, "sha256_hash"),
        use(interval, "time_taken");
}

///
/// Write a new entry to the changelog (when installing from scratch)
///
void changelog_table::write(
    const time_t &applied,
    const string &script_path,
    const semver &install_version,
    const string &sha256_hash,
    const double seconds)
{
    using namespace soci;
    auto &session_   = pimpl_->session_;
    auto &sql_       = pimpl_->sql_;
    auto &changeset_ = pimpl_->changeset_;

    // If there is no changelog table installed, create it now.
    if (!installed())
        pimpl_->create_changelog();

    auto applied_local = time::localtime(applied);
    string action_str = to_string(script_action::install);
    string from_version_str;
    indicator from_version_ind = i_null;
    string to_version_str = install_version.to_str();
    
    // Insert a new changelog row.
    session_ << sql_.insert_sql,
        use(changeset_, "changeset"),
        use(applied_local, "applied"),
        use(script_path, "script_path"),
        use(action_str, "action"),
        use(from_version_str, from_version_ind, "from_version"),
        use(to_version_str, "to_version"),
        use(sha256_hash, "sha256_hash"),
        use(seconds, "time_taken");
}

///
/// Write a new entry to the changelog (with from-version)
///
void changelog_table::write(
    const time_t &applied,
    const string &script_path,
    const script_action &action,
    const semver &from_version,
    const semver &to_version,
    const string &sha256_hash,
    const double seconds)
{
    using namespace soci;
    auto &session_   = pimpl_->session_;
    auto &sql_       = pimpl_->sql_;
    auto &changeset_ = pimpl_->changeset_;

    // If there is no changelog table installed, create it now.
    if (!installed())
        pimpl_->create_changelog();

    auto applied_local = time::localtime(applied);
    string from_version_str = from_version.to_str();
    string to_version_str = to_version.to_str();
    string action_str = to_string(action);
    
    // Insert a new changelog row.
    session_ << sql_.insert_sql,
        use(changeset_, "changeset"),
        use(applied_local, "applied"),
        use(script_path, "script_path"),
        use(action_str, "action"),
        use(from_version_str, "from_version"),
        use(to_version_str, "to_version"),
        use(sha256_hash, "sha256_hash"),
        use(seconds, "time_taken");
}

} // dbmig namespace

