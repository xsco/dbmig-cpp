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

#include "changelog.hpp"

#include <soci/soci.h>
#include "changelog_table.hpp"

using namespace std;


namespace dbmig {

struct changelog::impl
{
    impl(
        const std::string &conn_str,
        const std::string &changeset)
        :
        session_{conn_str},
        cl_table_{session_, changeset}
    {}
    
    soci::session session_;
    changelog_table cl_table_;
};


changelog::changelog(
    const std::string &conn_str,
    const std::string &changeset)
    // DRY, grrr
    : pimpl_(new impl(conn_str, changeset))
{}

changelog::~changelog() = default;

///
/// Is a changelog table installed on the database?
///
const bool changelog::installed() const
{
    return pimpl_->cl_table_.installed();
}

///
/// Get the currently-installed version of the database.
///
const semver changelog::version() const
{
    return pimpl_->cl_table_.version();
}

///
/// Get the most recent previously-installed version of the database.
///
const semver changelog::previous_version() const
{
    return pimpl_->cl_table_.previous_version();
}

///
/// Get the version that the database could be rolled back to
///
const semver changelog::rollback_version() const
{
    return pimpl_->cl_table_.rollback_version();
}

///
/// Get a list of steps to take to roll back to a given version
///
const rollback_step_list
changelog::rollback_steps(const semver &ver) const
{
    return pimpl_->cl_table_.rollback_steps(ver);
}

///
/// Get a list of the last batch of contiguous changelog entries
///
const changelog_entry_list
changelog::contiguous_history(bool exclude_rolled_back) const
{
    return pimpl_->cl_table_.contiguous_history(exclude_rolled_back);
}

///
/// Force the changelog to a certain version.
///
void changelog::override_version(const semver &ver)
{
    // Start transaction
    soci::transaction txn{pimpl_->session_};
    
    pimpl_->cl_table_.override_version(ver);
    
    // Commit
    txn.commit();
}

} // dbmig namespace

