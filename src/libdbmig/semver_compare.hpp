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

#ifndef DBMIG_SEMVER_COMPARE_INCLUDED
#define DBMIG_SEMVER_COMPARE_INCLUDED

#include "semantic_version.hpp"

namespace dbmig
{
    ///
    /// Enumeration of approaches to comparing "non-script" versions
    ///
    /// There are two different approaches to comparing so-called
    /// "non-script" version numbers to version numbers of real scripts:
    ///
    /// e.g. comparing      4.32.83+script.938 (a real script version) to
    ///      something like 4.35.0             (a "non-script" version)
    ///
    /// The approaches are "low", in which case X.Y.Z behaves like it were
    /// actually X.Y.Z+script.0, and "high", in which case X.Y.Z behaves
    /// like X.Y.Z+script.INFINITY.
    ///
    enum class non_script_alignment {
        low,
        high
    };
    
    template<non_script_alignment Alignment>
    struct semver_script_compare
    {
        semver::strict_compare cmp;
        semver::identifier_part_compare pcmp;
        
        bool operator()(const semver &a, const semver &b) const {
            // Try a strict comparison first.
            auto val = cmp.compare_to(a, b);
            if (val != 0) {
                return val == -1;
            }
            // The absence of a script number means lower precedence.
            bool a_has_num = a.bm_ids().size() >= 2 &&
                a.bm_ids()[0].str_value == "script" &&
                a.bm_ids()[1].is_numeric;
            bool b_has_num = b.bm_ids().size() >= 2 &&
                b.bm_ids()[0].str_value == "script" &&
                b.bm_ids()[1].is_numeric;
            
            if (!a_has_num && !b_has_num)
                return pcmp(a.bm_ids(), b.bm_ids()); // Use semver compare
            if (a_has_num && !b_has_num)
                return (Alignment != non_script_alignment::low);
            if (!a_has_num && b_has_num)
                return (Alignment == non_script_alignment::low);
            return a.bm_ids()[1].numeric_value < b.bm_ids()[1].numeric_value;
        }
    };
}

#endif // DBMIG_SEMVER_COMPARE_INCLUDED

