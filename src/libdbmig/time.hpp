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

#ifndef DBMIG_TIME_INCLUDED
#define DBMIG_TIME_INCLUDED

#include <chrono>
#include <ctime>

namespace dbmig {
namespace time {

///
/// Thread-safe replacement for std::localtime
///
std::tm localtime(const std::time_t &time);

///
/// Get the current time as std::time_t
///
std::time_t now();

} // time
} // dbmig

#endif // DBMIG_TIME_INCLUDED

