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

#ifndef DBMIG_SCRIPT_ACTION_INCLUDED
#define DBMIG_SCRIPT_ACTION_INCLUDED

#include <string>

namespace dbmig
{
    ///
    /// Enumeration of possible actions one might do with a script
    ///
    enum class script_action
    {
        install,
        upgrade,
        rollback
    };
    
    ///
    /// Parse a script_action from a string
    ///
    script_action script_action_parse(const std::string &expr);
    
    ///
    /// Convert a script_action to a string
    ///
    std::string to_string(const script_action &action);
}

#endif // DBMIG_SCRIPT_ACTION_INCLUDED

