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

#include "script_action.hpp"

#include <stdexcept>

namespace dbmig {

///
/// Parse a script_action from a string
///
script_action script_action_parse(const std::string &expr)
{
    if      (expr == "install")  return script_action::install;
    else if (expr == "upgrade")  return script_action::upgrade;
    else if (expr == "rollback") return script_action::rollback;
    throw std::out_of_range("Not a valid script action: " + expr);
}

///
/// Convert a script_action to a string
///
std::string to_string(const script_action &action)
{
    switch (action) {
        case script_action::install:  return "install";
        case script_action::upgrade:  return "upgrade";
        case script_action::rollback: return "rollback";
    }
    throw std::out_of_range{"Unknown script action " + std::to_string(
        static_cast<std::underlying_type<script_action>::type>(action))};
}

} // dbmig namespace

