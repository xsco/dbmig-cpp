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

#include "script_dir.hpp"

#include <algorithm>
#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include "semver_compare.hpp"
#include "filename_match.hpp"
#include "exception.hpp"

using namespace std;
using namespace boost::system;
namespace fs = boost::filesystem;

namespace dbmig {

// Private impl class
struct script_dir::impl
{
    explicit impl(const string &script_dir_path) : impl(script_dir_path, ".sql")
    {}
    
    impl(const string &script_dir_path, const string &file_extension) :
        path_(script_dir_path), file_extension_(file_extension), version_map_{}
    {
        preload();
    }

    const string path_;
    const string file_extension_;
    
    map_type version_map_;
    
private:
    
    void preload();
};

// Non-member functions
static bool matches_extension(const string &filename,
                              const string &file_extension);
static semver parse_filename(const string &parent_dir_name,
                             const string &filename);
static semver parse_filename(const string &filename);
static semver parse_semver_candidate(const string &candidate,
                                     const string &path);

///
/// Object generator method for script_dir::iterator_range
///
template<typename Iterator>
static script_dir::iterator_range<Iterator>
make_iterator_range(Iterator iter1, Iterator iter2) {
    return script_dir::iterator_range<Iterator>{
        std::make_pair(std::move(iter1), std::move(iter2))};
}


script_dir::script_dir(const string &path)
    // DRY, grrr
    : pimpl_(new impl(path))
{}
script_dir::script_dir(const string &path, const string &glob)
    // DRY, grrr
    : pimpl_(new impl(path, glob))
{}

script_dir::~script_dir() = default;


void script_dir::impl::preload()
{
    // Clear any existing cached version map.
    version_map_.clear();
    
    fs::path root(path_);
    if (!exists(root) || !is_directory(root))
    {
        // Path does not exist on disk.
        throw fs::filesystem_error("Script directory path does not exist", root,
            errc::make_error_code(errc::no_such_file_or_directory));
    }
    
    // Examine directory contents.
    for (auto it = fs::directory_iterator(root); it != fs::directory_iterator();
        ++it)
    {
        auto &p = it->path();
        bool is_dir = fs::is_directory(p);
        bool is_file = fs::is_regular_file(p);
        if (fs::is_symlink(p))
        {
            // Is it a symlink to a file or directory?
            // TODO
        }
        
        if (is_dir)
        {
            // Iterate over sub-dir contents.
            for (auto sd_it = fs::directory_iterator(p); sd_it !=
                fs::directory_iterator(); ++sd_it)
            {
                // Only concerned with regular files now.
                // TODO - and symlinks to regular files?
                auto sd_file = sd_it->path();
                if (!fs::is_regular_file(sd_file))
                    continue;
                
                // Only suitable file extensions.
                auto sub_filename = sd_file.filename().native();
                if (!matches_extension(sub_filename, file_extension_))
                    continue;
                
                // Try to parse a version from this file, taking into account
                // the name of its parent directory, which may contribute.
                auto parent_dir = p.filename().native();
                auto sub_ver = parse_filename(parent_dir, sub_filename);
                auto sub_path = parent_dir + "/" + sub_filename;
                if (version_map_.count(sub_ver)) {
                    throw script_dir_uniqueness_violation(
                        sub_ver, sub_path, version_map_[sub_ver]);
                }
                version_map_.insert(value_type{sub_ver, sub_path});
            }
        }
        else if (is_file)
        {
            // Only suitable file extensions.
            auto filename = p.filename().native();
            if (!matches_extension(filename, file_extension_))
                continue;
            
            // Try to parse a version from this file and add to the map.
            auto ver = parse_filename(filename);
            if (version_map_.count(ver)) {
                throw script_dir_uniqueness_violation(
                    ver, filename, version_map_[ver]);
            }
            version_map_.insert(value_type{ver, filename});
        }
    }
}


///
/// Get the path of the script directory.
///
const std::string &script_dir::path() const
{
    auto &path_ = pimpl_->path_;
    return path_;
}

///
/// Get the file extension pattern in use.
///
const std::string &script_dir::file_extension() const
{
    auto &file_extension_ = pimpl_->file_extension_;
    return file_extension_;
}

///
/// Return const_iterator to the beginning of the script directory.
///
script_dir::const_iterator script_dir::begin() const
{
    auto &version_map_ = pimpl_->version_map_;
    return version_map_.begin();
}

///
/// Return a const_reverse_iterator to the reverse beginning of the dir.
///
script_dir::const_reverse_iterator script_dir::rbegin() const
{
    auto &version_map_ = pimpl_->version_map_;
    return version_map_.rbegin();
}

///
/// Return a const_reverse_iterator to the reverse beginning of the dir.
///
script_dir::const_reverse_iterator script_dir::rend() const
{
    auto &version_map_ = pimpl_->version_map_;
    return version_map_.rend();
}

///
/// Return const_iterator to the end of the script directory.
///
script_dir::const_iterator script_dir::end() const
{
    auto &version_map_ = pimpl_->version_map_;
    return version_map_.end();
}

///
/// Return const_iterator to the beginning of the script directory.
///
script_dir::const_iterator script_dir::cbegin() const noexcept
{
    auto &version_map_ = pimpl_->version_map_;
    return version_map_.cbegin();
}

///
/// Return const_iterator to the end of the script directory.
///
script_dir::const_iterator script_dir::cend() const noexcept
{
    auto &version_map_ = pimpl_->version_map_;
    return version_map_.cend();
}

///
/// Return a const_reverse_iterator to the reverse beginning of the dir.
///
script_dir::const_reverse_iterator script_dir::crbegin() const
{
    auto &version_map_ = pimpl_->version_map_;
    return version_map_.crbegin();
}

///
/// Return a const_reverse_iterator to the reverse beginning of the dir.
///
script_dir::const_reverse_iterator script_dir::crend() const
{
    auto &version_map_ = pimpl_->version_map_;
    return version_map_.crend();
}

///
/// Get an iterator to the first script greater than the given version
///
/// This method will return an iterator that points to the first such
/// element in the script directory that is greater than the specified
/// semantic version.
///
script_dir::const_iterator
script_dir::first_greater (const semver &version) const
{
    auto &version_map_ = pimpl_->version_map_;

    // TODO - make this more efficient perhaps?
    // TODO - replace with upper_bound()?
    // Note that lower_bound() will return the first script >= rather than >.
    semver_script_compare<non_script_alignment::low> cmp;
    auto iter = std::lower_bound(version_map_.begin(), version_map_.end(),
        version,
        [&cmp](const value_type &kvp, const semver &v)
        {
            return cmp(kvp.first, v);
        });
    // If we got the one asked for, then increment to get the next.
    if (iter != version_map_.end() && iter->first == version)
        ++iter;
    return iter;
}

///
/// Get an iterator beyond the last script less than the given version
///
/// This method will return an iterator that points beyond the last such
/// element in the script directory that is less than or equal to the
/// specified semantic version.
///
/// If a version X.Y.Z is supplied that does not contain script.N build
/// meta-data, then the iterator returned will actually point beyond
/// the greatest element in the script directory that has at least
/// X.Y.Z as its principle version parts.  This is permissible under
/// semver v2.0, since there is no defined order to build meta-data, and
/// it also seems semantically compatible with the idea of an upper
/// bound on versioned scripts that derive meaning from script.N build
/// meta-data.
///
script_dir::const_iterator
script_dir::last_less_equal (const semver &version) const
{
    auto &version_map_ = pimpl_->version_map_;

    semver_script_compare<non_script_alignment::high> cmp;
    return std::upper_bound(version_map_.begin(), version_map_.end(), version,
        [&cmp](const semver &v, const value_type &kvp) { return cmp(v, kvp.first); });
}

///
/// Get the bounds of a range of elements in the script directory.
///
/// The range will run from the first script greater than the supplied
/// from version, to the last script less or equal to the to version.
///
/// If a from version is supplied that does not contain script meta-data
/// then it is considered to belong before any scripts that match its
/// major/minor/patch parts.
///
/// If a to version is supplied that does not contain script meta-data
/// then it is considered to belong after any scripts that match its
/// major/minor/patch parts.
///
script_dir::const_iterator_range
script_dir::range (const semver &from_version, const semver &to_version) const
{
    auto l_it = first_greater(from_version);
    // Note that the only reason this method doesn't delegate to
    // script_dir::upper_bound here is so that the call to std::upper_bound can
    // be passed in a more efficient first iterator.
    auto &version_map_ = pimpl_->version_map_;
    semver_script_compare<non_script_alignment::high> hcmp;
    script_dir::const_iterator end = version_map_.end(); // auto will break!
    auto u_it = std::upper_bound(l_it, end, to_version,
        [&](const semver &v, const value_type &kvp) { return hcmp(v, kvp.first); });
    return make_iterator_range(l_it, u_it);
}

///
/// Find out if a filename matches a given extension.
///
static bool matches_extension(const string &filename,
                              const string &file_extension)
{
    return filename_match(filename, "*" + file_extension);
}

///
/// Try to extract a semantic version by parsing a directory name and filename
///
static semver parse_filename(const string &parent_dir_name, const string &filename)
{
    // Several valid options:
    // 1. Dir is X.Y.Z, and filename is X.Y.Z+script.N_foo.sql
    // 2. Dir is X.Y.Z, and filename is X.Y.Z+script.N.sql
    // 3. Dir is X.Y.Z, and filename is script.N_foo.sql
    // 4. Dir is X.Y.Z, and filename is script.N.sql
    // 5. Dir is X.Y.Z, and filename is N_foo.sql
    // 6. Dir is X.Y.Z, and filename is N.sql
    // The above should result in a semantic version of X.Y.Z+script.N
    static const boost::regex pat{
        R"((\d+\.\d+\.\d+)/(?:\1\+)?(?:script\.)?(\d+)(?:_.*)?(?:\..*)?)"};
    
    boost::smatch results;
    auto str = parent_dir_name + "/" + filename;
    auto matched = boost::regex_match(str, results, pat,
        boost::match_default | boost::match_partial);
    if (!matched) {
        throw bad_subdir_filename(str);
    }

    auto candidate = results[1].str() + "+script." + results[2].str();
    return parse_semver_candidate(candidate, str);
}

///
/// Try to extract a semantic version by parsing a filename
///
static semver parse_filename(const string &filename)
{
    // Several valid options:
    // 1. Filename is X.Y.Z+script.N_foo.sql
    // 2. Filename is X.Y.Z+script.N.sql
    // The above should result in a semantic version of X.Y.Z+script.N
    static const boost::regex pat{
        R"((\d+\.\d+\.\d+)\+script\.(\d+)(?:_.*)?(?:\..*)?)"};
    
    boost::smatch results;
    auto matched = boost::regex_match(filename, results, pat,
        boost::match_default | boost::match_partial);
    if (!matched) {
        throw bad_toplevel_filename(filename);
    }

    auto candidate = results[1].str() + "+script." + results[2].str();
    return parse_semver_candidate(candidate, filename);
}

///
/// Try to parse a candidate string (either a filename, or a concatenated
/// dir/filename with path separator replaced by '+') to see if it's a valid
/// semantic version for a script.
///
static semver parse_semver_candidate(const string &candidate,
                                     const string &path)
{
    auto ver = semver::parse(candidate);
    auto &bm_ids = ver.bm_ids();

    if (bm_ids.size() >= 2 &&
        !bm_ids[0].is_numeric && bm_ids[0].str_value == "script" &&
        bm_ids[1].is_numeric)
    {
        // Reconstruct the semver, to ensure that a numerical representation
        // of the script number is used.  This has the effect of removing any
        // leading zeroes in the database changelog that may have been used in
        // the filename for convenience of ordering.
        std::string bms = "script." + std::to_string(bm_ids[1].numeric_value);
        return semver{ver.mj(), ver.mn(), ver.pt(), "", bms};
    }

    // Doesn't pass the tests.
    throw incomplete_filename(path);
}


} // dbmig namespace

