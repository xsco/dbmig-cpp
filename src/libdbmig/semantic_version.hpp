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

#ifndef SEMANTIC_VERSION
#define SEMANTIC_VERSION

#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace dbmig
{
    /**
        The semantic_version class represents an implementation of Semantic
        Versioning 2.0.0, as defined at http://www.semver.org
    */
    template<typename Tn, typename Ts>
    class semantic_version
    {
    public:
    
        struct appended_identifier_part
        {
            Ts str_value;
            bool is_numeric;
            Tn numeric_value;
            
            friend std::ostream & operator << (
                    std::ostream &os, const appended_identifier_part &aip)
            {
                return os << aip.str_value;
            }
        };
    
        typedef std::vector<appended_identifier_part> id_list;
    
    private:
    
        // NOTE: Watch out for the 'major' and 'minor' macros that can usually
        // be found in <sys/types.h> (for GNU, this is <sys/sysmacros.h>).  So
        // to avoid clashes, we abbreviate.
        Tn mj_; // major
        Tn mn_; // minor
        Tn pt_; // patch
        id_list pr_ids_;  // pre-release
        id_list bm_ids_;  // build metadata

        // String expressions, lazy instantiation.
        mutable Ts str_expr_;
        mutable Ts pr_str_expr_;
        mutable Ts bm_str_expr_;
        
        void set_dirty()
        {
            str_expr_ = "";
            pr_str_expr_ = "";
            bm_str_expr_ = "";
        }
    
    public:
    
        semantic_version(Tn mj) :
            mj_(mj), mn_(0), pt_(0), pr_ids_(), bm_ids_(),
            str_expr_(""), pr_str_expr_(""), bm_str_expr_("")
        {}
        
        semantic_version(Tn mj, Tn mn, Tn pt) :
            mj_(mj), mn_(mn), pt_(pt), pr_ids_(), bm_ids_(),
            str_expr_(""), pr_str_expr_(""), bm_str_expr_("")
        {}
        
        semantic_version(
            Tn mj, Tn mn, Tn pt,
            Ts pr_str, Ts bm_str) :
            mj_(mj), mn_(mn), pt_(pt),
            pr_ids_(parse_prerelease_str(pr_str)),
            bm_ids_(parse_build_metadata_str(bm_str)),
            str_expr_(""), pr_str_expr_(""), bm_str_expr_("")
        {}
        
        /**
            Copy constructor
        */
        semantic_version(const semantic_version &sv) :
            mj_(sv.mj_), mn_(sv.mn_), pt_(sv.pt_),
            pr_ids_(sv.pr_ids_), bm_ids_(sv.bm_ids_)
        {}
        
        /**
            Copy assignment
        */
        semantic_version &operator =(const semantic_version &v)
        {
            mj_ = v.mj();
            mn_ = v.mn();
            pt_ = v.pt();
            pr_ids_ = v.pr_ids();
            bm_ids_ = v.bm_ids();
            str_expr_ = v.str_expr_;
            pr_str_expr_ = v.pr_str_expr_;
            bm_str_expr_ = v.bm_str_expr_;
            return *this;
        }
        
        // TODO -implement move constructor and move assignment if C++11
        
    private:
        
        // ctor used only internally by parse() method.
        semantic_version(
            Tn mj, Tn mn, Tn pt,
            id_list pr_ids, id_list bm_ids) :
            mj_(mj), mn_(mn), pt_(pt), pr_ids_(pr_ids), bm_ids_(bm_ids),
            str_expr_(""), pr_str_expr_(""), bm_str_expr_("")
        {}
        
    public:
        
        // Getters/setters of public properties.
        
        const Tn mj() const { return mj_; }
        void mj(Tn mj) { mj_ = mj; set_dirty(); }
        const Tn mn() const { return mn_; }
        void mn(Tn mn) { mn_ = mn; set_dirty(); }
        const Tn pt() const { return pt_; }
        void pt(Tn pt) { pt_ = pt; set_dirty(); }
        
        const id_list &pr_ids() const { return pr_ids_; }
        const id_list &bm_ids() const { return bm_ids_; }
        
    public:
        
        ///
        /// A semantic version representing zero, i.e. no version
        ///
        static const semantic_version<Tn, Ts> zero()
        {
            return semantic_version<Tn, Ts>{0, 0, 0};
        }
        
        ///
        /// A semantic version that defines the initial public API, i.e. 1.0.0
        ///
        /// From the semver 2.0.0 specification, clause 5:
        /// Version 1.0.0 defines the public API. The way in which the version
        /// number is incremented after this release is dependent on this public
        /// API and how it changes.
        ///
        static const semantic_version<Tn, Ts> initial_public_api()
        {
            return semantic_version<Tn, Ts>{1, 0, 0};
        }
        
    private:
    
        static Tn parse_numerical_version_part(
                const Ts &version_part_str)
        {
            // From the semver 2.0.0 specification, clause 2:
            // A normal version number MUST take the form X.Y.Z where X, Y, and
            // Z are non-negative integers, and MUST NOT contain leading zeroes.
            
            if (version_part_str == "")
                throw std::domain_error("Version part must not be empty");
            if (version_part_str[0] == '0' && version_part_str.length() != 1)
                throw std::domain_error("Version part must not contain leading zeroes - got: " + version_part_str);
            for (typename Ts::const_iterator i = version_part_str.begin();
                i != version_part_str.end(); ++i)
            {
                if (!isdigit(*i))
                    throw std::domain_error("Version part must be a non-negative integer - got: " + version_part_str);
            }

            return atoi(version_part_str.c_str());
        }
        
        static id_list parse_prerelease_str(const Ts &prerelease_str)
        {
            // From the semver 2.0.0 specification, clause 9:
            // A pre-release version MAY be denoted by appending a hyphen and
            // a series of dot separated identifiers immediately following the
            // patch version.
            // Identifiers MUST comprise only ASCII alphanumerics and hyphen
            // [0-9A-Za-z-].
            // Identifiers MUST NOT be empty.
            // Numeric identifiers MUST NOT include leading zeroes.
            id_list v;
            if (prerelease_str == "")
                return v;

            // Handle situation where input string ends in a dot.  This won't
            // get caught by the std::getline() loop.
            if (prerelease_str[prerelease_str.length() - 1] == '.')
                throw std::domain_error("Dot-separated pre-release version parts must not be empty - got: " + prerelease_str);

            appended_identifier_part aip;
            std::stringstream ss(prerelease_str);
            for (
                std::getline(ss, aip.str_value, '.');
                (ss.rdstate() & std::stringstream::failbit) == 0;
                std::getline(ss, aip.str_value, '.'))
            {
                if (aip.str_value == "")
                    throw std::domain_error("Dot-separated pre-release version parts must not be empty - got: " + prerelease_str);
                aip.is_numeric = true;
                for (typename Ts::const_iterator i = aip.str_value.begin();
                    i != aip.str_value.end(); ++i)
                {
                    if (!isalnum(*i) && *i != '-')
                        throw std::domain_error("Dot-separated pre-release version parts must comprise only ASCII alphanumerics and hyphen - got: " + prerelease_str);
                    if (!isdigit(*i))
                        aip.is_numeric = false;
                }
                if (aip.is_numeric)
                {
                    if (aip.str_value.length() > 1 && aip.str_value[0] == '0')
                        throw std::domain_error("Dot-separated numeric pre-release version parts must not include leading zeroes - got: " + prerelease_str);
                    aip.numeric_value = atoi(aip.str_value.c_str());
                }
                v.push_back(aip);
            }
            return v;
        }
        
        static id_list parse_build_metadata_str(const Ts &build_metadata_str)
        {
            // From the semver 2.0.0 specification, clause 10:
            // Build metadata MAY be denoted by appending a plus sign and a
            // series of dot separated identifiers immediately following the
            // patch or pre-release version.
            // Identifiers MUST comprise only ASCII alphanumerics and hyphen
            // [0-9A-Za-z-].
            // Identifiers MUST NOT be empty.
            id_list v;
            if (build_metadata_str == "")
                return v;
            
            // Handle situation where input string ends in a dot.  This won't
            // get caught by the std::getline() loop.
            if (build_metadata_str[build_metadata_str.length() - 1] == '.')
                throw std::domain_error("Dot-separated build metadata version parts must not be empty - got: " + build_metadata_str);

            appended_identifier_part aip;
            std::stringstream ss(build_metadata_str);
            for (
                std::getline(ss, aip.str_value, '.');
                (ss.rdstate() & std::stringstream::failbit) == 0;
                std::getline(ss, aip.str_value, '.'))
            {
                if (aip.str_value == "")
                    throw std::domain_error("Dot-separated build metadata parts must not be empty - got: " + build_metadata_str);
                aip.is_numeric = true;
                for (typename Ts::const_iterator i = aip.str_value.begin();
                    i != aip.str_value.end(); ++i)
                {
                    if (!isalnum(*i) && *i != '-')
                        throw std::domain_error("Dot-separated build metadata parts must comprise only ASCII alphanumerics and hyphen - got: " + build_metadata_str);
                    if (!isdigit(*i))
                        aip.is_numeric = false;
                }
                if (aip.is_numeric)
                    aip.numeric_value = atoi(aip.str_value.c_str());
                v.push_back(aip);
            }
            return v;
        }
        
    public:
    
        /**
            From the semver 2.0.0 specification, clauses 2, 9, 10:
            
            A normal version number MUST take the form X.Y.Z where X, Y, and Z
            are non-negative integers, and MUST NOT contain leading zeroes. X
            is the major version, Y is the minor version, and Z is the patch
            version. Each element MUST increase numerically.
            For instance:
            1.9.0 -> 1.10.0 -> 1.11.0.
            
            A pre-release version MAY be denoted by appending a hyphen and a
            series of dot separated identifiers immediately following the patch
            version. Identifiers MUST comprise only ASCII alphanumerics and
            hyphen [0-9A-Za-z-]. Identifiers MUST NOT be empty. Numeric
            identifiers MUST NOT include leading zeroes. Pre-release versions
            have a lower precedence than the associated normal version. A
            pre-release version indicates that the version is unstable and
            might not satisfy the intended compatibility requirements as
            denoted by its associated normal version.
            Examples: 1.0.0-alpha, 1.0.0-alpha.1, 1.0.0-0.3.7, 1.0.0-x.7.z.92.

            Build metadata MAY be denoted by appending a plus sign and a series
            of dot separated identifiers immediately following the patch or
            pre-release version. Identifiers MUST comprise only ASCII
            alphanumerics and hyphen [0-9A-Za-z-]. Identifiers MUST NOT be
            empty. Build metadata SHOULD be ignored when determining version
            precedence. Thus two versions that differ only in the build
            metadata, have the same precedence.
            Examples: 1.0.0-alpha+001, 1.0.0+20130313144700,
                      1.0.0-beta+exp.sha.5114f85.
        */
        static semantic_version parse(const Ts &version_str)
        {
            Ts mj_str, mn_str, pt_str, pr_str, bm_str;
            if (version_str == "")
                throw std::domain_error("Input string cannot be empty");
            
            // NOTE: at the time of coding, <regex> isn't fully supported on
            // available compilers (e.g. Ubuntu 12.04 LTS only provides GCC 4.7)
            // so we're doing it the hard way.
            std::stringstream ss(version_str);
            
            // Look for the first dot (major version part).
            std::getline(ss, mj_str, '.');
            if (ss.rdstate() & std::stringstream::eofbit)
            {
                throw std::domain_error(
                    "No dots '.' found in input string - "
                    "a normal version number MUST take the form X.Y.Z");
            }
            Tn mj = parse_numerical_version_part(mj_str);
            
            // Look for the first dot (minor version part).
            std::getline(ss, mn_str, '.');
            if (ss.rdstate() & std::stringstream::eofbit)
            {
                throw std::domain_error(
                    "No second dot '.' found in input string - "
                    "a normal version number MUST take the form X.Y.Z");
            }
            Tn mn = parse_numerical_version_part(mn_str);
            
            // Go to the end now, and parse the rest in a different way, since
            // we can't be certain of a fixed delimiter anymore.
            Ts ptprbm_str;
            std::getline(ss, ptprbm_str);
            size_t pr_pos = ptprbm_str.find('-');
            size_t bm_pos = ptprbm_str.find('+');
            if (pr_pos != Ts::npos && bm_pos != Ts::npos)
            {
                // Patch, pre-release, and build metadata are all present.
                pt_str = ptprbm_str.substr(0, pr_pos);
                pr_str = ptprbm_str.substr(pr_pos + 1, bm_pos - pr_pos - 1);
                bm_str = ptprbm_str.substr(bm_pos + 1);
                if (pr_str == "")
                    throw std::domain_error("If a pre-release is indicated with a hyphen, it cannot be empty");
                if (bm_str == "")
                    throw std::domain_error("If build metadata is indicated with a plus sign, it cannot be empty");
            }
            else if (pr_pos != Ts::npos)
            {
                // Patch and pre-release are present, but no build metadata.
                pt_str = ptprbm_str.substr(0, pr_pos);
                pr_str = ptprbm_str.substr(pr_pos + 1);
                if (pr_str == "")
                    throw std::domain_error("If a pre-release is indicated with a hyphen, it cannot be empty");
            }
            else if (bm_pos != Ts::npos)
            {
                // Patch and build metadata are present, but no pre-release.
                pt_str = ptprbm_str.substr(0, bm_pos);
                bm_str = ptprbm_str.substr(bm_pos + 1);
                if (bm_str == "")
                    throw std::domain_error("If build metadata is indicated with a plus sign, it cannot be empty");
            }
            else
            {
                // Patch is present, but no pre-release or build metadata.
                pt_str = ptprbm_str;
            }
            Tn pt = parse_numerical_version_part(pt_str);
            id_list pr_ids = parse_prerelease_str(pr_str);
            id_list bm_ids = parse_build_metadata_str(bm_str);

            return semantic_version(mj, mn, pt, pr_ids, bm_ids);
        }
        
        /**
            Get string representation of the prerelease part, if any.
            If no prerelease part is present, returns the empty string.
        */
        Ts prerelease_str() const
        {
            if (pr_str_expr_ == "" && !pr_ids_.empty())
            {
                std::stringstream ss;
                typename id_list::const_iterator i = pr_ids_.begin();
                ss << *(i++);
                for (; i != pr_ids_.end(); ++i)
                    ss << "." << (*i);
                pr_str_expr_ = ss.str();
            }
            return pr_str_expr_;
        }
        
        /**
            Get string representation of the build metadata part, if any.
            If no build metadata part is present, returns the empty string.
        */
        Ts build_metadata_str() const
        {
            if (bm_str_expr_ == "" && !bm_ids_.empty())
            {
                std::stringstream ss;
                typename id_list::const_iterator i = bm_ids_.begin();
                ss << *(i++);
                for (; i != bm_ids_.end(); ++i)
                    ss << "." << (*i);
                bm_str_expr_ = ss.str();
            }
            return bm_str_expr_;
        }

        /**
            Get string representation.
        */
        Ts to_str() const
        {
            if (str_expr_ == "")
            {
                std::stringstream ss;
                ss << mj() << "." << mn() << "." << pt();
                if (!pr_ids_.empty())
                    ss << "-" << prerelease_str();
                if (!bm_ids_.empty())
                    ss << "+" << build_metadata_str();
                str_expr_ = ss.str();
            }
            return str_expr_;
        }
        
        /**
            Write string representation onto an ostream.
        */
        friend std::ostream & operator << (
                std::ostream &os, const semantic_version & v )
        {
            return os << v.to_str();
        }
        
        /**
            Increment to the next major version.
            There is the option of preserving build metadata.
        */
        void next_major(bool preserve_build_metadata = false)
        {
            mj_++;
            mn_ = 0;
            pt_ = 0;
            pr_ids_.clear();
            if (!preserve_build_metadata)
                bm_ids_.clear();
            set_dirty();
        }
        
        /**
            Increment to the next minor version.
            There is the option of preserving build metadata.
        */
        void next_minor(bool preserve_build_metadata = false)
        {
            mn_++;
            pt_ = 0;
            pr_ids_.clear();
            if (!preserve_build_metadata)
                bm_ids_.clear();
            set_dirty();
        }
        
        /**
            Increment to the next patch version.
            There is the option of preserving build metadata.
        */
        void next_patch(bool preserve_build_metadata = false)
        {
            pt_++;
            pr_ids_.clear();
            if (!preserve_build_metadata)
                bm_ids_.clear();
            set_dirty();
        }
        
        /**
            From the semver 2.0.0 specification, clause 4:
            
            Major version zero (0.y.z) is for initial development. Anything may
            change at any time. The public API should not be considered stable.
        */
        bool is_initial_development() const
        {
            return mj() == 0;
        }
        
        bool is_zero() const
        {
            return mj() == 0 && mn() == 0 && pt() == 0;
        }
        
        struct identifier_part_compare
        {
            int compare_to(const id_list &a_ids, const id_list &b_ids) const
            {
                if (!a_ids.empty() && b_ids.empty())
                    return -1;
                if (a_ids.empty() && !b_ids.empty())
                    return 1;
                
                typename id_list::const_iterator ai = a_ids.begin();
                typename id_list::const_iterator bi = b_ids.begin();
                for (; ai != a_ids.end() && bi != b_ids.end(); ++ai, ++bi)
                {
                    // Compare the elements.
                    // If the identifiers are numeric, do a numeric comparison.
                    if (ai->is_numeric && bi->is_numeric)
                    {
                        if (ai->numeric_value == bi->numeric_value)
                            continue;
                        return ai->numeric_value < bi->numeric_value ? -1 : 1;
                    }
                    else if (ai->is_numeric)
                        // If one is numeric, it has lower precedence.
                        return -1;
                    else if (bi->is_numeric)
                        // If one is numeric, it has lower precedence.
                        return 1;
                        
                    // Otherwise, compare strings.
                    if (ai->str_value == bi->str_value)
                        continue;
                    
                    // Do a lexicographical comparison.
                    return std::lexicographical_compare(
                        ai->str_value.begin(), ai->str_value.end(),
                        bi->str_value.begin(), bi->str_value.end()) ? -1 : 1;
                }
                
                if (ai == a_ids.end() && bi == b_ids.end())
                    // They really are identical, wow.
                    return 0;
                
                // If a finished first, it has lower precedence, and vice versa.
                return (ai == a_ids.end()) ? -1 : 1;
            }
            
            bool operator () (const id_list &a, const id_list &b) const
            {
                return compare_to(a, b) == -1;
            }
        };
        
        /**
            From the semver 2.0.0 specification, clause 11:
        
            Precedence refers to how versions are compared to each other when
            ordered. Precedence MUST be calculated by separating the version into
            major, minor, patch and pre-release identifiers in that order (Build
            metadata does not figure into precedence). Precedence is determined by
            the first difference when comparing each of these identifiers from left
            to right as follows: Major, minor, and patch versions are always
            compared numerically. Example: 1.0.0 < 2.0.0 < 2.1.0 < 2.1.1. When
            major, minor, and patch are equal, a pre-release version has lower
            precedence than a normal version. Example: 1.0.0-alpha < 1.0.0.
            Precedence for two pre-release versions with the same major, minor, and
            patch version MUST be determined by comparing each dot separated
            identifier from left to right until a difference is found as follows:
            identifiers consisting of only digits are compared numerically and
            identifiers with letters or hyphens are compared lexically in ASCII
            sort order. Numeric identifiers always have lower precedence than
            non-numeric identifiers. A larger set of pre-release fields has a
            higher precedence than a smaller set, if all of the preceding
            identifiers are equal.
            Example: 1.0.0-alpha
                   < 1.0.0-alpha.1
                   < 1.0.0-alpha.beta
                   < 1.0.0-beta
                   < 1.0.0-beta.2
                   < 1.0.0-beta.11
                   < 1.0.0-rc.1
                   < 1.0.0.
        */
        struct strict_compare
        {
            int compare_to (const semantic_version &a,
                            const semantic_version &b) const
            {
                // Check major/minor/patch first.
                if (a.mj() != b.mj())
                    return a.mj() < b.mj() ? -1 : 1;
                if (a.mn() != b.mn())
                    return a.mn() < b.mn() ? -1 : 1;
                if (a.pt() != b.pt())
                    return a.pt() < b.pt() ? -1 : 1;
            
                // If we are here, this means the major/minor/patch are
                // identical, and we must check precedence for the pre-release
                // part, if present.
                identifier_part_compare ipc;
                return ipc.compare_to(a.pr_ids(), b.pr_ids());
            }
            
            bool operator () (const semantic_version &a,
                              const semantic_version &b) const
            {
                return compare_to(a, b) == -1;
            }
        };
        
        /**
            The metadata compare adds an additional comparison on the build
            metadata over and above the standard 'strict' compare.  The semver
            2.0.0 specification permits this, as it is worded:
            
            "Build metadata SHOULD be ignored when determining version
            precedence."
            
            The use of the word "SHOULD" instead of "MUST" means we can offer
            this alternative without breaking the specification.
        */
        struct metadata_compare
        {
            int compare_to (const semantic_version &a,
                            const semantic_version &b) const
            {
                strict_compare scmp;
                int strict_val = scmp.compare_to(a, b);
                if (strict_val != 0)
                    return strict_val;
                
                // If we are here, the mj/mn/pt/pre-release are identical.
                // Both inputs differ only in the build metadata part.
                identifier_part_compare ipc;
                return ipc.compare_to(a.bm_ids(), b.bm_ids());
            }
            
            bool operator () (const semantic_version &a,
                              const semantic_version &b) const
            {
                return compare_to(a, b) == -1;
            }
        };
        
        bool operator ==(const semantic_version &other) const
        {
            if (this == &other)
                return true;
            return to_str() == other.to_str();
        }
        
        bool operator !=(const semantic_version &other) const
        {
            if (this == &other)
                return false;
            return to_str() != other.to_str();
        }
        
        bool operator <(const semantic_version &other) const
        {
            metadata_compare cmp;
            return cmp.compare_to(*this, other) == -1;
        }
        
        bool operator <=(const semantic_version &other) const
        {
            metadata_compare cmp;
            return cmp.compare_to(*this, other) != 1;
        }
        
        bool operator >(const semantic_version &other) const
        {
            metadata_compare cmp;
            return cmp.compare_to(*this, other) == 1;
        }
        
        bool operator >=(const semantic_version &other) const
        {
            metadata_compare cmp;
            return cmp.compare_to(*this, other) != -1;
        }
    };
    
    /**
        The abbreviated name "semver" is more concise.
    */    
    typedef semantic_version<unsigned int, std::string> semver;
}

// Specialise for some std functions
namespace std {

    /// Specialise std::<< for dbmig::semantic_version
    template <typename Tn, typename Ts>
    ostream & operator <<(ostream &os, const dbmig::semantic_version<Tn, Ts> &v)
    {
        return os << v.to_str();
    }
    
} // namespace std



#endif // SEMANTIC_VERSION

