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

#include "time.hpp"

using namespace std;

namespace dbmig {
namespace time {

///
/// Thread-safe replacement for std::localtime
///
std::tm localtime(const std::time_t &time)
{
  std::tm tm_snapshot;
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
  localtime_s(&tm_snapshot, &time);
#else
  localtime_r(&time, &tm_snapshot); // POSIX 
#endif
  return tm_snapshot;
}

///
/// Get the current time as std::time_t
///
std::time_t now()
{
  std::chrono::time_point<std::chrono::system_clock> system_now =
    std::chrono::system_clock::now();
  return std::chrono::system_clock::to_time_t(system_now);
}

} // time namespace
} // dbmig namespace

