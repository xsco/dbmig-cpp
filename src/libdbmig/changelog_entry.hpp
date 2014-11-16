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

#ifndef DBMIG_CHANGELOG_ENTRY_INCLUDED
#define DBMIG_CHANGELOG_ENTRY_INCLUDED

#include <string>
#include <vector>
#include <deque>
#include "script_action.hpp"
#include "semantic_version.hpp"

namespace dbmig
{
    struct rollback_step
    {
        semver from_version;
        semver to_version;
        std::string sha256_hash;
    };

    typedef std::vector<rollback_step> rollback_step_list;
    
    struct changelog_entry
    {
        std::string script_path;
        script_action action;
        semver from_version;
        semver to_version;
        std::string sha256_hash;
    };
    
    typedef std::deque<changelog_entry> changelog_entry_list;
}

#endif // DBMIG_CHANGELOG_ENTRY_INCLUDED

