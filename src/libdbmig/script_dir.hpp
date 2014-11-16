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

#ifndef DBMIG_SCRIPT_DIR_INCLUDED
#define DBMIG_SCRIPT_DIR_INCLUDED

#include <string>
#include <memory>
#include <utility>
#include <map>
#include "semantic_version.hpp"

namespace dbmig
{
    ///
    /// Represents a directory on disk containing versioned scripts
    ///
    /// The key feature is that it must be possible to associate every script
    /// with a semantic version, which by convention is derived from the
    /// filename (and possibly sub-directory) of the script within the base
    /// script directory.
    ///
    /// This class behaviours a "little bit" like an associative container, in
    /// that it can be thought of like a multimap keyed on semantic version,
    /// and mapped to file path on disk.
    ///
    class script_dir
    {
    public:
    
        typedef semver key_type;
        typedef std::string mapped_type;
        typedef std::pair<key_type, mapped_type> value_type;
        typedef semver::metadata_compare key_compare;
        typedef std::map<key_type, mapped_type, key_compare> map_type;
        typedef map_type::const_iterator const_iterator;
        typedef map_type::const_reverse_iterator const_reverse_iterator;
        
        ///
        /// Extend std::pair with begin/end to behave like real iterator range
        ///
        template <typename Iterator>
        struct iterator_range : public std::pair<Iterator, Iterator>
        {
            using pair_type = std::pair<Iterator, Iterator>;
            iterator_range(pair_type &&p)
                : std::pair<Iterator, Iterator>(std::forward<pair_type>(p))
            {}
            
            Iterator begin() const { return pair_type::first; }
            Iterator end() const { return pair_type::second; }
        };
        
        typedef iterator_range<const_iterator> const_iterator_range;
        typedef iterator_range<const_reverse_iterator>
            const_reverse_iterator_range;
    
        explicit script_dir(const std::string &path);
        script_dir(const std::string &path, const std::string &file_extension);
        ~script_dir();
        
        ///
        /// Get the path of the script directory.
        ///
        const std::string &path() const;
        
        ///
        /// Get the file extension pattern in use.
        ///
        const std::string &file_extension() const;

        ///
        /// Return const_iterator to the beginning of the script directory.
        ///
        const_iterator begin() const;
        
        ///
        /// Return const_iterator to the end of the script directory.
        ///
        const_iterator end() const;
        
        ///
        /// Return a const_reverse_iterator to the reverse beginning of the dir.
        ///
        const_reverse_iterator rbegin() const;
        
        ///
        /// Return a const_reverse_iterator to the reverse beginning of the dir.
        ///
        const_reverse_iterator rend() const;
        
        ///
        /// Return const_iterator to the beginning of the script directory.
        ///
        const_iterator cbegin() const noexcept;
        
        ///
        /// Return const_iterator to the end of the script directory.
        ///
        const_iterator cend() const noexcept;
        
        ///
        /// Return a const_reverse_iterator to the reverse beginning of the dir.
        ///
        const_reverse_iterator crbegin() const;
        
        ///
        /// Return a const_reverse_iterator to the reverse beginning of the dir.
        ///
        const_reverse_iterator crend() const;
        
        ///
        /// Get an iterator to the first script greater than the given version
        ///
        /// This method will return an iterator that points to the first such
        /// element in the script directory that is greater than the specified
        /// semantic version.
        ///
        const_iterator first_greater (const semver &version) const;
        
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
        const_iterator last_less_equal (const semver &version) const;
        
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
        const_iterator_range range (const semver &from_version,
                                    const semver &to_version) const;

    private:
    
        struct impl;
        std::unique_ptr<impl> pimpl_;
    };
}

#endif // DBMIG_SCRIPT_DIR_INCLUDED

