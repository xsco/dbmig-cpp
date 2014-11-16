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

#ifndef DBMIG_EXCEPTION_INCLUDED
#define DBMIG_EXCEPTION_INCLUDED

#include <string>
#include "semantic_version.hpp"

namespace dbmig
{
    ///
    /// Represents the condition when a file path to a script is not parseable
    /// into a suitable semantic version.  This class is abstract.
    ///
    class bad_filename : public std::exception
    {
    public:
        virtual const std::string &path() const noexcept = 0;
        virtual const char *what() const noexcept = 0;
    };
    
    ///
    /// This type of class is thrown when a script in a sub-directory is
    /// encountered that cannot be parsed into a semantic version.
    ///
    class bad_toplevel_filename : public bad_filename
    {
    public:
        bad_toplevel_filename(const std::string &path) : path_(path),
            msg_("Scripts not under sub-directories must be named "
            "X.Y.Z+script.N_desc.sql, file:" + path_) {}
        const std::string &path() const noexcept { return path_; }
        virtual const char *what() const noexcept { return msg_.c_str(); }
    private:
        const std::string path_;
        const std::string msg_;
    };
    
    ///
    /// This type of class is thrown when a script in a sub-directory is
    /// encountered that cannot be parsed into a semantic version.
    ///
    class bad_subdir_filename : public bad_filename
    {
    public:
        bad_subdir_filename(const std::string &path) : path_(path),
            msg_("Scripts under sub-directories must be named "
            "X.Y.Z/[X.Y.Z+][script.]N_desc.sql, file: " + path_) {}
        const std::string &path() const noexcept { return path_; }
        virtual const char *what() const noexcept { return msg_.c_str(); }
    private:
        const std::string path_;
        const std::string msg_;
    };
    
    ///
    /// This type of class is thrown when a script in a sub-directory is
    /// parsed into a semantic version that is missing key information.
    ///
    class incomplete_filename : public bad_filename
    {
    public:
        incomplete_filename(const std::string &path) : path_(path),
            msg_("Directory and/or filename must begin with a parseable "
        "semantic version, containing build metadata matching 'script.nnn', "
        "and followed by an optional underscore before any remaining filename "
        "data: " + path_) {}
        const std::string &path() const noexcept { return path_; }
        virtual const char *what() const noexcept { return msg_.c_str(); }
    private:
        const std::string path_;
        const std::string msg_;
    };
    
    ///
    /// This type of class is thrown when a script dir contains two different
    /// script files that parse to the exact same semantic version.
    ///
    class script_dir_uniqueness_violation : public std::exception
    {
    public:
        script_dir_uniqueness_violation(const semver &ver,
                                        const std::string &path1,
                                        const std::string &path2) :
            ver_(ver), path1_(path1), path2_(path2),
            msg_("Two different files resolve to script version " +
                 ver_.to_str() + ": " + path1_ + " and " + path2_) {}
        const semver ver() const noexcept { return ver_; }
        const std::string &path1() const noexcept { return path1_; }
        const std::string &path2() const noexcept { return path2_; }
        virtual const char *what() const noexcept { return msg_.c_str(); }
    private:
        const semver ver_;
        const std::string path1_;
        const std::string path2_;
        const std::string msg_;
    };

    ///
    /// This type of class is thrown when a script is not a contiguous increment
    /// of a specified earlier version.
    ///    
    class script_noncontiguous : public std::exception
    {
    public:
        script_noncontiguous(const semver &base_version,
                             const semver &script_version,
                             const std::string &script_path) :
            base_version_(base_version),
            script_version_(script_version),
            script_path_(script_path),
            msg_("Script " + script_path_ + " is not a contiguous increment "
                "from version " + base_version_.to_str()) {}
        const semver base_version() const noexcept { return base_version_; }
        const semver script_version() const noexcept { return script_version_; }
        const std::string &script_path() const noexcept { return script_path_; }
        virtual const char *what() const noexcept { return msg_.c_str(); }
    private:
        const semver base_version_;
        const semver script_version_;
        const std::string script_path_;
        const std::string msg_;
    };
    
    class script_changed_since_deployment : public std::exception
    {
    public:
        script_changed_since_deployment(const std::string &changelog_sum,
                                        const std::string &script_sum,
                                        const std::string &script_path) :
            changelog_sum_(changelog_sum),
            script_sum_(script_sum),
            script_path_(script_path),
            msg_("Script " + script_path_ + " has changed since being " +
                 "deployed, original hash = " + changelog_sum_ +
                 ", hash of script on disk = " + script_sum_) {}
        const std::string &changelog_sum() const noexcept {
            return changelog_sum_;
        }
        const std::string &script_sum() const noexcept { return script_sum_; }
        const std::string &script_path() const noexcept { return script_path_; }
        virtual const char *what() const noexcept { return msg_.c_str(); }
    private:
        const std::string changelog_sum_;
        const std::string script_sum_;
        const std::string script_path_;
        const std::string msg_;
    };
}

#endif // DBMIG_EXCEPTION_INCLUDED

