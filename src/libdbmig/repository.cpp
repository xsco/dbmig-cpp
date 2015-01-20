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

#include "repository.hpp"

#include <algorithm>
#include <numeric>
#include <boost/filesystem.hpp>
#include <nowide/fstream.hpp>
#include <stdexcept>

#include "script_dir.hpp"
#include "script_stream.hpp"
#include "exception.hpp"

using std::string;
using nowide::ifstream;

namespace dbmig {

struct repository::impl
{
    explicit impl(const string &path) :
        latest_schema_path_(path + "/latest"),
        upgrade_script_dir_(path + "/upgrade"),
        install_script_dir_(path + "/install")
    {}
    impl(
        const string &latest_schema_path,
        const string &upgrade_script_path,
        const string &install_script_path)
        :
        latest_schema_path_(latest_schema_path),
        upgrade_script_dir_(upgrade_script_path),
        install_script_dir_(install_script_path)
    {}

    const string latest_schema_path_;
    
    script_dir upgrade_script_dir_;
    script_dir install_script_dir_;
};



repository::repository(const string &path)
    // DRY, grrr
    : pimpl_(new impl(path))
{}
repository::repository(
    const string &latest_schema_path,
    const string &upgrade_script_path,
    const string &install_script_path)
    // DRY, grrr
    : pimpl_(new impl(
        latest_schema_path, upgrade_script_path, install_script_path))
{}

repository::~repository() = default;


const string &repository::latest_schema_path() const
{
    auto &latest_schema_path_ = pimpl_->latest_schema_path_;
    return latest_schema_path_;
}

const string &repository::upgrade_script_path() const
{
    auto &upgrade_script_dir_ = pimpl_->upgrade_script_dir_;
    return upgrade_script_dir_.path();
}

const string &repository::install_script_path() const
{
    auto &install_script_dir_ = pimpl_->install_script_dir_;
    return install_script_dir_.path();
}

///
/// The latest version available in the repository.
///
/// Returns the latest version in the repository that scripts are
/// available for.  If the repository is empty, the return value is
/// semver::zero().
///
semver repository::latest_version() const
{
    auto &install_script_dir_ = pimpl_->install_script_dir_;
    auto &upgrade_script_dir_ = pimpl_->upgrade_script_dir_;

    auto ins_iter = install_script_dir_.crbegin();
    auto upg_iter = upgrade_script_dir_.crbegin();
    
    auto ins_ver = ins_iter == install_script_dir_.crend()
        ? semver::zero() : ins_iter->first;
    auto upg_ver = upg_iter == upgrade_script_dir_.crend()
        ? semver::zero() : upg_iter->first;
    
    return std::max(upg_ver, ins_ver);
}

///
/// Return iterator range over the (single) nearest install script
///
/// If the repository contains no suitable install script, the range
/// will be empty.
///
script_dir::const_iterator_range repository::nearest_install_script(
    const semver &target) const
{
    auto &install_script_dir_ = pimpl_->install_script_dir_;
    
    // The nearest install script will be the last one that has a version less
    // than or equal to the requested target version.
    auto e = install_script_dir_.last_less_equal(target);
    auto b = e;
    if (b != install_script_dir_.cbegin()) {
        // last_less_equal() points beyond the element we want, so wind back.
        --b;
    }
    
    return std::make_pair(b, e);
}

///
/// Return iterator range over upgrade actions
///
/// Given a starting version, and a target version that is later than
/// the starting version, this method will provide an iterator range
/// over upgrade_actions that will take an installation to the target
/// version from the starting version.
///
script_dir::const_iterator_range repository::upgrade_scripts(
    const semver &start, const semver &target) const
{
    auto &upgrade_script_dir_ = pimpl_->upgrade_script_dir_;
    
    // Check that each script is a valid step from the previous version.
    // i.e  from starting version X.Y.Z+script.N, the valid next steps are:
    //  * (X+1).0.0+script.M where M > 0
    //  * X.(Y+1).0+script.M where M > 0
    //  * X.Y.(Z+1)+script.M where M > 0
    //  * X.Y.Z+script.M     where M > N
    // That is, only incremental changes are permitted in the major/minor/patch
    // part of the semantic version, but the script number may jump ahead by any
    // amount, to cater for removed/retired scripts from in-dev versions.
    auto range = upgrade_script_dir_.range(start, target);
    
    semver::strict_compare cmp;
    semver last_version = std::accumulate(range.first, range.second, start,
        [&](semver prev, script_dir::value_type script_entry)
        {
            auto &next = script_entry.first;
            
            // Is the next version within the same X.Y.Z version of the prev?
            if (cmp.compare_to(prev, next) == 0) {
                return next;
            }
                
            // Is the next version a patch increment of the first?
            prev.next_patch();
            if (cmp.compare_to(prev, next) == 0) {
                return next;
            }
            
            // Is the next version a minor increment of the first?
            prev.next_minor();
            if (cmp.compare_to(prev, next) == 0) {
                return next;
            }
            
            // Is the next version a major increment of the first?
            prev.next_major();
            if (cmp.compare_to(prev, next) == 0) {
                return next;
            }
            
            // The next version is not contiguous with the first.
            throw script_noncontiguous{prev, next, script_entry.second};
        });
    
    return range;
}

///
/// Range over the (single) upgrade script matching a given version
///
script_dir::const_iterator_range
repository::upgrade_script_at(const semver &ver) const
{
    auto &upgrade_script_dir_ = pimpl_->upgrade_script_dir_;
    
    auto e = upgrade_script_dir_.last_less_equal(ver);
    auto b = e;
    if (b != upgrade_script_dir_.cbegin()) {
        // last_less_equal() points beyond the element we want, so wind back.
        --b;
    }
    // Check we got the version we wanted.
    if (b != upgrade_script_dir_.cend() && b->first != ver)
        return std::make_pair(b, b);
    
    return std::make_pair(b, e);
}

///
/// Convenience method to obtain the SHA256 hash of a given script
///
std::string
calculate_script_hash(const repository &repo,
                      const script_action &action,
                      const std::string &script_path)
{
    switch (action) {
        case script_action::install:
        {
            string path = repo.install_script_path() + "/" + script_path;
            ifstream ifs{path};
            return read_install_statements(ifs).sha256_sum();
        }
        case script_action::upgrade:
        {
            string path = repo.upgrade_script_path() + "/" + script_path;
            ifstream ifs{path};
            return read_upgrade_statements(ifs).sha256_sum();
        }
        case script_action::rollback:
        {
            string path = repo.upgrade_script_path() + "/" + script_path;
            ifstream ifs{path};
            return read_rollback_statements(ifs).sha256_sum();
        }
    }
    throw std::out_of_range{to_string(action)};
}

} // dbmig namespace

