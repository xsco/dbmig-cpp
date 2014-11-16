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

#ifndef DBMIG_FNMATCH_INCLUDED
#define DBMIG_FNMATCH_INCLUDED

// Simple C++ wrapper around POSIX fnmatch()
extern "C" {
    #include <fnmatch.h>
}
#include <cerrno>

#include <string>
#include <system_error>

namespace dbmig {

inline bool filename_match(
    const std::string &filename, const std::string &pattern)
{
    using namespace std;
    
    int ret = fnmatch(pattern.c_str(), filename.c_str(), 0);
    if (ret == 0)
        return true;
    if (ret == FNM_NOMATCH)
        return false;
    // Other return values indicate error.
    throw system_error(errno, system_category(),
        "unknown error from fnmatch");
}

} // namespace dbmig

#endif // DBMIG_FNMATCH_INCLUDED

